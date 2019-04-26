// Code source based on libsvm 322 but modified to add support for multi-threading and SIMD (SSE, AVX and NEON)
#ifndef _LIBSVM_H
#define _LIBSVM_H

#define LIBSVM_VERSION 322

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"
#include "compv/base/compv_common.h"
#include "compv/base/math/compv_math_exp.h"

#include <cmath> /* std::exp */

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int libsvm_version;
using namespace COMPV_NAMESPACE;

#if !defined(COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY)
#	define COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY		1
#endif /* COMPV_MACHINE_LEARNING_SVM_MAKE_SIMD_FRIENDLY */

enum { NODE_TYPE_INDEXED, NODE_TYPE_MAT };

struct svm_node_base
{
	int type; // NODE_TYPE_INDEXED or NODE_TYPE_MAT
	const void* node;
	svm_node_base(int type_, const void* node_) : type (type_), node(node_) { }
};

struct svm_simd_func_ptrs
{
	void(*dotSub_64f64f)(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
		= nullptr; // Prediction
	void(*dot_64f64f)(const compv_float64_t* ptrA, const compv_float64_t* ptrB, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t strideA, const compv_uscalar_t strideB, compv_float64_t* ret)
		= nullptr; // Prediction + training
	void(*scale_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const compv_float64_t* s1)
		= nullptr; // Prediction
	void(*kernel_rbf0_out_64f64f)(const double& gamma, const double* xSquarePtr, const double* dotMatPtr, double* outPtr, const size_t count)
		= nullptr; // Training
	void(*kernel_rbf1_out_Step1_64f64f)(const double& gamma, const double& x_squarei, const double* xSquarejPtr, const double* dotMatPtr, double* outPtr, const size_t count)
		= nullptr; // Training
	void(*kernel_rbf1_out_Step2_64f32f)(const double& yi, const double* yjPtr, const double* outStep1Ptr, float* outPtr, const size_t count)
		= nullptr; // Training
	
	svm_simd_func_ptrs();
	void init();

	void expo(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const size_t count) const {
		exp_64f64f_minpackx(ptrIn, ptrOut, count, 1, 0, CompVMathExp::lut64u(), CompVMathExp::vars64u(), CompVMathExp::vars64f());
		for (size_t i = (count & -exp_64f64f_minpack); i < count; ++i) {
			ptrOut[i] = std::exp(ptrIn[i]);
		}
	}

private:
	void(*exp_64f64f_minpackx)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f)
		= nullptr; // Prediction + training
	int exp_64f64f_minpack = 1;
};

struct svm_node
{
	int index;
	double value;
};

struct svm_problem
{
	int l;
	double *y;
	struct svm_node **x;
};

enum { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };	/* svm_type */
enum { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED }; /* kernel_type */

struct svm_parameter
{
	int svm_type;
	int kernel_type;
	int degree;	/* for poly */
	double gamma;	/* for poly/rbf/sigmoid */
	double coef0;	/* for poly/sigmoid */

	/* these are for training only */
	double cache_size; /* in MB */
	double eps;	/* stopping criteria */
	double C;	/* for C_SVC, EPSILON_SVR and NU_SVR */
	int nr_weight;		/* for C_SVC */
	int *weight_label;	/* for C_SVC */
	double* weight;		/* for C_SVC */
	double nu;	/* for NU_SVC, ONE_CLASS, and NU_SVR */
	double p;	/* for EPSILON_SVR */
	int shrinking;	/* use the shrinking heuristics */
	int probability; /* do probability estimates */
};

//
// svm_model
// 
struct svm_model
{
	struct svm_parameter param;	/* parameter */
	int nr_class;		/* number of classes, = 2 in regression/one class svm */
	int l;			/* total #SV */
	struct svm_node **SV = nullptr;		/* SVs (SV[l]) */
	CompVMatPtr SVMat = nullptr; /* Same as SV but aligned to make it SIMD friendly */
	double **sv_coef = nullptr;	/* coefficients for SVs in decision functions (sv_coef[k-1][l]) */
	double *rho = nullptr;		/* constants in decision functions (rho[k*(k-1)/2]) */
	double *probA = nullptr;		/* pariwise probability information */
	double *probB = nullptr;
	int *sv_indices = nullptr;        /* sv_indices[0,...,nSV-1] are values in [1,...,num_traning_data] to indicate SVs in the training set */

	/* for classification only */

	int *label = nullptr;		/* label of each class (label[k]) */
	int *nSV = nullptr;		/* number of SVs for each class (nSV[k]) */
				/* nSV[0] + nSV[1] + ... + nSV[k-1] = l */
	/* XXX */
	int free_sv;		/* 1 if svm_model is created by svm_load_model*/
				/* 0 if svm_model is created by svm_train */

	struct svm_simd_func_ptrs simd_func_ptrs; //SIMD-function pointers

	svm_model();
};

struct svm_model *svm_train(const struct svm_problem *prob, const struct svm_parameter *param);
void svm_cross_validation(const struct svm_problem *prob, const struct svm_parameter *param, int nr_fold, double *target);

int svm_save_model(const char *model_file_name, const struct svm_model *model);
COMPV_BASE_API struct svm_model *svm_load_model(const char *model_file_name);

int svm_get_svm_type(const struct svm_model *model);
int svm_get_nr_class(const struct svm_model *model);
void svm_get_labels(const struct svm_model *model, int *label);
void svm_get_sv_indices(const struct svm_model *model, int *sv_indices);
int svm_get_nr_sv(const struct svm_model *model);
double svm_get_svr_probability(const struct svm_model *model);

double svm_predict_values(const struct svm_model *model, const struct svm_node_base *x, double* dec_values);
double svm_predict(const struct svm_model *model, const struct svm_node_base *x);
double svm_predict_distance(const struct svm_model *model, const struct svm_node_base *x, double *confidence);
double svm_predict_probability(const struct svm_model *model, const struct svm_node_base *x, double* prob_estimates);

void svm_free_model_content(struct svm_model *model_ptr);
COMPV_BASE_API void svm_free_and_destroy_model(struct svm_model **model_ptr_ptr);
void svm_destroy_param(struct svm_parameter *param);

const char *svm_check_parameter(const struct svm_problem *prob, const struct svm_parameter *param);
int svm_check_probability_model(const struct svm_model *model);
int svm_check_SIMDFriendly_model(const struct svm_model *model);

void svm_set_print_string_function(void (*print_func)(const char *));

size_t svm_count(const struct svm_node *x);
COMPV_ERROR_CODE svm_copy(const struct svm_node *x, CompVMatPtrPtr xMat, size_t count = 0);
COMPV_BASE_API COMPV_ERROR_CODE svm_makeSVs_SIMD_frienly(struct svm_model *model, const size_t expectedSVsize, const bool freeSV = true);
COMPV_ERROR_CODE svm_k_function_rbf(const CompVMatPtr& x, const CompVMatPtr& yy, const size_t count, const double& gamma, CompVMatPtr& kvalues, const struct svm_simd_func_ptrs *simd_func_ptrs);

#ifdef __cplusplus
}
#endif

#endif /* _LIBSVM_H */
