// Code source based on libsvm 322 but modified to add support for multi-threading and SIMD (SSE, AVX and NEON)
#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_fileutils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <locale.h>
#include "compv/base/ml/libsvm-322/libsvm.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math_dot.h"
#include "compv/base/math/compv_math_scale.h"
#include "compv/base/math/compv_math_exp.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/ml/libsvm-322/intrin/x86/compv_ml_libsvm-322_intrin_sse2.h"
#include "compv/base/ml/libsvm-322/intrin/x86/compv_ml_libsvm-322_intrin_avx.h"
#define COMPV_THIS_CLASSNAME "LIBSVM"
int libsvm_version = LIBSVM_VERSION;
static svm_model libsvm_static_model; // Forcing ctor() to be called (e.g. will initialize CompVMat members)
typedef float Qfloat;
typedef signed char schar;
#ifndef min
template <class T> static inline T min(T x, T y) { return (x < y) ? x : y; }
#endif
#ifndef max
template <class T> static inline T max(T x, T y) { return (x > y) ? x : y; }
#endif
template <class T> static inline void swap(T& x, T& y) { T t = x; x = y; y = t; }
template <class S, class T> static inline void clone(T*& dst, S* src, int n)
{
	dst = new T[n];
	COMPV_NAMESPACE::CompVMem::copy((void *)dst, (void *)src, sizeof(T)*n);
}
static inline double powi(double base, int times)
{
	double tmp = base, ret = 1.0;

	for (int t = times; t > 0; t /= 2)
	{
		if (t % 2 == 1) ret *= tmp;
		tmp = tmp * tmp;
	}
	return ret;
}
#define INF HUGE_VAL
#define TAU 1e-12
#define Malloc(type,n)	reinterpret_cast<type *>(COMPV_NAMESPACE::CompVMem::malloc((n)*sizeof(type)))
#define Free(ptr)		COMPV_NAMESPACE::CompVMem::free(reinterpret_cast<void**>(&(ptr)))
#define Realloc(p,n)	COMPV_NAMESPACE::CompVMem::realloc((p), (n))
#define Calloc(n,sz)	COMPV_NAMESPACE::CompVMem::calloc((n), (sz))

//
// Kernel Cache
//
// l is the number of total data items
// size is the cache size limit in bytes
//
class Cache
{
public:
	Cache(int l, long int size);
	~Cache();

	// request data [0,len)
	// return some position p where [p,len) need to be filled
	// (p >= len if nothing needs to be filled)
	int get_data(const int index, Qfloat **data, int len);
	void swap_index(int i, int j);
private:
	int l;
	long int size;
	struct head_t
	{
		head_t *prev, *next;	// a circular list
		Qfloat *data;
		int len;		// data[0,len) is cached in this entry
	};

	head_t *head;
	head_t lru_head;
	void lru_delete(head_t *h);
	void lru_insert(head_t *h);
};

Cache::Cache(int l_, long int size_) :l(l_), size(size_)
{
	head = (head_t *)Calloc(l, sizeof(head_t));	// initialized to 0
	size /= sizeof(Qfloat);
	size -= l * sizeof(head_t) / sizeof(Qfloat);
	size = max(size, 2 * (long int)l);	// cache must be large enough for two columns
	lru_head.next = lru_head.prev = &lru_head;
}

Cache::~Cache()
{
	for (head_t *h = lru_head.next; h != &lru_head; h = h->next)
		Free(h->data);
	Free(head);
}

void Cache::lru_delete(head_t *h)
{
	// delete from current location
	h->prev->next = h->next;
	h->next->prev = h->prev;
}

void Cache::lru_insert(head_t *h)
{
	// insert to last position
	h->next = &lru_head;
	h->prev = lru_head.prev;
	h->prev->next = h;
	h->next->prev = h;
}

int Cache::get_data(const int index, Qfloat **data, int len)
{
	head_t *h = &head[index];
	if (h->len) lru_delete(h);
	int more = len - h->len;

	if (more > 0)
	{
		// free old space
		while (size < more)
		{
			head_t *old = lru_head.next;
			lru_delete(old);
			Free(old->data);
			size += old->len;
			old->data = 0;
			old->len = 0;
		}

		// allocate new space
		h->data = (Qfloat *)Realloc(h->data, sizeof(Qfloat)*len);
		size -= more;
		swap(h->len, len);
	}

	lru_insert(h);
	*data = h->data;
	return len;
}

void Cache::swap_index(int i, int j)
{
	if (i == j) return;

	if (head[i].len) lru_delete(&head[i]);
	if (head[j].len) lru_delete(&head[j]);
	swap(head[i].data, head[j].data);
	swap(head[i].len, head[j].len);
	if (head[i].len) lru_insert(&head[i]);
	if (head[j].len) lru_insert(&head[j]);

	if (i > j) swap(i, j);
	for (head_t *h = lru_head.next; h != &lru_head; h = h->next)
	{
		if (h->len > i)
		{
			if (h->len > j)
				swap(h->data[i], h->data[j]);
			else
			{
				// give up
				lru_delete(h);
				Free(h->data);
				size += h->len;
				h->data = 0;
				h->len = 0;
			}
		}
	}
}

//
// Kernel evaluation
//
// the static method k_function is for doing single kernel evaluation
// the constructor of Kernel prepares to calculate the l*l kernel matrix
// the member function get_Q is for getting one column from the Q Matrix
//
class QMatrix {
public:
	virtual Qfloat *get_Q(int column, int len) const = 0;
	virtual double *get_QD() const = 0;
	virtual void swap_index(int i, int j) const = 0;
	virtual ~QMatrix() {}
};

class Kernel : public QMatrix {
public:
	Kernel(int l, svm_node * const * x, const svm_parameter& param);
	virtual ~Kernel();

	static double k_function(const svm_node *x, const svm_node *y, const svm_parameter& param);
	static void k_function(const CompVMatPtr& x, const CompVMatPtr& yy, const size_t count, const svm_parameter& param, CompVMatPtr& kvalues, const struct svm_simd_func_ptrs *simd_func_ptrs);

	virtual Qfloat *get_Q(int column, int len) const = 0;
	virtual double *get_QD() const = 0;
	virtual void swap_index(int i, int j) const	// no so const...
	{
		swap(x[i], x[j]);
		if (x_square) swap(x_square[i], x_square[j]);
	}
	static void svm_kernel_rbf0_out_C(const double& gamma, const double* xSquarePtr, const double* dotMatPtr, double* outPtr, const size_t count)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found - For training");
#if 0 // TODO(dmi): next code faster
		const double gamma_minus = -gamma;
		for (size_t i = 0; i < count; ++i) {
			outPtr[i] = gamma_minus * ((xSquarePtr[i] + xSquarePtr[i]) - (2 * dotMatPtr[i]));
		}
#else
		const double gamma_times2_minus = -(2.0 * gamma);
		for (size_t i = 0; i < count; ++i) {
			outPtr[i] = gamma_times2_minus * (xSquarePtr[i] - dotMatPtr[i]);
		}
#endif
	}

	static void svm_kernel_rbf1_out_Step1_C(const double& gamma, const double& x_squarei, const double* xSquarejPtr, const double* dotMatPtr, double* outPtr, const size_t count)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found - For training");
		const double gamma_minus = -gamma;
		for (size_t j = 0; j < count; ++j) {
			outPtr[j] = gamma_minus * (x_squarei + xSquarejPtr[j] - 2 * dotMatPtr[j]);
		}
	}

	static void svm_kernel_rbf1_out_Step2_C(const double& yi, const double* yjPtr, const double* outStep1Ptr, float* outPtr, const size_t count)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found - For training");
		for (size_t j = 0; j < count; ++j) {
			outPtr[j] = static_cast<float>(yi * yjPtr[j] * outStep1Ptr[j]);
		}
	}

protected:

	double (Kernel::*kernel_function)(int i, int j) const;

	const svm_node* get_x(int i) const {
		return x[i];
	}
	const double* get_x_square(int i) const {
		return &x_square[i];
	}

	double kernel_rbf0(int i) const {
		return exp(-gamma*(x_square[i] + x_square[i] - 2 * dot(x[i], x[i])));
	}
	double kernel_rbf1(const double& x_squarei, const svm_node* xi, const double& yi, const double* y, int j) const {
		return exp(-gamma*(x_squarei + x_square[j] - 2 * dot(xi, x[j])));
	}

	// Multi-threaded
	void kernel_rbf0(const size_t count, double* out) const {
		const size_t xsize = svm_count(x[0]);
		CompVMatPtr dotMat;
		COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<double>(&dotMat, 1, count));
		double* dotMatPtr = dotMat->data<double>();
		auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
			CompVMatPtr xMat;
			const size_t mt_count = (end - start);
			// Compute dot(xi, xi)
			for (size_t i = start; i < end; ++i) {
				COMPV_CHECK_CODE_ASSERT(svm_copy(x[i], &xMat, xsize)); // convert to CompVMat
				simd_func_ptrs.dot_64f64f(xMat->data<const double>(), xMat->data<const double>(), xsize, 1, 0, 0, &dotMatPtr[i]);
			}
			// Compute output
			simd_func_ptrs.kernel_rbf0_out_64f64f(gamma, &x_square[start], &dotMatPtr[start], &out[start], mt_count);
			// Compute final out = exp(out)
			simd_func_ptrs.expo(&out[start], &out[start], mt_count);
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_ASSERT(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			1,
			count,
			1
		));
	}

	// Multi-threaded
	void kernel_rbf1(const double& x_squarei, const svm_node* xi, const double& yi, const double* y, const int start_, const int end_, Qfloat* out) const {
		const svm_node** xj = &x[start_];
		const double* x_squarej = &x_square[start_];
		const double* yj = &y[start_];
		Qfloat* outj = &out[start_];
		const size_t count = static_cast<size_t>(end_ - start_);
		const size_t xsize = svm_count(x[0]);
		CompVMatPtr xiMat, dotMat, outMat;
		COMPV_CHECK_CODE_ASSERT(svm_copy(xi, &xiMat, xsize));
		COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<double>(&dotMat, 1, count));
		COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<double>(&outMat, 1, count));
		double* dotMatPtr = dotMat->data<double>();
		double* outMatPtr = outMat->data<double>();

		auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
			const size_t mt_count = (end - start);
			CompVMatPtr xjMat;
			// Compute dot(xi, xj[j])
			for (size_t j = start; j < end; ++j) {
				COMPV_CHECK_CODE_ASSERT(svm_copy(xj[j], &xjMat, xsize));
				simd_func_ptrs.dot_64f64f(xiMat->data<const double>(), xjMat->data<const double>(), xsize, 1, 0, 0, &dotMatPtr[j]);
			}
			// Compute out step #1
			simd_func_ptrs.kernel_rbf1_out_Step1_64f64f(gamma, x_squarei, &x_squarej[start], &dotMatPtr[start], &outMatPtr[start], mt_count);
			// Compute exp(out step #1)
			simd_func_ptrs.expo(&outMatPtr[start], &outMatPtr[start], mt_count);
			// Compute out step #2
			simd_func_ptrs.kernel_rbf1_out_Step2_64f32f(yi, &yj[start], &outMatPtr[start], &outj[start], mt_count);
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_ASSERT(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			1,
			count,
			1
		));
	}

private:
	const svm_node **x;
	double *x_square;
	svm_simd_func_ptrs simd_func_ptrs;

	// svm_parameter
	const int kernel_type;
	const int degree;
	const double gamma;
	const double coef0;

	static double dot(const svm_node *px, const svm_node *py);
	double kernel_linear(int i, int j) const
	{
		return dot(x[i], x[j]);
	}
	double kernel_poly(int i, int j) const
	{
		return powi(gamma*dot(x[i], x[j]) + coef0, degree);
	}
	double kernel_rbf(int i, int j) const
	{
		return exp(-gamma*(x_square[i] + x_square[j] - 2 * dot(x[i], x[j])));
	}
	double kernel_sigmoid(int i, int j) const
	{
		return tanh(gamma*dot(x[i], x[j]) + coef0);
	}
	double kernel_precomputed(int i, int j) const
	{
		return x[i][(int)(x[j][0].value)].value;
	}
};

Kernel::Kernel(int l, svm_node * const * x_, const svm_parameter& param)
	:kernel_type(param.kernel_type), degree(param.degree),
	gamma(param.gamma), coef0(param.coef0)
{
	switch (kernel_type)
	{
	case LINEAR:
		kernel_function = &Kernel::kernel_linear;
		break;
	case POLY:
		kernel_function = &Kernel::kernel_poly;
		break;
	case RBF:
		kernel_function = &Kernel::kernel_rbf;
		break;
	case SIGMOID:
		kernel_function = &Kernel::kernel_sigmoid;
		break;
	case PRECOMPUTED:
		kernel_function = &Kernel::kernel_precomputed;
		break;
	}

	clone(x, x_, l);

	if (kernel_type == RBF) {
		x_square = new double[l];
		for (int i = 0; i < l; i++) {
			x_square[i] = dot(x[i], x[i]);
		}
	}
	else {
		x_square = 0;
	}

	simd_func_ptrs.init();
}

Kernel::~Kernel()
{
	delete[] x;
	delete[] x_square;
}

double Kernel::dot(const svm_node *px, const svm_node *py)
{
	double sum = 0;
	while (px->index != -1 && py->index != -1)
	{
		if (px->index == py->index)
		{
			sum += px->value * py->value;
			++px;
			++py;
		}
		else
		{
			if (px->index > py->index)
				++py;
			else
				++px;
		}
	}
	return sum;
}

double Kernel::k_function(const svm_node *x, const svm_node *y,
	const svm_parameter& param)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	switch (param.kernel_type)
	{
	case LINEAR:
		return dot(x, y);
	case POLY:
		return powi(param.gamma*dot(x, y) + param.coef0, param.degree);
	case RBF:
	{
		double sum = 0;
		while (x->index != -1 && y->index != -1) {
			if (x->index == y->index) {
				double d = x->value - y->value;
				sum += d*d;
				++x;
				++y;
			}
			else {
				if (x->index > y->index) {
					sum += y->value * y->value;
					++y;
				}
				else {
					sum += x->value * x->value;
					++x;
				}
			}
		}

		while (x->index != -1) {
			sum += x->value * x->value;
			++x;
		}

		while (y->index != -1) {
			sum += y->value * y->value;
			++y;
		}
		return exp(-param.gamma*sum);
	}
	case SIGMOID:
		return tanh(param.gamma*dot(x, y) + param.coef0);
	case PRECOMPUTED:  //x: test (validation), y: SV
		return x[(int)(y->value)].value;
	default:
		return 0;  // Unreachable 
	}
}

void Kernel::k_function(const CompVMatPtr& x, const CompVMatPtr& yy, const size_t count, const svm_parameter& param, CompVMatPtr& kvalues, const struct svm_simd_func_ptrs *simd_func_ptrs)
{
	COMPV_ASSERT(param.kernel_type == RBF); // For now RBF implementation only (see above)
	COMPV_CHECK_CODE_ASSERT(svm_k_function_rbf(x, yy, count, param.gamma, kvalues, simd_func_ptrs));
}

// An SMO algorithm in Fan et al., JMLR 6(2005), p. 1889--1918
// Solves:
//
//	min 0.5(\alpha^T Q \alpha) + p^T \alpha
//
//		y^T \alpha = \delta
//		y_i = +1 or -1
//		0 <= alpha_i <= Cp for y_i = 1
//		0 <= alpha_i <= Cn for y_i = -1
//
// Given:
//
//	Q, p, y, Cp, Cn, and an initial feasible point \alpha
//	l is the size of vectors and matrices
//	eps is the stopping tolerance
//
// solution will be put in \alpha, objective value will be put in obj
//
class Solver {
public:
	Solver() {};
	virtual ~Solver() {};

	struct SolutionInfo {
		double obj;
		double rho;
		double upper_bound_p;
		double upper_bound_n;
		double r;	// for Solver_NU
	};

	void Solve(int l, const QMatrix& Q, const double *p_, const schar *y_,
		double *alpha_, double Cp, double Cn, double eps,
		SolutionInfo* si, int shrinking);
protected:
	int active_size;
	schar *y;
	double *G;		// gradient of objective function
	enum { LOWER_BOUND, UPPER_BOUND, FREE };
	char *alpha_status;	// LOWER_BOUND, UPPER_BOUND, FREE
	double *alpha;
	const QMatrix *Q;
	const double *QD;
	double eps;
	double GMin;
	double Cp, Cn;
	double *p;
	int *active_set;
	double *G_bar;		// gradient, if we treat free variables as 0
	int l;
	bool unshrink;	// XXX

	double get_C(int i)
	{
		return (y[i] > 0) ? Cp : Cn;
	}
	void update_alpha_status(int i)
	{
		if (alpha[i] >= get_C(i))
			alpha_status[i] = UPPER_BOUND;
		else if (alpha[i] <= 0)
			alpha_status[i] = LOWER_BOUND;
		else alpha_status[i] = FREE;
	}
	bool is_upper_bound(int i) { return alpha_status[i] == UPPER_BOUND; }
	bool is_lower_bound(int i) { return alpha_status[i] == LOWER_BOUND; }
	bool is_free(int i) { return alpha_status[i] == FREE; }
	void swap_index(int i, int j);
	void reconstruct_gradient();
	virtual int select_working_set(int &i, int &j);
	virtual double calculate_rho();
	virtual void do_shrinking();
private:
	bool be_shrunk(int i, double Gmax1, double Gmax2);
};

void Solver::swap_index(int i, int j)
{
	Q->swap_index(i, j);
	swap(y[i], y[j]);
	swap(G[i], G[j]);
	swap(alpha_status[i], alpha_status[j]);
	swap(alpha[i], alpha[j]);
	swap(p[i], p[j]);
	swap(active_set[i], active_set[j]);
	swap(G_bar[i], G_bar[j]);
}

void Solver::reconstruct_gradient()
{
	// reconstruct inactive elements of G from G_bar and free variables

	if (active_size == l) return;

	int i, j;
	int nr_free = 0;

	for (j = active_size; j < l; j++)
		G[j] = G_bar[j] + p[j];

	for (j = 0; j < active_size; j++)
		if (is_free(j))
			nr_free++;

	if (2 * nr_free < active_size)
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "WARNING: using -h 0 may be faster");

	if (nr_free*l > 2 * active_size*(l - active_size))
	{
		for (i = active_size; i < l; i++)
		{
			const Qfloat *Q_i = Q->get_Q(i, active_size);
			for (j = 0; j < active_size; j++)
				if (is_free(j))
					G[i] += alpha[j] * Q_i[j];
		}
	}
	else
	{
		for (i = 0; i < active_size; i++)
			if (is_free(i))
			{
				const Qfloat *Q_i = Q->get_Q(i, l);
				double alpha_i = alpha[i];
				for (j = active_size; j < l; j++)
					G[j] += alpha_i * Q_i[j];
			}
	}
}

void Solver::Solve(int l, const QMatrix& Q, const double *p_, const schar *y_,
	double *alpha_, double Cp, double Cn, double eps,
	SolutionInfo* si, int shrinking)
{
	this->l = l;
	this->Q = &Q;
	QD = Q.get_QD();
	clone(p, p_, l);
	clone(y, y_, l);
	clone(alpha, alpha_, l);
	this->Cp = Cp;
	this->Cn = Cn;
	this->eps = eps;
	this->GMin = DBL_MAX;
	unshrink = false;

	// initialize alpha_status
	{
		alpha_status = new char[l];
		for (int i = 0; i < l; i++)
			update_alpha_status(i);
	}

	// initialize active set (for shrinking)
	{
		active_set = new int[l];
		for (int i = 0; i < l; i++)
			active_set[i] = i;
		active_size = l;
	}

	// initialize gradient
	{
		G = new double[l];
		G_bar = new double[l];
		int i;
		for (i = 0; i < l; i++)
		{
			G[i] = p[i];
			G_bar[i] = 0;
		}
		for (i = 0; i < l; i++)
			if (!is_lower_bound(i))
			{
				const Qfloat *Q_i = Q.get_Q(i, l);
				double alpha_i = alpha[i];
				int j;
				for (j = 0; j < l; j++)
					G[j] += alpha_i*Q_i[j];
				if (is_upper_bound(i))
					for (j = 0; j < l; j++)
						G_bar[j] += get_C(i) * Q_i[j];
			}
	}

	// optimization step

	int iter = 0;
	int max_iter = max(10000000, l > INT_MAX / 100 ? INT_MAX : 100 * l);
	int counter = min(l, 1000) + 1;

	while (iter < max_iter)
	{
		// show progress and do shrinking

		if (--counter == 0)
		{
			counter = min(l, 1000);
			if (shrinking) do_shrinking();
			COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, ".");
		}

		int i, j;
		if (select_working_set(i, j) != 0)
		{
			// reconstruct the whole gradient
			reconstruct_gradient();
			// reset active set size and check
			active_size = l;
			COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "*");
			if (select_working_set(i, j) != 0)
				break;
			else
				counter = 1;	// do shrinking next iteration
		}

		++iter;

		// update alpha[i] and alpha[j], handle bounds carefully

		const Qfloat *Q_i = Q.get_Q(i, active_size);
		const Qfloat *Q_j = Q.get_Q(j, active_size);

		double C_i = get_C(i);
		double C_j = get_C(j);

		double old_alpha_i = alpha[i];
		double old_alpha_j = alpha[j];

		if (y[i] != y[j])
		{
			double quad_coef = QD[i] + QD[j] + 2 * Q_i[j];
			if (quad_coef <= 0)
				quad_coef = TAU;
			double delta = (-G[i] - G[j]) / quad_coef;
			double diff = alpha[i] - alpha[j];
			alpha[i] += delta;
			alpha[j] += delta;

			if (diff > 0)
			{
				if (alpha[j] < 0)
				{
					alpha[j] = 0;
					alpha[i] = diff;
				}
			}
			else
			{
				if (alpha[i] < 0)
				{
					alpha[i] = 0;
					alpha[j] = -diff;
				}
			}
			if (diff > C_i - C_j)
			{
				if (alpha[i] > C_i)
				{
					alpha[i] = C_i;
					alpha[j] = C_i - diff;
				}
			}
			else
			{
				if (alpha[j] > C_j)
				{
					alpha[j] = C_j;
					alpha[i] = C_j + diff;
				}
			}
		}
		else
		{
			double quad_coef = QD[i] + QD[j] - 2 * Q_i[j];
			if (quad_coef <= 0)
				quad_coef = TAU;
			double delta = (G[i] - G[j]) / quad_coef;
			double sum = alpha[i] + alpha[j];
			alpha[i] -= delta;
			alpha[j] += delta;

			if (sum > C_i)
			{
				if (alpha[i] > C_i)
				{
					alpha[i] = C_i;
					alpha[j] = sum - C_i;
				}
			}
			else
			{
				if (alpha[j] < 0)
				{
					alpha[j] = 0;
					alpha[i] = sum;
				}
			}
			if (sum > C_j)
			{
				if (alpha[j] > C_j)
				{
					alpha[j] = C_j;
					alpha[i] = sum - C_j;
				}
			}
			else
			{
				if (alpha[i] < 0)
				{
					alpha[i] = 0;
					alpha[j] = sum;
				}
			}
		}

		// update G

		double delta_alpha_i = alpha[i] - old_alpha_i;
		double delta_alpha_j = alpha[j] - old_alpha_j;

		for (int k = 0; k < active_size; k++)
		{
			G[k] += Q_i[k] * delta_alpha_i + Q_j[k] * delta_alpha_j;
		}

		// update alpha_status and G_bar

		{
			bool ui = is_upper_bound(i);
			bool uj = is_upper_bound(j);
			update_alpha_status(i);
			update_alpha_status(j);
			int k;
			if (ui != is_upper_bound(i))
			{
				Q_i = Q.get_Q(i, l);
				if (ui)
					for (k = 0; k < l; k++)
						G_bar[k] -= C_i * Q_i[k];
				else
					for (k = 0; k < l; k++)
						G_bar[k] += C_i * Q_i[k];
			}

			if (uj != is_upper_bound(j))
			{
				Q_j = Q.get_Q(j, l);
				if (uj)
					for (k = 0; k < l; k++)
						G_bar[k] -= C_j * Q_j[k];
				else
					for (k = 0; k < l; k++)
						G_bar[k] += C_j * Q_j[k];
			}
		}
	}

	if (iter >= max_iter)
	{
		if (active_size < l)
		{
			// reconstruct the whole gradient to calculate objective value
			reconstruct_gradient();
			active_size = l;
			COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "*");
		}
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "WARNING: reaching max number of iterations");
	}

	// calculate rho

	si->rho = calculate_rho();

	// calculate objective value
	{
		double v = 0;
		int i;
		for (i = 0; i < l; i++)
			v += alpha[i] * (G[i] + p[i]);

		si->obj = v / 2;
	}

	// put back the solution
	{
		for (int i = 0; i < l; i++)
			alpha_[active_set[i]] = alpha[i];
	}

	// juggle everything back
	/*{
		for(int i=0;i<l;i++)
			while(active_set[i] != i)
				swap_index(i,active_set[i]);
				// or Q.swap_index(i,active_set[i]);
	}*/

	si->upper_bound_p = Cp;
	si->upper_bound_n = Cn;

	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "optimization finished, #iter = %d", iter);

	delete[] p;
	delete[] y;
	delete[] alpha;
	delete[] alpha_status;
	delete[] active_set;
	delete[] G;
	delete[] G_bar;
}

// return 1 if already optimal, return 0 otherwise
int Solver::select_working_set(int &out_i, int &out_j)
{
	// return i,j such that
	// i: maximizes -y_i * grad(f)_i, i in I_up(\alpha)
	// j: minimizes the decrease of obj value
	//    (if quadratic coefficeint <= 0, replace it with tau)
	//    -y_j*grad(f)_j < -y_i*grad(f)_i, j in I_low(\alpha)

	double Gmax = -INF;
	double Gmax2 = -INF;
	int Gmax_idx = -1;
	int Gmin_idx = -1;
	double obj_diff_min = INF;

	for (int t = 0; t < active_size; t++)
		if (y[t] == +1)
		{
			if (!is_upper_bound(t))
				if (-G[t] >= Gmax)
				{
					Gmax = -G[t];
					Gmax_idx = t;
				}
		}
		else
		{
			if (!is_lower_bound(t))
				if (G[t] >= Gmax)
				{
					Gmax = G[t];
					Gmax_idx = t;
				}
		}

	int i = Gmax_idx;
	const Qfloat *Q_i = nullptr;
	if (i != -1) // nullptr Q_i not accessed: Gmax=-INF if i=-1
		Q_i = Q->get_Q(i, active_size);

	for (int j = 0; j < active_size; j++)
	{
		if (y[j] == +1)
		{
			if (!is_lower_bound(j))
			{
				double grad_diff = Gmax + G[j];
				if (G[j] >= Gmax2)
					Gmax2 = G[j];
				if (grad_diff > 0)
				{
					double obj_diff;
					double quad_coef = QD[i] + QD[j] - 2.0*y[i] * Q_i[j];
					if (quad_coef > 0)
						obj_diff = -(grad_diff*grad_diff) / quad_coef;
					else
						obj_diff = -(grad_diff*grad_diff) / TAU;

					if (obj_diff <= obj_diff_min)
					{
						Gmin_idx = j;
						obj_diff_min = obj_diff;
					}
				}
			}
		}
		else
		{
			if (!is_upper_bound(j))
			{
				double grad_diff = Gmax - G[j];
				if (-G[j] >= Gmax2)
					Gmax2 = -G[j];
				if (grad_diff > 0)
				{
					double obj_diff;
					double quad_coef = QD[i] + QD[j] + 2.0*y[i] * Q_i[j];
					if (quad_coef > 0)
						obj_diff = -(grad_diff*grad_diff) / quad_coef;
					else
						obj_diff = -(grad_diff*grad_diff) / TAU;

					if (obj_diff <= obj_diff_min)
					{
						Gmin_idx = j;
						obj_diff_min = obj_diff;
					}
				}
			}
		}
	}

	const double GmaxSum = Gmax + Gmax2;
	if (GmaxSum < this->GMin) {
		this->GMin = GmaxSum;
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "GmaxSum = %lf, GMin = %lf", GmaxSum, this->GMin);

	if (GmaxSum < eps || Gmin_idx == -1) {
		return 1;
	}

	out_i = Gmax_idx;
	out_j = Gmin_idx;
	return 0;
}

bool Solver::be_shrunk(int i, double Gmax1, double Gmax2)
{
	if (is_upper_bound(i))
	{
		if (y[i] == +1)
			return(-G[i] > Gmax1);
		else
			return(-G[i] > Gmax2);
	}
	else if (is_lower_bound(i))
	{
		if (y[i] == +1)
			return(G[i] > Gmax2);
		else
			return(G[i] > Gmax1);
	}
	else
		return(false);
}

void Solver::do_shrinking()
{
	int i;
	double Gmax1 = -INF;		// max { -y_i * grad(f)_i | i in I_up(\alpha) }
	double Gmax2 = -INF;		// max { y_i * grad(f)_i | i in I_low(\alpha) }

	// find maximal violating pair first
	for (i = 0; i < active_size; i++)
	{
		if (y[i] == +1)
		{
			if (!is_upper_bound(i))
			{
				if (-G[i] >= Gmax1)
					Gmax1 = -G[i];
			}
			if (!is_lower_bound(i))
			{
				if (G[i] >= Gmax2)
					Gmax2 = G[i];
			}
		}
		else
		{
			if (!is_upper_bound(i))
			{
				if (-G[i] >= Gmax2)
					Gmax2 = -G[i];
			}
			if (!is_lower_bound(i))
			{
				if (G[i] >= Gmax1)
					Gmax1 = G[i];
			}
		}
	}

	if (unshrink == false && Gmax1 + Gmax2 <= eps * 10)
	{
		unshrink = true;
		reconstruct_gradient();
		active_size = l;
		COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "*");
	}

	for (i = 0; i < active_size; i++)
		if (be_shrunk(i, Gmax1, Gmax2))
		{
			active_size--;
			while (active_size > i)
			{
				if (!be_shrunk(active_size, Gmax1, Gmax2))
				{
					swap_index(i, active_size);
					break;
				}
				active_size--;
			}
		}
}

double Solver::calculate_rho()
{
	double r;
	int nr_free = 0;
	double ub = INF, lb = -INF, sum_free = 0;
	for (int i = 0; i < active_size; i++)
	{
		double yG = y[i] * G[i];

		if (is_upper_bound(i))
		{
			if (y[i] == -1)
				ub = min(ub, yG);
			else
				lb = max(lb, yG);
		}
		else if (is_lower_bound(i))
		{
			if (y[i] == +1)
				ub = min(ub, yG);
			else
				lb = max(lb, yG);
		}
		else
		{
			++nr_free;
			sum_free += yG;
		}
	}

	if (nr_free > 0)
		r = sum_free / nr_free;
	else
		r = (ub + lb) / 2;

	return r;
}

//
// Solver for nu-svm classification and regression
//
// additional constraint: e^T \alpha = constant
//
class Solver_NU : public Solver
{
public:
	Solver_NU() {}
	void Solve(int l, const QMatrix& Q, const double *p, const schar *y,
		double *alpha, double Cp, double Cn, double eps,
		SolutionInfo* si, int shrinking)
	{
		this->si = si;
		Solver::Solve(l, Q, p, y, alpha, Cp, Cn, eps, si, shrinking);
	}
private:
	SolutionInfo *si;
	int select_working_set(int &i, int &j);
	double calculate_rho();
	bool be_shrunk(int i, double Gmax1, double Gmax2, double Gmax3, double Gmax4);
	void do_shrinking();
};

// return 1 if already optimal, return 0 otherwise
int Solver_NU::select_working_set(int &out_i, int &out_j)
{
	// return i,j such that y_i = y_j and
	// i: maximizes -y_i * grad(f)_i, i in I_up(\alpha)
	// j: minimizes the decrease of obj value
	//    (if quadratic coefficeint <= 0, replace it with tau)
	//    -y_j*grad(f)_j < -y_i*grad(f)_i, j in I_low(\alpha)

	double Gmaxp = -INF;
	double Gmaxp2 = -INF;
	int Gmaxp_idx = -1;

	double Gmaxn = -INF;
	double Gmaxn2 = -INF;
	int Gmaxn_idx = -1;

	int Gmin_idx = -1;
	double obj_diff_min = INF;

	for (int t = 0; t < active_size; t++)
		if (y[t] == +1)
		{
			if (!is_upper_bound(t))
				if (-G[t] >= Gmaxp)
				{
					Gmaxp = -G[t];
					Gmaxp_idx = t;
				}
		}
		else
		{
			if (!is_lower_bound(t))
				if (G[t] >= Gmaxn)
				{
					Gmaxn = G[t];
					Gmaxn_idx = t;
				}
		}

	int ip = Gmaxp_idx;
	int in = Gmaxn_idx;
	const Qfloat *Q_ip = nullptr;
	const Qfloat *Q_in = nullptr;
	if (ip != -1) // nullptr Q_ip not accessed: Gmaxp=-INF if ip=-1
		Q_ip = Q->get_Q(ip, active_size);
	if (in != -1)
		Q_in = Q->get_Q(in, active_size);

	for (int j = 0; j < active_size; j++)
	{
		if (y[j] == +1)
		{
			if (!is_lower_bound(j))
			{
				double grad_diff = Gmaxp + G[j];
				if (G[j] >= Gmaxp2)
					Gmaxp2 = G[j];
				if (grad_diff > 0)
				{
					double obj_diff;
					double quad_coef = QD[ip] + QD[j] - 2 * Q_ip[j];
					if (quad_coef > 0)
						obj_diff = -(grad_diff*grad_diff) / quad_coef;
					else
						obj_diff = -(grad_diff*grad_diff) / TAU;

					if (obj_diff <= obj_diff_min)
					{
						Gmin_idx = j;
						obj_diff_min = obj_diff;
					}
				}
			}
		}
		else
		{
			if (!is_upper_bound(j))
			{
				double grad_diff = Gmaxn - G[j];
				if (-G[j] >= Gmaxn2)
					Gmaxn2 = -G[j];
				if (grad_diff > 0)
				{
					double obj_diff;
					double quad_coef = QD[in] + QD[j] - 2 * Q_in[j];
					if (quad_coef > 0)
						obj_diff = -(grad_diff*grad_diff) / quad_coef;
					else
						obj_diff = -(grad_diff*grad_diff) / TAU;

					if (obj_diff <= obj_diff_min)
					{
						Gmin_idx = j;
						obj_diff_min = obj_diff;
					}
				}
			}
		}
	}

	if (max(Gmaxp + Gmaxp2, Gmaxn + Gmaxn2) < eps || Gmin_idx == -1)
		return 1;

	if (y[Gmin_idx] == +1)
		out_i = Gmaxp_idx;
	else
		out_i = Gmaxn_idx;
	out_j = Gmin_idx;

	return 0;
}

bool Solver_NU::be_shrunk(int i, double Gmax1, double Gmax2, double Gmax3, double Gmax4)
{
	if (is_upper_bound(i))
	{
		if (y[i] == +1)
			return(-G[i] > Gmax1);
		else
			return(-G[i] > Gmax4);
	}
	else if (is_lower_bound(i))
	{
		if (y[i] == +1)
			return(G[i] > Gmax2);
		else
			return(G[i] > Gmax3);
	}
	else
		return(false);
}

void Solver_NU::do_shrinking()
{
	double Gmax1 = -INF;	// max { -y_i * grad(f)_i | y_i = +1, i in I_up(\alpha) }
	double Gmax2 = -INF;	// max { y_i * grad(f)_i | y_i = +1, i in I_low(\alpha) }
	double Gmax3 = -INF;	// max { -y_i * grad(f)_i | y_i = -1, i in I_up(\alpha) }
	double Gmax4 = -INF;	// max { y_i * grad(f)_i | y_i = -1, i in I_low(\alpha) }

	// find maximal violating pair first
	int i;
	for (i = 0; i < active_size; i++)
	{
		if (!is_upper_bound(i))
		{
			if (y[i] == +1)
			{
				if (-G[i] > Gmax1) Gmax1 = -G[i];
			}
			else	if (-G[i] > Gmax4) Gmax4 = -G[i];
		}
		if (!is_lower_bound(i))
		{
			if (y[i] == +1)
			{
				if (G[i] > Gmax2) Gmax2 = G[i];
			}
			else	if (G[i] > Gmax3) Gmax3 = G[i];
		}
	}

	if (unshrink == false && max(Gmax1 + Gmax2, Gmax3 + Gmax4) <= eps * 10)
	{
		unshrink = true;
		reconstruct_gradient();
		active_size = l;
	}

	for (i = 0; i < active_size; i++)
		if (be_shrunk(i, Gmax1, Gmax2, Gmax3, Gmax4))
		{
			active_size--;
			while (active_size > i)
			{
				if (!be_shrunk(active_size, Gmax1, Gmax2, Gmax3, Gmax4))
				{
					swap_index(i, active_size);
					break;
				}
				active_size--;
			}
		}
}

double Solver_NU::calculate_rho()
{
	int nr_free1 = 0, nr_free2 = 0;
	double ub1 = INF, ub2 = INF;
	double lb1 = -INF, lb2 = -INF;
	double sum_free1 = 0, sum_free2 = 0;

	for (int i = 0; i < active_size; i++)
	{
		if (y[i] == +1)
		{
			if (is_upper_bound(i))
				lb1 = max(lb1, G[i]);
			else if (is_lower_bound(i))
				ub1 = min(ub1, G[i]);
			else
			{
				++nr_free1;
				sum_free1 += G[i];
			}
		}
		else
		{
			if (is_upper_bound(i))
				lb2 = max(lb2, G[i]);
			else if (is_lower_bound(i))
				ub2 = min(ub2, G[i]);
			else
			{
				++nr_free2;
				sum_free2 += G[i];
			}
		}
	}

	double r1, r2;
	if (nr_free1 > 0)
		r1 = sum_free1 / nr_free1;
	else
		r1 = (ub1 + lb1) / 2;

	if (nr_free2 > 0)
		r2 = sum_free2 / nr_free2;
	else
		r2 = (ub2 + lb2) / 2;

	si->r = (r1 + r2) / 2;
	return (r1 - r2) / 2;
}

//
// Q matrices for various formulations
//
class SVC_Q : public Kernel
{
public:
	SVC_Q(const svm_problem& prob, const svm_parameter& param, const double *y_)
		: Kernel(prob.l, prob.x, param), isRBF(param.kernel_type == RBF)
	{
		clone(y, y_, prob.l);
		cache = new Cache(prob.l, (long int)(param.cache_size*(1 << 20)));
		QD = new double[prob.l];
		if (isRBF) {
			kernel_rbf0(static_cast<size_t>(prob.l), QD);
		}
		else {
			for (int i = 0; i < prob.l; i++) {
				QD[i] = (this->*kernel_function)(i, i);
			}
		}
	}

	Qfloat *get_Q(int i, int len) const
	{
		Qfloat *data;
		int start, j;
		if ((start = cache->get_data(i, &data, len)) < len) {
			if (isRBF) {
				const double& yi = y[i];
				const double& x_squarei = *get_x_square(i);
				const svm_node* xi = get_x(i);
				kernel_rbf1(
					x_squarei, xi, yi,
					y,
					start, len,
					data);
			}
			else {
				for (j = start; j < len; j++) {
					data[j] = (Qfloat)(y[i] * y[j] * (this->*kernel_function)(i, j));
				}
			}
		}
		return data;
	}

	double *get_QD() const
	{
		return QD;
	}

	void swap_index(int i, int j) const
	{
		cache->swap_index(i, j);
		Kernel::swap_index(i, j);
		swap(y[i], y[j]);
		swap(QD[i], QD[j]);
	}

	~SVC_Q()
	{
		delete[] y;
		delete cache;
		delete[] QD;
	}
private:
	double *y;
	Cache *cache;
	double *QD;
	bool isRBF;
};

class ONE_CLASS_Q : public Kernel
{
public:
	ONE_CLASS_Q(const svm_problem& prob, const svm_parameter& param)
		:Kernel(prob.l, prob.x, param)
	{
		cache = new Cache(prob.l, (long int)(param.cache_size*(1 << 20)));
		QD = new double[prob.l];
		for (int i = 0; i < prob.l; i++)
			QD[i] = (this->*kernel_function)(i, i);
	}

	Qfloat *get_Q(int i, int len) const
	{
		Qfloat *data;
		int start, j;
		if ((start = cache->get_data(i, &data, len)) < len)
		{
			for (j = start; j < len; j++)
				data[j] = (Qfloat)(this->*kernel_function)(i, j);
		}
		return data;
	}

	double *get_QD() const
	{
		return QD;
	}

	void swap_index(int i, int j) const
	{
		cache->swap_index(i, j);
		Kernel::swap_index(i, j);
		swap(QD[i], QD[j]);
	}

	~ONE_CLASS_Q()
	{
		delete cache;
		delete[] QD;
	}
private:
	Cache *cache;
	double *QD;
};

class SVR_Q : public Kernel
{
public:
	SVR_Q(const svm_problem& prob, const svm_parameter& param)
		:Kernel(prob.l, prob.x, param)
	{
		l = prob.l;
		cache = new Cache(l, (long int)(param.cache_size*(1 << 20)));
		QD = new double[2 * l];
		sign = new schar[2 * l];
		index = new int[2 * l];
		for (int k = 0; k < l; k++)
		{
			sign[k] = 1;
			sign[k + l] = -1;
			index[k] = k;
			index[k + l] = k;
			QD[k] = (this->*kernel_function)(k, k);
			QD[k + l] = QD[k];
		}
		buffer[0] = new Qfloat[2 * l];
		buffer[1] = new Qfloat[2 * l];
		next_buffer = 0;
	}

	void swap_index(int i, int j) const
	{
		swap(sign[i], sign[j]);
		swap(index[i], index[j]);
		swap(QD[i], QD[j]);
	}

	Qfloat *get_Q(int i, int len) const
	{
		Qfloat *data;
		int j, real_i = index[i];
		if (cache->get_data(real_i, &data, l) < l)
		{
			for (j = 0; j < l; j++)
				data[j] = (Qfloat)(this->*kernel_function)(real_i, j);
		}

		// reorder and copy
		Qfloat *buf = buffer[next_buffer];
		next_buffer = 1 - next_buffer;
		schar si = sign[i];
		for (j = 0; j < len; j++)
			buf[j] = (Qfloat)si * (Qfloat)sign[j] * data[index[j]];
		return buf;
	}

	double *get_QD() const
	{
		return QD;
	}

	~SVR_Q()
	{
		delete cache;
		delete[] sign;
		delete[] index;
		delete[] buffer[0];
		delete[] buffer[1];
		delete[] QD;
	}
private:
	int l;
	Cache *cache;
	schar *sign;
	int *index;
	mutable int next_buffer;
	Qfloat *buffer[2];
	double *QD;
};

//
// construct and solve various formulations
//
static void solve_c_svc(
	const svm_problem *prob, const svm_parameter* param,
	double *alpha, Solver::SolutionInfo* si, double Cp, double Cn)
{
	int l = prob->l;
	double *minus_ones = new double[l];
	schar *y = new schar[l];
	double *yd = new double[l];

	int i;

	for (i = 0; i < l; i++)	{
		alpha[i] = 0;
		minus_ones[i] = -1;
		y[i] = (prob->y[i] > 0) ? +1 : -1;
		yd[i] = y[i];
	}

	Solver s;
	s.Solve(l, SVC_Q(*prob, *param, yd), minus_ones, y,
		alpha, Cp, Cn, param->eps, si, param->shrinking);

	double sum_alpha = 0;
	for (i = 0; i < l; i++) {
		sum_alpha += alpha[i];
	}

	if (Cp == Cn) {
		COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "nu = %f", sum_alpha / (Cp*prob->l));
	}

	for (i = 0; i < l; i++) {
		alpha[i] *= yd[i];
	}

	delete[] minus_ones;
	delete[] y;
	delete[] yd;
}

static void solve_nu_svc(
	const svm_problem *prob, const svm_parameter *param,
	double *alpha, Solver::SolutionInfo* si)
{
	int i;
	int l = prob->l;
	double nu = param->nu;

	schar *y = new schar[l];
	double *yd = new double[l];

	for (i = 0; i < l; i++) {
		y[i] = (prob->y[i] > 0) ? +1 : -1;
		yd[i] = y[i];
	}

	double sum_pos = nu*l / 2;
	double sum_neg = nu*l / 2;

	for (i = 0; i < l; i++)
		if (y[i] == +1)
		{
			alpha[i] = min(1.0, sum_pos);
			sum_pos -= alpha[i];
		}
		else
		{
			alpha[i] = min(1.0, sum_neg);
			sum_neg -= alpha[i];
		}

	double *zeros = new double[l];

	for (i = 0; i < l; i++)
		zeros[i] = 0;

	Solver_NU s;
	s.Solve(l, SVC_Q(*prob, *param, yd), zeros, y,
		alpha, 1.0, 1.0, param->eps, si, param->shrinking);
	double r = si->r;

	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "C = %f", 1 / r);

	for (i = 0; i < l; i++)
		alpha[i] *= yd[i] / r;

	si->rho /= r;
	si->obj /= (r*r);
	si->upper_bound_p = 1 / r;
	si->upper_bound_n = 1 / r;

	delete[] yd;
	delete[] y;
	delete[] zeros;
}

static void solve_one_class(
	const svm_problem *prob, const svm_parameter *param,
	double *alpha, Solver::SolutionInfo* si)
{
	int l = prob->l;
	double *zeros = new double[l];
	schar *ones = new schar[l];
	int i;

	int n = (int)(param->nu*prob->l);	// # of alpha's at upper bound

	for (i = 0; i < n; i++)
		alpha[i] = 1;
	if (n < prob->l)
		alpha[n] = param->nu * prob->l - n;
	for (i = n + 1; i < l; i++)
		alpha[i] = 0;

	for (i = 0; i < l; i++)
	{
		zeros[i] = 0;
		ones[i] = 1;
	}

	Solver s;
	s.Solve(l, ONE_CLASS_Q(*prob, *param), zeros, ones,
		alpha, 1.0, 1.0, param->eps, si, param->shrinking);

	delete[] zeros;
	delete[] ones;
}

static void solve_epsilon_svr(
	const svm_problem *prob, const svm_parameter *param,
	double *alpha, Solver::SolutionInfo* si)
{
	int l = prob->l;
	double *alpha2 = new double[2 * l];
	double *linear_term = new double[2 * l];
	schar *y = new schar[2 * l];
	int i;

	for (i = 0; i < l; i++)
	{
		alpha2[i] = 0;
		linear_term[i] = param->p - prob->y[i];
		y[i] = 1;

		alpha2[i + l] = 0;
		linear_term[i + l] = param->p + prob->y[i];
		y[i + l] = -1;
	}

	Solver s;
	s.Solve(2 * l, SVR_Q(*prob, *param), linear_term, y,
		alpha2, param->C, param->C, param->eps, si, param->shrinking);

	double sum_alpha = 0;
	for (i = 0; i < l; i++)
	{
		alpha[i] = alpha2[i] - alpha2[i + l];
		sum_alpha += fabs(alpha[i]);
	}
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "nu = %f", sum_alpha / (param->C*l));

	delete[] alpha2;
	delete[] linear_term;
	delete[] y;
}

static void solve_nu_svr(
	const svm_problem *prob, const svm_parameter *param,
	double *alpha, Solver::SolutionInfo* si)
{
	int l = prob->l;
	double C = param->C;
	double *alpha2 = new double[2 * l];
	double *linear_term = new double[2 * l];
	schar *y = new schar[2 * l];
	int i;

	double sum = C * param->nu * l / 2;
	for (i = 0; i < l; i++)
	{
		alpha2[i] = alpha2[i + l] = min(sum, C);
		sum -= alpha2[i];

		linear_term[i] = -prob->y[i];
		y[i] = 1;

		linear_term[i + l] = prob->y[i];
		y[i + l] = -1;
	}

	Solver_NU s;
	s.Solve(2 * l, SVR_Q(*prob, *param), linear_term, y,
		alpha2, C, C, param->eps, si, param->shrinking);

	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "epsilon = %f", -si->r);

	for (i = 0; i < l; i++)
		alpha[i] = alpha2[i] - alpha2[i + l];

	delete[] alpha2;
	delete[] linear_term;
	delete[] y;
}

//
// decision_function
//
struct decision_function
{
	double *alpha;
	double rho;
};

static decision_function svm_train_one(
	const svm_problem *prob, const svm_parameter *param,
	double Cp, double Cn)
{
	double *alpha = Malloc(double, prob->l);
	Solver::SolutionInfo si;
	switch (param->svm_type)
	{
	case C_SVC:
		solve_c_svc(prob, param, alpha, &si, Cp, Cn);
		break;
	case NU_SVC:
		solve_nu_svc(prob, param, alpha, &si);
		break;
	case ONE_CLASS:
		solve_one_class(prob, param, alpha, &si);
		break;
	case EPSILON_SVR:
		solve_epsilon_svr(prob, param, alpha, &si);
		break;
	case NU_SVR:
		solve_nu_svr(prob, param, alpha, &si);
		break;
	}

	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "obj = %f, rho = %f", si.obj, si.rho);

	// output SVs

	int nSV = 0;
	int nBSV = 0;
	for (int i = 0; i < prob->l; i++)
	{
		if (fabs(alpha[i]) > 0)
		{
			++nSV;
			if (prob->y[i] > 0)
			{
				if (fabs(alpha[i]) >= si.upper_bound_p)
					++nBSV;
			}
			else
			{
				if (fabs(alpha[i]) >= si.upper_bound_n)
					++nBSV;
			}
		}
	}

	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "nSV = %d, nBSV = %d", nSV, nBSV);

	decision_function f;
	f.alpha = alpha;
	f.rho = si.rho;
	return f;
}

// Platt's binary SVM Probablistic Output: an improvement from Lin et al.
static void sigmoid_train(
	int l, const double *dec_values, const double *labels,
	double& A, double& B)
{
	double prior1 = 0, prior0 = 0;
	int i;

	for (i = 0; i < l; i++)
		if (labels[i] > 0) prior1 += 1;
		else prior0 += 1;

		int max_iter = 100;	// Maximal number of iterations
		double min_step = 1e-10;	// Minimal step taken in line search
		double sigma = 1e-12;	// For numerically strict PD of Hessian
		double eps = 1e-5;
		double hiTarget = (prior1 + 1.0) / (prior1 + 2.0);
		double loTarget = 1 / (prior0 + 2.0);
		double *t = Malloc(double, l);
		double fApB, p, q, h11, h22, h21, g1, g2, det, dA, dB, gd, stepsize;
		double newA, newB, newf, d1, d2;
		int iter;

		// Initial Point and Initial Fun Value
		A = 0.0; B = log((prior0 + 1.0) / (prior1 + 1.0));
		double fval = 0.0;

		for (i = 0; i < l; i++)
		{
			if (labels[i] > 0) t[i] = hiTarget;
			else t[i] = loTarget;
			fApB = dec_values[i] * A + B;
			if (fApB >= 0)
				fval += t[i] * fApB + log(1 + exp(-fApB));
			else
				fval += (t[i] - 1)*fApB + log(1 + exp(fApB));
		}
		for (iter = 0; iter < max_iter; iter++)
		{
			// Update Gradient and Hessian (use H' = H + sigma I)
			h11 = sigma; // numerically ensures strict PD
			h22 = sigma;
			h21 = 0.0; g1 = 0.0; g2 = 0.0;
			for (i = 0; i < l; i++)
			{
				fApB = dec_values[i] * A + B;
				if (fApB >= 0)
				{
					p = exp(-fApB) / (1.0 + exp(-fApB));
					q = 1.0 / (1.0 + exp(-fApB));
				}
				else
				{
					p = 1.0 / (1.0 + exp(fApB));
					q = exp(fApB) / (1.0 + exp(fApB));
				}
				d2 = p*q;
				h11 += dec_values[i] * dec_values[i] * d2;
				h22 += d2;
				h21 += dec_values[i] * d2;
				d1 = t[i] - p;
				g1 += dec_values[i] * d1;
				g2 += d1;
			}

			// Stopping Criteria
			if (fabs(g1) < eps && fabs(g2) < eps)
				break;

			// Finding Newton direction: -inv(H') * g
			det = h11*h22 - h21*h21;
			dA = -(h22*g1 - h21 * g2) / det;
			dB = -(-h21*g1 + h11 * g2) / det;
			gd = g1*dA + g2*dB;


			stepsize = 1;		// Line Search
			while (stepsize >= min_step)
			{
				newA = A + stepsize * dA;
				newB = B + stepsize * dB;

				// New function value
				newf = 0.0;
				for (i = 0; i < l; i++)
				{
					fApB = dec_values[i] * newA + newB;
					if (fApB >= 0)
						newf += t[i] * fApB + log(1 + exp(-fApB));
					else
						newf += (t[i] - 1)*fApB + log(1 + exp(fApB));
				}
				// Check sufficient decrease
				if (newf < fval + 0.0001*stepsize*gd)
				{
					A = newA; B = newB; fval = newf;
					break;
				}
				else
					stepsize = stepsize / 2.0;
			}

			if (stepsize < min_step)
			{
				COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Line search fails in two-class probability estimates");
				break;
			}
		}

		if (iter >= max_iter)
			COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Reaching maximal iterations in two-class probability estimates");
		Free(t);
}

static double sigmoid_predict(double decision_value, double A, double B)
{
	double fApB = decision_value*A + B;
	// 1-p used later; avoid catastrophic cancellation
	if (fApB >= 0)
		return exp(-fApB) / (1.0 + exp(-fApB));
	else
		return 1.0 / (1 + exp(fApB));
}

// Method 2 from the multiclass_prob paper by Wu, Lin, and Weng
static void multiclass_probability(int k, double **r, double *p)
{
	int t, j;
	int iter = 0, max_iter = max(100, k);
	double **Q = Malloc(double *, k);
	double *Qp = Malloc(double, k);
	double pQp, eps = 0.005 / k;

	for (t = 0; t < k; t++) {
		p[t] = 1.0 / k;  // Valid if k = 1
		Q[t] = Malloc(double, k);
		Q[t][t] = 0;
		for (j = 0; j < t; j++) {
			Q[t][t] += r[j][t] * r[j][t];
			Q[t][j] = Q[j][t];
		}
		for (j = t + 1; j < k; j++) {
			Q[t][t] += r[j][t] * r[j][t];
			Q[t][j] = -r[j][t] * r[t][j];
		}
	}
	for (iter = 0; iter < max_iter; iter++) {
		// stopping condition, recalculate QP,pQP for numerical accuracy
		pQp = 0;
		for (t = 0; t < k; t++) {
			Qp[t] = 0;
			for (j = 0; j < k; j++) {
				Qp[t] += Q[t][j] * p[j];
			}
			pQp += p[t] * Qp[t];
		}
		double max_error = 0;
		for (t = 0; t < k; t++) {
			double error = fabs(Qp[t] - pQp);
			if (error > max_error) {
				max_error = error;
			}
		}
		if (max_error < eps) {
			break;
		}

		for (t = 0; t < k; t++) {
			const double diff = (-Qp[t] + pQp) / Q[t][t];
			const double scale = 1.0 / (1 + diff);
			p[t] += diff;
			pQp = (pQp + diff*(diff*Q[t][t] + 2 * Qp[t])) * scale * scale;
			for (j = 0; j < k; j++) {
				Qp[j] = (Qp[j] + diff*Q[t][j]) * scale;
				p[j] *= scale;
			}
		}
	}
	if (iter >= max_iter) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Exceeds max_iter in multiclass_prob");
	}
	for (t = 0; t < k; t++) {
		Free(Q[t]);
	}
	Free(Q);
	Free(Qp);
}

// Cross-validation decision values for probability estimates
static void svm_binary_svc_probability(
	const svm_problem *prob, const svm_parameter *param,
	double Cp, double Cn, double& probA, double& probB)
{
	int i;
	int nr_fold = 5;
	int *perm = Malloc(int, prob->l);
	double *dec_values = Malloc(double, prob->l);

	// random shuffle
	for (i = 0; i < prob->l; i++) perm[i] = i;
	for (i = 0; i < prob->l; i++)
	{
		int j = i + rand() % (prob->l - i);
		swap(perm[i], perm[j]);
	}
	for (i = 0; i < nr_fold; i++)
	{
		int begin = i*prob->l / nr_fold;
		int end = (i + 1)*prob->l / nr_fold;
		int j, k;
		struct svm_problem subprob;

		subprob.l = prob->l - (end - begin);
		subprob.x = Malloc(struct svm_node*, subprob.l);
		subprob.y = Malloc(double, subprob.l);

		k = 0;
		for (j = 0; j < begin; j++)
		{
			subprob.x[k] = prob->x[perm[j]];
			subprob.y[k] = prob->y[perm[j]];
			++k;
		}
		for (j = end; j < prob->l; j++)
		{
			subprob.x[k] = prob->x[perm[j]];
			subprob.y[k] = prob->y[perm[j]];
			++k;
		}
		int p_count = 0, n_count = 0;
		for (j = 0; j < k; j++)
			if (subprob.y[j] > 0)
				p_count++;
			else
				n_count++;

		if (p_count == 0 && n_count == 0)
			for (j = begin; j < end; j++)
				dec_values[perm[j]] = 0;
		else if (p_count > 0 && n_count == 0)
			for (j = begin; j < end; j++)
				dec_values[perm[j]] = 1;
		else if (p_count == 0 && n_count > 0)
			for (j = begin; j < end; j++)
				dec_values[perm[j]] = -1;
		else
		{
			svm_parameter subparam = *param;
			subparam.probability = 0;
			subparam.C = 1.0;
			subparam.nr_weight = 2;
			subparam.weight_label = Malloc(int, 2);
			subparam.weight = Malloc(double, 2);
			subparam.weight_label[0] = +1;
			subparam.weight_label[1] = -1;
			subparam.weight[0] = Cp;
			subparam.weight[1] = Cn;
			struct svm_model *submodel = svm_train(&subprob, &subparam);
#if COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY
			if (subparam.kernel_type == RBF) {
				COMPV_CHECK_CODE_ASSERT(svm_makeSVs_SIMD_frienly(submodel, svm_count(prob->x[perm[begin]]), false));
			}
#endif /* COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY */
			int SIMDFriendly = svm_check_SIMDFriendly_model(submodel);
			CompVMatPtr xMat;
			const size_t xMatCount = SIMDFriendly ? submodel->SVMat->cols() : 0;
			for (j = begin; j < end; j++) {
				if (SIMDFriendly) {
					COMPV_CHECK_CODE_ASSERT(svm_copy(prob->x[perm[j]], &xMat, xMatCount));
				}
				const svm_node_base node(
					SIMDFriendly ? NODE_TYPE_MAT : NODE_TYPE_INDEXED,
					SIMDFriendly ? reinterpret_cast<const void*>(&xMat) : reinterpret_cast<const void*>(prob->x[perm[j]])
				);
				svm_predict_values(submodel, &node, &(dec_values[perm[j]]));
				// ensure +1 -1 order; reason not using CV subroutine
				dec_values[perm[j]] *= submodel->label[0];
			}
			svm_free_and_destroy_model(&submodel);
			svm_destroy_param(&subparam);
		}
		Free(subprob.x);
		Free(subprob.y);
	}
	sigmoid_train(prob->l, dec_values, prob->y, probA, probB);
	Free(dec_values);
	Free(perm);
}

// Return parameter of a Laplace distribution 
static double svm_svr_probability(
	const svm_problem *prob, const svm_parameter *param)
{
	int i;
	int nr_fold = 5;
	double *ymv = Malloc(double, prob->l);
	double mae = 0;

	svm_parameter newparam = *param;
	newparam.probability = 0;
	svm_cross_validation(prob, &newparam, nr_fold, ymv);
	for (i = 0; i < prob->l; i++)
	{
		ymv[i] = prob->y[i] - ymv[i];
		mae += fabs(ymv[i]);
	}
	mae /= prob->l;
	double std = sqrt(2 * mae*mae);
	int count = 0;
	mae = 0;
	for (i = 0; i < prob->l; i++)
		if (fabs(ymv[i]) > 5 * std)
			count = count + 1;
		else
			mae += fabs(ymv[i]);
	mae /= (prob->l - count);
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma= %g", mae);
	Free(ymv);
	return mae;
}


// label: label name, start: begin of each class, count: #data of classes, perm: indices to the original data
// perm, length l, must be allocated before calling this subroutine
static void svm_group_classes(const svm_problem *prob, int *nr_class_ret, int **label_ret, int **start_ret, int **count_ret, int *perm)
{
	int l = prob->l;
	int max_nr_class = 16;
	int nr_class = 0;
	int *label = Malloc(int, max_nr_class);
	int *count = Malloc(int, max_nr_class);
	int *data_label = Malloc(int, l);
	int i;

	for (i = 0; i < l; i++)
	{
		int this_label = (int)prob->y[i];
		int j;
		for (j = 0; j < nr_class; j++)
		{
			if (this_label == label[j])
			{
				++count[j];
				break;
			}
		}
		data_label[i] = j;
		if (j == nr_class)
		{
			if (nr_class == max_nr_class)
			{
				max_nr_class *= 2;
				label = (int *)Realloc(label, max_nr_class * sizeof(int));
				count = (int *)Realloc(count, max_nr_class * sizeof(int));
			}
			label[nr_class] = this_label;
			count[nr_class] = 1;
			++nr_class;
		}
	}

	//
	// Labels are ordered by their first occurrence in the training set. 
	// However, for two-class sets with -1/+1 labels and -1 appears first, 
	// we swap labels to ensure that internally the binary SVM has positive data corresponding to the +1 instances.
	//
	if (nr_class == 2 && label[0] == -1 && label[1] == 1)
	{
		swap(label[0], label[1]);
		swap(count[0], count[1]);
		for (i = 0; i < l; i++)
		{
			if (data_label[i] == 0)
				data_label[i] = 1;
			else
				data_label[i] = 0;
		}
	}

	int *start = Malloc(int, nr_class);
	start[0] = 0;
	for (i = 1; i < nr_class; i++)
		start[i] = start[i - 1] + count[i - 1];
	for (i = 0; i < l; i++)
	{
		perm[start[data_label[i]]] = i;
		++start[data_label[i]];
	}
	start[0] = 0;
	for (i = 1; i < nr_class; i++)
		start[i] = start[i - 1] + count[i - 1];

	*nr_class_ret = nr_class;
	*label_ret = label;
	*start_ret = start;
	*count_ret = count;
	Free(data_label);
}

//
// Interface functions
//
svm_model *svm_train(const svm_problem *prob, const svm_parameter *param)
{
	svm_model *model = Malloc(svm_model, 1);
	CompVMem::copy(model, &libsvm_static_model, sizeof(svm_model));
	model->param = *param;
	model->free_sv = 0;	// XXX

	if (param->svm_type == ONE_CLASS ||
		param->svm_type == EPSILON_SVR ||
		param->svm_type == NU_SVR)
	{
		// regression or one-class-svm
		model->nr_class = 2;
		model->label = nullptr;
		model->nSV = nullptr;
		model->probA = nullptr; model->probB = nullptr;
		model->sv_coef = Malloc(double *, 1);

		if (param->probability &&
			(param->svm_type == EPSILON_SVR ||
				param->svm_type == NU_SVR))
		{
			model->probA = Malloc(double, 1);
			model->probA[0] = svm_svr_probability(prob, param);
		}

		decision_function f = svm_train_one(prob, param, 0, 0);
		model->rho = Malloc(double, 1);
		model->rho[0] = f.rho;

		int nSV = 0;
		int i;
		for (i = 0; i < prob->l; i++)
			if (fabs(f.alpha[i]) > 0) ++nSV;
		model->l = nSV;
		model->SV = Malloc(svm_node *, nSV);
		model->sv_coef[0] = Malloc(double, nSV);
		model->sv_indices = Malloc(int, nSV);
		int j = 0;
		for (i = 0; i < prob->l; i++) {
			if (fabs(f.alpha[i]) > 0) {
				model->SV[j] = prob->x[i];
				model->sv_coef[0][j] = f.alpha[i];
				model->sv_indices[j] = i + 1;
				++j;
			}
		}

		Free(f.alpha);
	}
	else
	{
		// classification
		int l = prob->l;
		int nr_class;
		int *label = nullptr;
		int *start = nullptr;
		int *count = nullptr;
		int *perm = Malloc(int, l);

		// group training data of the same class
		svm_group_classes(prob, &nr_class, &label, &start, &count, perm);
		if (nr_class == 1)
			COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "WARNING: training data in only one class. See README for details.");

		svm_node **x = Malloc(svm_node *, l);
		int i;
		for (i = 0; i < l; i++)
			x[i] = prob->x[perm[i]];

		// calculate weighted C

		double *weighted_C = Malloc(double, nr_class);
		for (i = 0; i < nr_class; i++)
			weighted_C[i] = param->C;
		for (i = 0; i < param->nr_weight; i++)
		{
			int j;
			for (j = 0; j < nr_class; j++)
				if (param->weight_label[i] == label[j])
					break;
			if (j == nr_class) {
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "WARNING: class label %d specified in weight is not found", param->weight_label[i]);
			}
			else {
				weighted_C[j] *= param->weight[i];
			}
		}

		// train k*(k-1)/2 models

		bool *nonzero = Malloc(bool, l);
		for (i = 0; i < l; i++)
			nonzero[i] = false;
		decision_function *f = Malloc(decision_function, nr_class*(nr_class - 1) / 2);

		double *probA = nullptr, *probB = nullptr;
		if (param->probability)
		{
			probA = Malloc(double, nr_class*(nr_class - 1) / 2);
			probB = Malloc(double, nr_class*(nr_class - 1) / 2);
		}

		int p = 0;
		for (i = 0; i < nr_class; i++)
			for (int j = i + 1; j < nr_class; j++)
			{
				svm_problem sub_prob;
				int si = start[i], sj = start[j];
				int ci = count[i], cj = count[j];
				sub_prob.l = ci + cj;
				sub_prob.x = Malloc(svm_node *, sub_prob.l);
				sub_prob.y = Malloc(double, sub_prob.l);
				int k;
				for (k = 0; k < ci; k++)
				{
					sub_prob.x[k] = x[si + k];
					sub_prob.y[k] = +1;
				}
				for (k = 0; k < cj; k++)
				{
					sub_prob.x[ci + k] = x[sj + k];
					sub_prob.y[ci + k] = -1;
				}

				if (param->probability)
					svm_binary_svc_probability(&sub_prob, param, weighted_C[i], weighted_C[j], probA[p], probB[p]);

				f[p] = svm_train_one(&sub_prob, param, weighted_C[i], weighted_C[j]);
				for (k = 0; k < ci; k++)
					if (!nonzero[si + k] && fabs(f[p].alpha[k]) > 0)
						nonzero[si + k] = true;
				for (k = 0; k < cj; k++)
					if (!nonzero[sj + k] && fabs(f[p].alpha[ci + k]) > 0)
						nonzero[sj + k] = true;
				Free(sub_prob.x);
				Free(sub_prob.y);
				++p;
			}

		// build output

		model->nr_class = nr_class;

		model->label = Malloc(int, nr_class);
		for (i = 0; i < nr_class; i++)
			model->label[i] = label[i];

		model->rho = Malloc(double, nr_class*(nr_class - 1) / 2);
		for (i = 0; i < nr_class*(nr_class - 1) / 2; i++)
			model->rho[i] = f[i].rho;

		if (param->probability)
		{
			model->probA = Malloc(double, nr_class*(nr_class - 1) / 2);
			model->probB = Malloc(double, nr_class*(nr_class - 1) / 2);
			for (i = 0; i < nr_class*(nr_class - 1) / 2; i++)
			{
				model->probA[i] = probA[i];
				model->probB[i] = probB[i];
			}
		}
		else
		{
			model->probA = nullptr;
			model->probB = nullptr;
		}

		int total_sv = 0;
		int *nz_count = Malloc(int, nr_class);
		model->nSV = Malloc(int, nr_class);
		for (i = 0; i < nr_class; i++)
		{
			int nSV = 0;
			for (int j = 0; j < count[i]; j++)
				if (nonzero[start[i] + j])
				{
					++nSV;
					++total_sv;
				}
			model->nSV[i] = nSV;
			nz_count[i] = nSV;
		}

		COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "Total nSV = %d", total_sv);

		model->l = total_sv;
		model->SV = Malloc(svm_node *, total_sv);
		model->sv_indices = Malloc(int, total_sv);
		p = 0;
		for (i = 0; i < l; i++)
			if (nonzero[i])
			{
				model->SV[p] = x[i];
				model->sv_indices[p++] = perm[i] + 1;
			}

		int *nz_start = Malloc(int, nr_class);
		nz_start[0] = 0;
		for (i = 1; i < nr_class; i++)
			nz_start[i] = nz_start[i - 1] + nz_count[i - 1];

		model->sv_coef = Malloc(double *, nr_class - 1);
		for (i = 0; i < nr_class - 1; i++)
			model->sv_coef[i] = Malloc(double, total_sv);

		p = 0;
		for (i = 0; i < nr_class; i++)
			for (int j = i + 1; j < nr_class; j++)
			{
				// classifier (i,j): coefficients with
				// i are in sv_coef[j-1][nz_start[i]...],
				// j are in sv_coef[i][nz_start[j]...]

				int si = start[i];
				int sj = start[j];
				int ci = count[i];
				int cj = count[j];

				int q = nz_start[i];
				int k;
				for (k = 0; k < ci; k++)
					if (nonzero[si + k])
						model->sv_coef[j - 1][q++] = f[p].alpha[k];
				q = nz_start[j];
				for (k = 0; k < cj; k++)
					if (nonzero[sj + k])
						model->sv_coef[i][q++] = f[p].alpha[ci + k];
				++p;
			}

		Free(label);
		Free(probA);
		Free(probB);
		Free(count);
		Free(perm);
		Free(start);
		Free(x);
		Free(weighted_C);
		Free(nonzero);
		for (i = 0; i < nr_class*(nr_class - 1) / 2; i++)
			Free(f[i].alpha);
		Free(f);
		Free(nz_count);
		Free(nz_start);
	}
	return model;
}

// Stratified cross validation
void svm_cross_validation(const svm_problem *prob, const svm_parameter *param, int nr_fold, double *target)
{
	int i;
	int *fold_start;
	int l = prob->l;
	int *perm = Malloc(int, l);
	int nr_class;
	if (nr_fold > l)
	{
		nr_fold = l;
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "WARNING: # folds > # data. Will use # folds = # data instead (i.e., leave-one-out cross validation)");
	}
	fold_start = Malloc(int, nr_fold + 1);
	// stratified cv may not give leave-one-out rate
	// Each class to l folds -> some folds may have zero elements
	if ((param->svm_type == C_SVC ||
		param->svm_type == NU_SVC) && nr_fold < l)
	{
		int *start = nullptr;
		int *label = nullptr;
		int *count = nullptr;
		svm_group_classes(prob, &nr_class, &label, &start, &count, perm);

		// random shuffle and then data grouped by fold using the array perm
		int *fold_count = Malloc(int, nr_fold);
		int c;
		int *index = Malloc(int, l);
		for (i = 0; i < l; i++)
			index[i] = perm[i];
		for (c = 0; c < nr_class; c++)
			for (i = 0; i < count[c]; i++)
			{
				int j = i + rand() % (count[c] - i);
				swap(index[start[c] + j], index[start[c] + i]);
			}
		for (i = 0; i < nr_fold; i++)
		{
			fold_count[i] = 0;
			for (c = 0; c < nr_class; c++)
				fold_count[i] += (i + 1)*count[c] / nr_fold - i*count[c] / nr_fold;
		}
		fold_start[0] = 0;
		for (i = 1; i <= nr_fold; i++)
			fold_start[i] = fold_start[i - 1] + fold_count[i - 1];
		for (c = 0; c < nr_class; c++)
			for (i = 0; i < nr_fold; i++)
			{
				int begin = start[c] + i*count[c] / nr_fold;
				int end = start[c] + (i + 1)*count[c] / nr_fold;
				for (int j = begin; j < end; j++)
				{
					perm[fold_start[i]] = index[j];
					fold_start[i]++;
				}
			}
		fold_start[0] = 0;
		for (i = 1; i <= nr_fold; i++)
			fold_start[i] = fold_start[i - 1] + fold_count[i - 1];
		Free(start);
		Free(label);
		Free(count);
		Free(index);
		Free(fold_count);
	}
	else
	{
		for (i = 0; i < l; i++) perm[i] = i;
		for (i = 0; i < l; i++)
		{
			int j = i + rand() % (l - i);
			swap(perm[i], perm[j]);
		}
		for (i = 0; i <= nr_fold; i++)
			fold_start[i] = i*l / nr_fold;
	}

	for (i = 0; i < nr_fold; i++)
	{
		int begin = fold_start[i];
		int end = fold_start[i + 1];
		int j, k;
		struct svm_problem subprob;

		subprob.l = l - (end - begin);
		subprob.x = Malloc(struct svm_node*, subprob.l);
		subprob.y = Malloc(double, subprob.l);

		k = 0;
		for (j = 0; j < begin; j++)
		{
			subprob.x[k] = prob->x[perm[j]];
			subprob.y[k] = prob->y[perm[j]];
			++k;
		}
		for (j = end; j < l; j++)
		{
			subprob.x[k] = prob->x[perm[j]];
			subprob.y[k] = prob->y[perm[j]];
			++k;
		}
		struct svm_model *submodel = svm_train(&subprob, param);
#if COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY
		if (param->kernel_type == RBF) {
			COMPV_CHECK_CODE_ASSERT(svm_makeSVs_SIMD_frienly(submodel, svm_count(prob->x[perm[begin]]), false));
		}
#endif /* COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY */
		int SIMDFriendly = svm_check_SIMDFriendly_model(submodel);
		CompVMatPtr xMat;
		const size_t xMatCount = SIMDFriendly ? submodel->SVMat->cols() : 0;
		if (param->probability &&
			(param->svm_type == C_SVC || param->svm_type == NU_SVC))
		{
			double *prob_estimates = Malloc(double, svm_get_nr_class(submodel));
			for (j = begin; j < end; j++) {
				if (SIMDFriendly) {
					COMPV_CHECK_CODE_ASSERT(svm_copy(prob->x[perm[j]], &xMat, xMatCount));
				}
				const svm_node_base node(
					SIMDFriendly ? NODE_TYPE_MAT: NODE_TYPE_INDEXED,
					SIMDFriendly ? reinterpret_cast<const void*>(&xMat) : reinterpret_cast<const void*>(prob->x[perm[j]])
				);
				target[perm[j]] = svm_predict_probability(submodel, &node, prob_estimates);
			}
			Free(prob_estimates);
		}
		else {
			for (j = begin; j < end; j++) {
				if (SIMDFriendly) {
					COMPV_CHECK_CODE_ASSERT(svm_copy(prob->x[perm[j]], &xMat, xMatCount));
				}
				const svm_node_base node(
					SIMDFriendly ? NODE_TYPE_MAT : NODE_TYPE_INDEXED,
					SIMDFriendly ? reinterpret_cast<const void*>(&xMat) : reinterpret_cast<const void*>(prob->x[perm[j]])
				);
				target[perm[j]] = svm_predict(submodel, &node);
			}
		}
		svm_free_and_destroy_model(&submodel);
		Free(subprob.x);
		Free(subprob.y);
	}
	Free(fold_start);
	Free(perm);
}


int svm_get_svm_type(const svm_model *model)
{
	return model->param.svm_type;
}

int svm_get_nr_class(const svm_model *model)
{
	return model->nr_class;
}

void svm_get_labels(const svm_model *model, int* label)
{
	if (model->label != nullptr)
		for (int i = 0; i < model->nr_class; i++)
			label[i] = model->label[i];
}

void svm_get_sv_indices(const svm_model *model, int* indices)
{
	if (model->sv_indices != nullptr)
		for (int i = 0; i < model->l; i++)
			indices[i] = model->sv_indices[i];
}

int svm_get_nr_sv(const svm_model *model)
{
	return model->l;
}

double svm_get_svr_probability(const svm_model *model)
{
	if ((model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) &&
		model->probA != nullptr)
		return model->probA[0];
	else
	{
		COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "Model doesn't contain information for SVR probability inference");
		return 0;
	}
}

double svm_predict_values(const svm_model *model, const svm_node_base *xx, double* dec_values)
{
	int i;
	const svm_simd_func_ptrs& simd_func_ptrs = model->simd_func_ptrs;
	if (model->param.svm_type == ONE_CLASS ||
		model->param.svm_type == EPSILON_SVR ||
		model->param.svm_type == NU_SVR)
	{
		double *sv_coef = model->sv_coef[0];
		double sum = 0;
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found for 'Kernel::k_function' -> https://www.csie.ntu.edu.tw/~cjlin/libsvm/faq.html#f432");
		if (xx->type == NODE_TYPE_INDEXED) {
			const svm_node* x = reinterpret_cast<const svm_node*>(xx->node);
			for (i = 0; i < model->l; i++) {
				sum += sv_coef[i] * Kernel::k_function(x, model->SV[i], model->param);
			}
		}
		else {
			CompVMatPtr kvalueMat;
			COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<double>(&kvalueMat, 1, model->l));
			Kernel::k_function(*reinterpret_cast<const CompVMatPtr*>(xx->node), model->SVMat, static_cast<size_t>(model->l), model->param, kvalueMat, &model->simd_func_ptrs);
			simd_func_ptrs.dot_64f64f(sv_coef, kvalueMat->data<const double>(), static_cast<compv_uscalar_t>(model->l), 1, 0, 0, &sum);
		}
		sum -= model->rho[0];
		*dec_values = sum;

		if (model->param.svm_type == ONE_CLASS) {
			return (sum > 0) ? 1 : -1;
		}
		else {
			return sum;
		}
	}
	else
	{
		int nr_class = model->nr_class;
		int l = model->l;

		CompVMatPtr kvalueMat;
		COMPV_CHECK_CODE_ASSERT(CompVMat::newObjAligned<double>(&kvalueMat, 1, l));
		double* kvaluePtr = kvalueMat->data<double>();
		if (xx->type == NODE_TYPE_INDEXED) {
			const svm_node* x = reinterpret_cast<const svm_node*>(xx->node);
			for (i = 0; i < l; i++) {
				kvaluePtr[i] = Kernel::k_function(x, model->SV[i], model->param);
			}
		}
		else {
			Kernel::k_function(*reinterpret_cast<const CompVMatPtr*>(xx->node), model->SVMat, static_cast<size_t>(model->l), model->param, kvalueMat, &model->simd_func_ptrs);
		}

		int *start = Malloc(int, nr_class);
		start[0] = 0;
		for (i = 1; i < nr_class; i++) {
			start[i] = start[i - 1] + model->nSV[i - 1];
		}

		int *vote = reinterpret_cast<int*>(Calloc(nr_class, sizeof(int)));
		int p = 0;
		for (i = 0; i < nr_class; i++) {
			for (int j = i + 1; j < nr_class; j++) {
				double sumi, sumj;
				const int& si = start[i];
				const int& sj = start[j];
				const int& ci = model->nSV[i];
				const int& cj = model->nSV[j];
				const double *coef1 = model->sv_coef[j - 1] + si;
				const double *coef2 = model->sv_coef[i] + sj;
				simd_func_ptrs.dot_64f64f(coef1, &kvaluePtr[si], static_cast<compv_uscalar_t>(ci), 1, 0, 0, &sumi);
				simd_func_ptrs.dot_64f64f(coef2, &kvaluePtr[sj], static_cast<compv_uscalar_t>(cj), 1, 0, 0, &sumj);
				dec_values[p] = (sumi + sumj) - model->rho[p];
				if (dec_values[p] > 0) {
					++vote[i];
				}
				else {
					++vote[j];
				}
				p++;
			}
		}

		int vote_max_idx = 0;
		for (i = 1; i < nr_class; i++) {
			if (vote[i] > vote[vote_max_idx]) {
				vote_max_idx = i;
			}
		}

		Free(start);
		Free(vote);
		return model->label[vote_max_idx];
	}
}

double svm_predict(const svm_model *model, const svm_node_base *x)
{
	int nr_class = model->nr_class;
	double *dec_values;
	if (model->param.svm_type == ONE_CLASS ||
		model->param.svm_type == EPSILON_SVR ||
		model->param.svm_type == NU_SVR)
		dec_values = Malloc(double, 1);
	else
		dec_values = Malloc(double, nr_class*(nr_class - 1) / 2);
	double pred_result = svm_predict_values(model, x, dec_values);
	Free(dec_values);
	return pred_result;
}

double svm_predict_distance(const svm_model *model, const svm_node_base *x, double *distance)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Not fully tested yet");
	if ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) && model->probA && model->probB) {
		int nr_class = model->nr_class;
		double *dec_values = Malloc(double, nr_class*(nr_class - 1) / 2);
		const int pred_label = static_cast<int>(svm_predict_values(model, x, dec_values)); // this the label (model->label[x]), not the class
		int pred_class = INT_MAX;
		for (int i = 0; i < nr_class; i++) {
			if (model->label[i] == pred_label) {
				pred_class = i;
				break;
			}
		}
		COMPV_ASSERT(pred_class < nr_class);
		double* fApB_values = Malloc(double, nr_class - 1);
		int* fApB_signs = Malloc(int, nr_class - 1);
		int* dec_signs = Malloc(int, nr_class - 1);
		
		int FIXME_COUNT = 0;
		for (int i = 0, k = 0; i < nr_class; i++) {
			const bool bi = (i == pred_class);
			for (int j = i + 1; j < nr_class; j++, k++) {
				const bool bj = (j == pred_class);
				if (bi || bj) {
					const double fApB = dec_values[k] * model->probA[k] + model->probB[k];
					fApB_signs[FIXME_COUNT] = (fApB >= 0);
					fApB_values[FIXME_COUNT] = fApB_signs[FIXME_COUNT] ? -fApB : fApB;
					dec_signs[FIXME_COUNT] = (dec_values[k] < 0);
					++FIXME_COUNT;
				}
			}
		}

		COMPV_ASSERT(FIXME_COUNT == (nr_class - 1));

		model->simd_func_ptrs.expo(fApB_values, fApB_values, static_cast<size_t>(FIXME_COUNT));
		double prob_sum = 0.0;
		for (int i = 0; i < FIXME_COUNT; ++i) {
			const double& exp_fApB = fApB_values[i];
			const double exp_fApB_scale = 1.0 / (1.0 + exp_fApB);
			const double prob = fApB_signs[i]
				? exp_fApB * exp_fApB_scale
				: exp_fApB_scale;
			prob_sum += dec_signs[i]
				? prob
				: (1 - prob);
		}
		
		*distance =  (prob_sum / double(FIXME_COUNT));

		Free(fApB_signs);
		Free(dec_signs);
		Free(fApB_values);
		Free(dec_values);
		return pred_label;
	}
	else {
		return svm_predict(model, x);
	}
}

double svm_predict_probability(
	const svm_model *model, const svm_node_base *x, double *prob_estimates)
{
	if ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) &&
		model->probA != nullptr && model->probB != nullptr)
	{
		int i;
		int nr_class = model->nr_class;
		double *dec_values = Malloc(double, nr_class*(nr_class - 1) / 2);
		svm_predict_values(model, x, dec_values);

		double min_prob = 1e-7;
		double **pairwise_prob = Malloc(double *, nr_class);
		for (i = 0; i < nr_class; i++) {
			pairwise_prob[i] = Malloc(double, nr_class);
		}
		int k = 0;
		for (i = 0; i < nr_class; i++) {
			for (int j = i + 1; j < nr_class; j++) {
				pairwise_prob[i][j] = min(max(sigmoid_predict(dec_values[k], model->probA[k], model->probB[k]), min_prob), 1 - min_prob);
				pairwise_prob[j][i] = 1 - pairwise_prob[i][j];
				k++;
			}
		}
		if (nr_class == 2) {
			prob_estimates[0] = pairwise_prob[0][1];
			prob_estimates[1] = pairwise_prob[1][0];
		}
		else {
			multiclass_probability(nr_class, pairwise_prob, prob_estimates);
		}

		int prob_max_idx = 0;
		for (i = 1; i < nr_class; i++) {
			if (prob_estimates[i] > prob_estimates[prob_max_idx]) {
				prob_max_idx = i;
			}
		}

		for (i = 0; i < nr_class; i++) {
			Free(pairwise_prob[i]);
		}

		Free(dec_values);
		Free(pairwise_prob);
		return model->label[prob_max_idx];
	}
	else {
		return svm_predict(model, x);
	}
}

static const char *svm_type_table[] =
{
	"c_svc","nu_svc","one_class","epsilon_svr","nu_svr",nullptr
};

static const char *kernel_type_table[] =
{
	"linear","polynomial","rbf","sigmoid","precomputed",nullptr
};

int svm_save_model(const char *model_file_name, const svm_model *model)
{
	FILE *fp = COMPV_NAMESPACE::CompVFileUtils::open(model_file_name, "w");
	if (fp == nullptr) return -1;

	char *old_locale = setlocale(LC_ALL, nullptr);
	if (old_locale) {
		old_locale = strdup(old_locale);
	}
	setlocale(LC_ALL, "C");

	const svm_parameter& param = model->param;

	fprintf(fp, "svm_type %s\n", svm_type_table[param.svm_type]);
	fprintf(fp, "kernel_type %s\n", kernel_type_table[param.kernel_type]);

	if (param.kernel_type == POLY)
		fprintf(fp, "degree %d\n", param.degree);

	if (param.kernel_type == POLY || param.kernel_type == RBF || param.kernel_type == SIGMOID)
		fprintf(fp, "gamma %g\n", param.gamma);

	if (param.kernel_type == POLY || param.kernel_type == SIGMOID)
		fprintf(fp, "coef0 %g\n", param.coef0);

	int nr_class = model->nr_class;
	int l = model->l;
	fprintf(fp, "nr_class %d\n", nr_class);
	fprintf(fp, "total_sv %d\n", l);

	{
		fprintf(fp, "rho");
		for (int i = 0; i < nr_class*(nr_class - 1) / 2; i++)
			fprintf(fp, " %g", model->rho[i]);
		fprintf(fp, "\n");
	}

	if (model->label)
	{
		fprintf(fp, "label");
		for (int i = 0; i < nr_class; i++)
			fprintf(fp, " %d", model->label[i]);
		fprintf(fp, "\n");
	}

	if (model->probA) // regression has probA only
	{
		fprintf(fp, "probA");
		for (int i = 0; i < nr_class*(nr_class - 1) / 2; i++)
			fprintf(fp, " %g", model->probA[i]);
		fprintf(fp, "\n");
	}
	if (model->probB)
	{
		fprintf(fp, "probB");
		for (int i = 0; i < nr_class*(nr_class - 1) / 2; i++)
			fprintf(fp, " %g", model->probB[i]);
		fprintf(fp, "\n");
	}

	if (model->nSV)
	{
		fprintf(fp, "nr_sv");
		for (int i = 0; i < nr_class; i++)
			fprintf(fp, " %d", model->nSV[i]);
		fprintf(fp, "\n");
	}

	fprintf(fp, "SV\n");
	const double * const *sv_coef = model->sv_coef;
	const svm_node * const *SV = model->SV;

	for (int i = 0; i < l; i++)
	{
		for (int j = 0; j < nr_class - 1; j++)
			fprintf(fp, "%.16g ", sv_coef[j][i]);

		const svm_node *p = SV[i];

		if (param.kernel_type == PRECOMPUTED)
			fprintf(fp, "0:%d ", (int)(p->value));
		else
			while (p->index != -1)
			{
				fprintf(fp, "%d:%.8g ", p->index, p->value);
				p++;
			}
		fprintf(fp, "\n");
	}

	setlocale(LC_ALL, old_locale);
	::free(old_locale); // allocate using "strdup", so use standard free

	if (ferror(fp) != 0 || (COMPV_NAMESPACE::CompVFileUtils::close(&fp) != COMPV_NAMESPACE::COMPV_ERROR_CODE_S_OK)) return -1;
	else return 0;
}

static char *line = nullptr;
static int max_line_len;

static char* readline(FILE *input)
{
	int len;

	if (fgets(line, max_line_len, input) == nullptr)
		return nullptr;

	while (strrchr(line, '\n') == nullptr)
	{
		max_line_len *= 2;
		line = (char *)Realloc(line, max_line_len);
		len = (int)strlen(line);
		if (fgets(line + len, max_line_len - len, input) == nullptr)
			break;
	}
	return line;
}

//
// FSCANF helps to handle fscanf failures.
// Its do-while block avoids the ambiguity when
// if (...)
//    FSCANF();
// is used
//
#define FSCANF(_stream, _format, _var) do{ if (fscanf(_stream, _format, _var) != 1) return false; }while(0)
bool read_model_header(FILE *fp, svm_model* model)
{
	svm_parameter& param = model->param;
	// parameters for training only won't be assigned, but arrays are assigned as nullptr for safety
	param.nr_weight = 0;
	param.weight_label = nullptr;
	param.weight = nullptr;

	char cmd[81];
	while (1)
	{
		FSCANF(fp, "%80s", cmd);

		if (strcmp(cmd, "svm_type") == 0)
		{
			FSCANF(fp, "%80s", cmd);
			int i;
			for (i = 0; svm_type_table[i]; i++)
			{
				if (strcmp(svm_type_table[i], cmd) == 0)
				{
					param.svm_type = i;
					break;
				}
			}
			if (svm_type_table[i] == nullptr)
			{
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "unknown svm type.");
				return false;
			}
		}
		else if (strcmp(cmd, "kernel_type") == 0)
		{
			FSCANF(fp, "%80s", cmd);
			int i;
			for (i = 0; kernel_type_table[i]; i++)
			{
				if (strcmp(kernel_type_table[i], cmd) == 0)
				{
					param.kernel_type = i;
					break;
				}
			}
			if (kernel_type_table[i] == nullptr)
			{
				COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "unknown kernel function.");
				return false;
			}
		}
		else if (strcmp(cmd, "degree") == 0)
			FSCANF(fp, "%d", &param.degree);
		else if (strcmp(cmd, "gamma") == 0)
			FSCANF(fp, "%lf", &param.gamma);
		else if (strcmp(cmd, "coef0") == 0)
			FSCANF(fp, "%lf", &param.coef0);
		else if (strcmp(cmd, "nr_class") == 0)
			FSCANF(fp, "%d", &model->nr_class);
		else if (strcmp(cmd, "total_sv") == 0)
			FSCANF(fp, "%d", &model->l);
		else if (strcmp(cmd, "rho") == 0)
		{
			int n = model->nr_class * (model->nr_class - 1) / 2;
			model->rho = Malloc(double, n);
			for (int i = 0; i < n; i++)
				FSCANF(fp, "%lf", &model->rho[i]);
		}
		else if (strcmp(cmd, "label") == 0)
		{
			int n = model->nr_class;
			model->label = Malloc(int, n);
			for (int i = 0; i < n; i++)
				FSCANF(fp, "%d", &model->label[i]);
		}
		else if (strcmp(cmd, "probA") == 0)
		{
			int n = model->nr_class * (model->nr_class - 1) / 2;
			model->probA = Malloc(double, n);
			for (int i = 0; i < n; i++)
				FSCANF(fp, "%lf", &model->probA[i]);
		}
		else if (strcmp(cmd, "probB") == 0)
		{
			int n = model->nr_class * (model->nr_class - 1) / 2;
			model->probB = Malloc(double, n);
			for (int i = 0; i < n; i++)
				FSCANF(fp, "%lf", &model->probB[i]);
		}
		else if (strcmp(cmd, "nr_sv") == 0)
		{
			int n = model->nr_class;
			model->nSV = Malloc(int, n);
			for (int i = 0; i < n; i++)
				FSCANF(fp, "%d", &model->nSV[i]);
		}
		else if (strcmp(cmd, "SV") == 0)
		{
			while (1)
			{
				int c = getc(fp);
				if (c == EOF || c == '\n') break;
			}
			break;
		}
		else
		{
			COMPV_DEBUG_WARN_EX(COMPV_THIS_CLASSNAME, "unknown text in model file: [%s]", cmd);
			return false;
		}
	}

	return true;

}

svm_model *svm_load_model(const char *model_file_name)
{
	FILE *fp = COMPV_NAMESPACE::CompVFileUtils::open(model_file_name, "rb");
	if (fp == nullptr) return nullptr;

	char *old_locale = setlocale(LC_ALL, nullptr);
	if (old_locale) {
		old_locale = strdup(old_locale);
	}
	setlocale(LC_ALL, "C");

	// read parameters

	svm_model *model = Malloc(svm_model, 1);
	CompVMem::copy(model, &libsvm_static_model, sizeof(svm_model));

	// read header
	if (!read_model_header(fp, model))
	{
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "ERROR: fscanf failed to read model");
		setlocale(LC_ALL, old_locale);
		::free(old_locale); // Allocated using "strdup" which means must use standard "free"
		Free(model->rho);
		Free(model->label);
		Free(model->nSV);
		Free(model);
		return nullptr;
	}

	// read sv_coef and SV

	int elements = 0;
	long pos = ftell(fp);

	max_line_len = 1024;
	line = Malloc(char, max_line_len);
	char *p, *endptr, *idx, *val;

	while (readline(fp) != nullptr)
	{
		p = strtok(line, ":");
		while (1)
		{
			p = strtok(nullptr, ":");
			if (p == nullptr)
				break;
			++elements;
		}
	}
	elements += model->l;

	fseek(fp, pos, SEEK_SET);

	int m = model->nr_class - 1;
	int l = model->l;
	model->sv_coef = Malloc(double *, m);
	int i;
	for (i = 0; i < m; i++)
		model->sv_coef[i] = Malloc(double, l);
	model->SV = Malloc(svm_node*, l);
	svm_node *x_space = nullptr;
	if (l > 0) x_space = Malloc(svm_node, elements);

	int j = 0;
	for (i = 0; i < l; i++)
	{
		readline(fp);
		model->SV[i] = &x_space[j];

		p = strtok(line, " \t");
		model->sv_coef[0][i] = strtod(p, &endptr);
		for (int k = 1; k < m; k++)
		{
			p = strtok(nullptr, " \t");
			model->sv_coef[k][i] = strtod(p, &endptr);
		}

		while (1)
		{
			idx = strtok(nullptr, ":");
			val = strtok(nullptr, " \t");

			if (val == nullptr)
				break;
			x_space[j].index = (int)strtol(idx, &endptr, 10);
			x_space[j].value = strtod(val, &endptr);

			++j;
		}
		x_space[j++].index = -1;
	}
	Free(line);

	setlocale(LC_ALL, old_locale);
	::free(old_locale); // Allocated using "strdup" which means must use standard "free"

	if (ferror(fp) != 0 || (COMPV_NAMESPACE::CompVFileUtils::close(&fp) != COMPV_NAMESPACE::COMPV_ERROR_CODE_S_OK))
		return nullptr;

	model->free_sv = 1;	// XXX
	return model;
}

void svm_free_model_content(svm_model* model_ptr)
{
	if (model_ptr->free_sv) {
		Free(model_ptr->SV[0]);
	}
	if (model_ptr->sv_coef)
	{
		for (int i = 0; i < model_ptr->nr_class - 1; i++) {
			Free(model_ptr->sv_coef[i]);
		}
	}
	model_ptr->SVMat = nullptr;

	Free(model_ptr->SV);
	Free(model_ptr->sv_coef);
	Free(model_ptr->rho);
	Free(model_ptr->label);
	Free(model_ptr->probA);
	Free(model_ptr->probB);
	Free(model_ptr->sv_indices);
	Free(model_ptr->nSV);
}

void svm_free_and_destroy_model(svm_model** model_ptr_ptr)
{
	if (model_ptr_ptr && *model_ptr_ptr) {
		svm_free_model_content(*model_ptr_ptr);
		Free(*model_ptr_ptr);
	}
}

void svm_destroy_param(svm_parameter* param)
{
	Free(param->weight_label);
	Free(param->weight);
}

const char *svm_check_parameter(const svm_problem *prob, const svm_parameter *param)
{
	// svm_type

	int svm_type = param->svm_type;
	if (svm_type != C_SVC &&
		svm_type != NU_SVC &&
		svm_type != ONE_CLASS &&
		svm_type != EPSILON_SVR &&
		svm_type != NU_SVR)
		return "unknown svm type";

	// kernel_type, degree

	int kernel_type = param->kernel_type;
	if (kernel_type != LINEAR &&
		kernel_type != POLY &&
		kernel_type != RBF &&
		kernel_type != SIGMOID &&
		kernel_type != PRECOMPUTED)
		return "unknown kernel type";

	if (param->gamma < 0)
		return "gamma < 0";

	if (param->degree < 0)
		return "degree of polynomial kernel < 0";

	// cache_size,eps,C,nu,p,shrinking

	if (param->cache_size <= 0)
		return "cache_size <= 0";

	if (param->eps <= 0)
		return "eps <= 0";

	if (svm_type == C_SVC ||
		svm_type == EPSILON_SVR ||
		svm_type == NU_SVR)
		if (param->C <= 0)
			return "C <= 0";

	if (svm_type == NU_SVC ||
		svm_type == ONE_CLASS ||
		svm_type == NU_SVR)
		if (param->nu <= 0 || param->nu > 1)
			return "nu <= 0 or nu > 1";

	if (svm_type == EPSILON_SVR)
		if (param->p < 0)
			return "p < 0";

	if (param->shrinking != 0 &&
		param->shrinking != 1)
		return "shrinking != 0 and shrinking != 1";

	if (param->probability != 0 &&
		param->probability != 1)
		return "probability != 0 and probability != 1";

	if (param->probability == 1 &&
		svm_type == ONE_CLASS)
		return "one-class SVM probability output not supported yet";


	// check whether nu-svc is feasible

	if (svm_type == NU_SVC)
	{
		int l = prob->l;
		int max_nr_class = 16;
		int nr_class = 0;
		int *label = Malloc(int, max_nr_class);
		int *count = Malloc(int, max_nr_class);

		int i;
		for (i = 0; i < l; i++)
		{
			int this_label = (int)prob->y[i];
			int j;
			for (j = 0; j < nr_class; j++)
				if (this_label == label[j])
				{
					++count[j];
					break;
				}
			if (j == nr_class)
			{
				if (nr_class == max_nr_class)
				{
					max_nr_class *= 2;
					label = (int *)Realloc(label, max_nr_class * sizeof(int));
					count = (int *)Realloc(count, max_nr_class * sizeof(int));
				}
				label[nr_class] = this_label;
				count[nr_class] = 1;
				++nr_class;
			}
		}

		for (i = 0; i < nr_class; i++)
		{
			int n1 = count[i];
			for (int j = i + 1; j < nr_class; j++)
			{
				int n2 = count[j];
				if (param->nu*(n1 + n2) / 2 > min(n1, n2))
				{
					Free(label);
					Free(count);
					return "specified nu is infeasible";
				}
			}
		}
		Free(label);
		Free(count);
	}

	return nullptr;
}

int svm_check_probability_model(const svm_model *model)
{
	return ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) &&
		model->probA != nullptr && model->probB != nullptr) ||
		((model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) &&
			model->probA != nullptr);
}

int svm_check_SIMDFriendly_model(const struct svm_model *model)
{
	return (model->SVMat != nullptr) ? 1 : 0;
}

size_t svm_count(const struct svm_node *x)
{
	size_t cc = 0;
	while (x[cc++].index != -1);
	return (cc - 1);
}

COMPV_ERROR_CODE svm_copy(const struct svm_node *x, CompVMatPtrPtr xMat, size_t count COMPV_DEFAULT(0))
{
	COMPV_CHECK_EXP_RETURN(!x || !xMat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (!count) {
		count = svm_count(x);
	}
	COMPV_CHECK_EXP_RETURN(x[count].index != -1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(xMat, 1, count));
	double* xMatPtr = (*xMat)->data<double>();
	for (size_t i = 0; i < count; ++i) {
		xMatPtr[i] = x[i].value;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Not optiz: This function must be called at load phase only
COMPV_ERROR_CODE svm_makeSVs_SIMD_frienly(struct svm_model *model, const size_t expectedSVsize, const bool freeSV COMPV_DEFAULT(true))
{
	COMPV_DEBUG_VERBOSE_EX(COMPV_THIS_CLASSNAME, "This function must be called at trainning phase only");
	COMPV_CHECK_EXP_RETURN(!model || !model->l || !model->SV || !*model->SV, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t count = static_cast<size_t>(model->l);
	CompVMatPtr& SVMat = model->SVMat;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&SVMat, count, expectedSVsize));
	const size_t stride = SVMat->stride();
	
	auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		double* SVMatPtr = SVMat->ptr<double>(start);
		for (size_t i = start; i < end; ++i) {
			const struct svm_node *sv = model->SV[i];
			COMPV_ASSERT(sv[expectedSVsize].index == -1);
			for (size_t j = 0; j < expectedSVsize; ++j) {
				SVMatPtr[j] = sv[j].value;
			}
			SVMatPtr += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		1,
		count,
		1
	));

	// Free SV
	if (freeSV) {
		Free(model->SV[0]);
		Free(model->SV);
	}

	// Update SIMD function ptrs using CPU flags
	model->simd_func_ptrs.init();

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE svm_k_function_rbf(const CompVMatPtr& x, const CompVMatPtr& yy, const size_t count, const double& gamma, CompVMatPtr& kvalues, const struct svm_simd_func_ptrs *simd_func_ptrs)
{
	COMPV_ASSERT(x && yy && kvalues && kvalues->cols() == count && yy->rows() >= count && yy->cols() == x->cols() && x->rows() == 1 
		&& kvalues->subType() == COMPV_SUBTYPE_RAW_FLOAT64 && x->subType() == COMPV_SUBTYPE_RAW_FLOAT64 && yy->subType() == COMPV_SUBTYPE_RAW_FLOAT64
	);

	CompVMatPtr sumMat;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&sumMat, 1, count));
	double* sumMatPtr = sumMat->ptr<double>();

	const compv_uscalar_t xsize = static_cast<compv_uscalar_t>(x->cols());
	const double* xPtr = x->data<const double>(); // always SIMD-aligned and strided
	double* kvaluesPtr = kvalues->data<double>(); // always SIMD-aligned and strided
	const double& gamma_minus = -gamma;
	const size_t ystride = yy->stride();
	// MT model: https://www.csie.ntu.edu.tw/~cjlin/libsvm/faq.html#f432
	auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		const double* yPtr = yy->ptr<const double>(start); // always SIMD-aligned and strided
		for (size_t j = start; j < end; ++j) {
			simd_func_ptrs->dotSub_64f64f(xPtr, yPtr, xsize, 1, 0, 0, &sumMatPtr[j]);
			yPtr += ystride;
		}
		simd_func_ptrs->scale_64f64f(&sumMatPtr[start], &kvaluesPtr[start], (end - start), 1, 0, &gamma_minus); // not aligned-copy (decause of start-indexing)
		simd_func_ptrs->expo(&kvaluesPtr[start], &kvaluesPtr[start], (end - start)); // not aligned-copy (decause of start-indexing)
		
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		1,
		count,
		1
	));

	return COMPV_ERROR_CODE_S_OK;
}

svm_model::svm_model()
{

}

svm_simd_func_ptrs::svm_simd_func_ptrs()
{
	init();
}

void svm_simd_func_ptrs::init()
{
	/* From external code */
	COMPV_CHECK_CODE_ASSERT(CompVMathDot::hookDotSub_64f(&dotSub_64f64f));
	COMPV_CHECK_CODE_ASSERT(CompVMathDot::hookDot_64f(&dot_64f64f));
	COMPV_CHECK_CODE_ASSERT(CompVMathScale::hookScale_64f(&scale_64f64f));
	COMPV_CHECK_CODE_ASSERT(CompVMathExp::hookExp_64f(&exp_64f64f_minpackx, &exp_64f64f_minpack));

	/* From local code - C */
	kernel_rbf0_out_64f64f = Kernel::svm_kernel_rbf0_out_C;
	kernel_rbf1_out_Step1_64f64f = Kernel::svm_kernel_rbf1_out_Step1_C;
	kernel_rbf1_out_Step2_64f32f = Kernel::svm_kernel_rbf1_out_Step2_C;

	/* From local code - SIMD */
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(kernel_rbf0_out_64f64f = CompVLibSVM322KernelRbf0Out_64f64f_SSE2);
		COMPV_EXEC_IFDEF_INTRIN_X86(kernel_rbf1_out_Step1_64f64f = CompVLibSVM322KernelRbf1Out_Step1_64f64f_SSE2);
		COMPV_EXEC_IFDEF_INTRIN_X86(kernel_rbf1_out_Step2_64f32f = CompVLibSVM322KernelRbf1Out_Step2_64f32f_SSE2);
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(kernel_rbf0_out_64f64f = CompVLibSVM322KernelRbf0Out_64f64f_AVX);
		COMPV_EXEC_IFDEF_INTRIN_X86(kernel_rbf1_out_Step1_64f64f = CompVLibSVM322KernelRbf1Out_Step1_64f64f_AVX);
		COMPV_EXEC_IFDEF_INTRIN_X86(kernel_rbf1_out_Step2_64f32f = CompVLibSVM322KernelRbf1Out_Step2_64f32f_AVX);
	}
#elif COMPV_ARCH_ARM
#endif
}
