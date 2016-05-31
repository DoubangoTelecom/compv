#include <compv/compv_api.h>

using namespace compv;

#define LOOP_COUNT	100000000

// 9 for Homography, otherwise 3 or 4
#define ARRAY_ROWS				9
#define ARRAY_COLS				9
#define EIGEN_EPSILON			1.1921e-07 // 1e-5

static double compv_hypot(double x, double y)
{
#if 1
	return hypot(x, y);
#elif 0
	// https://en.wikipedia.org/wiki/Hypot
	// Without overflow / underflow
	double t;
	x = COMPV_MATH_ABS(x);
	y = COMPV_MATH_ABS(y);
	t = COMPV_MATH_MIN(x, y);
	x = COMPV_MATH_MAX(x, y);
	t = t / x;
	return x*COMPV_MATH_SQRT(1 + t*t);
#elif 1
	// naive implementation
	return sqrt(x*x + y*y);
#endif
}

static void matrixPrint(const double* M, const char* desc = "Matrix")
{
	printf("----------\n%s\n----------\n", desc);
	for (int j = 0; j < ARRAY_ROWS; ++j) {
		for (int i = 0; i < ARRAY_COLS; ++i) {
			printf("%.4f, ", M[(j * ARRAY_COLS) + i]);
		}
		printf("\n");
	}
	printf("+++++++++++++\n");
}

// C <> A,B
static void matrixSquareMul(const double* A, const double* B, double* C)
{
	CompVMem::zero(C, ARRAY_ROWS * ARRAY_COLS * sizeof(double));
	COMPV_ASSERT(ARRAY_ROWS == ARRAY_COLS);
	static const int count = ARRAY_ROWS;
	for (int i = 0; i < count; ++i) {
		for (int j = 0; j < count; ++j) {
			for (int k = 0; k < count; ++k) {
				C[(i*count) + j] += A[(i*count) + k] * B[(k*count) + j];
			}
		}
	}
}

static void matrixMulAB(const double *A, int a_rows, int a_cols, const double *B, int b_rows, int b_cols, double *R)
{
	COMPV_ASSERT(a_rows && a_cols && b_rows == a_cols && b_cols && A && B && R && R != A && R != B);
	const double *a;
	double *r;
	// R is a (a_rows, b_cols) matrix
	for (int i = 0; i < a_rows; ++i) { // m1 rows
		for (int j = 0; j < b_cols; ++j) { // m2 cols
			a = A + (i * a_cols);
			r = R + (i * b_cols);
			r[j] = 0;
			for (int k = 0; k < b_rows; ++k) { // m2 rows
				r[j] += a[k] * B[(k*b_cols) + j];
			}
		}
	}
}

// B <> A
static void matrixSquareTranspose(const double* A, double* B)
{
	COMPV_ASSERT(ARRAY_ROWS == ARRAY_COLS);
	COMPV_ASSERT(A != B);
	static const int count = ARRAY_ROWS;
	for (int j = 0; j < count; ++j) {
		for (int i = 0; i < count; ++i) {
			B[(j*count) + i] = A[(i*count) + j];
		}
	}
}

static void matrixCopy(double* dst, const double* src)
{
	CompVMem::copy(dst, src, (ARRAY_ROWS * ARRAY_COLS) * sizeof(double));
}

// Build Givens rotation matrix at (i, j) with
static void GivensRotMatrix(double *G, int ith, int jth, double c, double s)
{
	int j, i;

	// i -> row
	// j -> col

	// From https://en.wikipedia.org/wiki/Givens_rotation

	// Identity matrix
	CompVMem::zero(G, ARRAY_ROWS*ARRAY_COLS*sizeof(double));
	for (i = 0; i < ARRAY_ROWS; ++i) {
		for (j = 0; j < ARRAY_COLS; ++j) {
			if (i == j) {
				G[(i*ARRAY_COLS) + j] = 1.0;
			}
		}
	}
	
	// Gii = c
	G[(ith*ARRAY_COLS) + ith] = c;
	// Gjj = c
	G[(jth*ARRAY_COLS) + jth] = c;
	// Gji = -s
	G[(jth*ARRAY_COLS) + ith] = -s;
	// Gij = s
	G[(ith*ARRAY_COLS) + jth] = s;
}

// Compute cos('c') and sin ('s')
static void JacobiAngles(const double *S, int ith, int jth, double *c, double *s)
{
#if 1
	// From https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
	double theta;
	double Sii = S[(ith * ARRAY_COLS) + ith];
	double Sjj = S[(jth * ARRAY_COLS) + jth];
	if (Sii == Sjj) {
		theta = COMPV_MATH_PI / 4.0;
	}
	else {
		theta = 0.5 * ::atan2(2.0*S[(ith * ARRAY_COLS) + jth], Sjj - Sii);
	}
	*c = ::cos(theta);
	*s = ::sin(theta);
#elif 0
	double d = (S[(ith * ARRAY_COLS) + ith] - S[(jth * ARRAY_COLS) + jth]) / (2.0*S[(ith * ARRAY_COLS) + jth]);
	double t = (d >= 0 ? +1 : -1) / (::abs(d) + ::sqrt(d*d + 1));
	*c = 1.0 / ::sqrt(t*t + 1);
	*s = t**c;
#else
	// FIXME: use this but find where comes the sign error
	double Sij = S[(ith * ARRAY_COLS) + jth];
	if (Sij == 0.0) {
		*c = 1.0;
		*s = 0.0;
	}
	else {
		// rho = (Aii - Ajj) / 2Aij
		double rho = (S[(ith * ARRAY_COLS) + ith] - S[(jth * ARRAY_COLS) + jth]) / (2.0 * Sij);
		double t;
		if (rho >= 0) {
			t = 1.0 / (rho + sqrt(1 + (rho * rho)));
		}
		else {
			t = -1 / (-rho + sqrt(1 + (rho * rho)));
		}
		*c = 1.0 / sqrt(1 + (t * t));
		*s = t * *c;
	}
#endif
}

// Largest absolute val
// S must be symmetric
static double SymmMaxOffDiag(const double *S, int *row, int *col)
{
	double r0_ = 0.0, r1_;
	const double* S_;
	for (int j = 0; j < ARRAY_ROWS; ++j) {
		S_ = S + (j * ARRAY_COLS);
		for (int i = 0; i < j; ++i) { // i stops at j because the matrix is symmetric and remains symetric after #1 Jacobi iteration
			if ((r1_ = abs(S_[i])) > r0_) {
				r0_ = r1_;
				*row = j;
				*col = i;
			}
		}
	}
	return r0_;
}

// Requires symetric matrix as input
// D: Diagonal matrix of eigenvalues
// Q: Matrix of eigenvectors
static void JacobiIter(const double *in, double *D, double *Q)
{
	int row, col;
	double G[ARRAY_ROWS][ARRAY_COLS], matrixTemp0[ARRAY_ROWS][ARRAY_COLS];
	double gc_, gs_;
	const double *matrixIn = (double*)in;
	double *matrixOut0 = (double*)matrixTemp0;
	bool is_diag = false;
	int sweeps = 0, ops = 0;
	double laxOffDiag;

	// Set Q to identity matrix
	CompVMem::zero(Q, ARRAY_ROWS*ARRAY_COLS*sizeof(double));
	for (row = 0; row < ARRAY_ROWS; ++row) {
		for (col = 0; col < ARRAY_COLS; ++col) {
			if (row == col) {
				Q[(row*ARRAY_COLS) + col] = 1.0;
			}
		}
	}

#if 1
	// Sign correct
	// Less ops
	// Could be MT?
	// TODO(dmi): No need for matrix multiplication -> use vector mul on the concerned rows (all other rows are unchanged)
	while ((laxOffDiag = SymmMaxOffDiag(matrixIn, &row, &col)) > EIGEN_EPSILON) {
		++ops;
		JacobiAngles(matrixIn, row, col, &gc_, &gs_);
		GivensRotMatrix((double*)G, row, col, gc_, gs_);
		//matrixPrint((const double*)G, "G");
		// Q = QG
		matrixSquareMul(Q, (const double*)G, matrixOut0);
		matrixCopy(Q, matrixOut0);
		//matrixPrint((const double*)Q, "Q=QG");
		// AG
		matrixSquareMul(matrixIn, (const double*)G, matrixOut0); // Input and Output must be different 
		//matrixPrint((const double*)matrixOut0, "AG");
		// G*
		// matrixSquareTranspose((double*)matrixGivens, (double*)matrixGivens);
		GivensRotMatrix((double*)G, col, row, gc_, gs_); // FIXME: change GivensRotMatrix to output G and G*
		//matrixPrint((const double*)G, "G*");
		// G*AG
		matrixSquareMul((const double*)G, matrixOut0, D);
		//matrixPrint((const double*)D, "D=G*AG");
		matrixIn = D;
	}
#else
	// Easier to MT
	// More ops
	// Sign not correct
	const double *in_;
	while (!is_diag) {
		is_diag = true;
		for (row = 0; row < ARRAY_ROWS; ++row) {
			for (col = 0; col < row; ++col) {
				if (::abs(matrixIn[(row * ARRAY_COLS) + col]) > EIGEN_EPSILON) {
					++ops;
					is_diag = false;
					JacobiAngles(matrixIn, row, col, &gc_, &gs_);
					GivensRotMatrix((double*)G, row, col, gc_, gs_);
					//matrixPrint((const double*)G, "G");
					// Q = QG
					matrixSquareMul(Q, (const double*)G, matrixOut0);
					matrixCopy(Q, matrixOut0);
					//matrixPrint((const double*)Q, "Q=QG");
					// AG
					matrixSquareMul(matrixIn, (const double*)G, matrixOut0); // Input and Output must be different 
					//matrixPrint((const double*)matrixOut0, "AG");
					// G*
					// matrixSquareTranspose((double*)matrixGivens, (double*)matrixGivens);
					GivensRotMatrix((double*)G, col, row, gc_, gs_); // FIXME: change GivensRotMatrix to output G and G*
					//matrixPrint((const double*)G, "G*");
					// G*AG
					matrixSquareMul((const double*)G, matrixOut0, D);
					//matrixPrint((const double*)D, "D=G*AG");
					matrixIn = D;
				}
			}
		}
		if (!is_diag) ++sweeps;
	}
#endif

	matrixPrint((const double*)D, "D = Eigenvalues");
	matrixPrint((const double*)Q, "Q = Eigenvectors");
	
	COMPV_DEBUG_INFO("Number of sweeps: %d, ops: %d", sweeps, ops);
}

static void Homography(double(*H)[3][3])
{
	// Solve: equation x' = Hx
	static const int kNumPoints = 4;
	static const double angle = COMPV_MATH_PI / 4;
	static const double scaleX = 5.0;
	static const double scaleY = 3.0;
	static const double transX = 28.5;
	static const double transY = 10.0;

	// expected H
	const double H_[3][3] = {
		{ ::cos(angle)*scaleX, -::sin(angle), transX },
		{ ::sin(angle), ::cos(angle)*scaleY, transY },
		{ 0, 0, 1 },
	};
	// x
	const double X_[3/*x,y,z*/][kNumPoints] = { // (X, Y, Z)
		{ 2, 3, 5, 8 },
		{ 5, 2, 9, 4 },
		{ 1, 1, 1, 1 },
	};
	// x' = Hx
	double XPrime_[3/*x',y',z'*/][kNumPoints]; // (X', Y', Z')
	matrixMulAB(&H_[0][0], 3, 3, &X_[0][0], 3, kNumPoints, &XPrime_[0][0]);

	// Hartley and Zisserman
	// Normalization, translation to have coordinate system centered at the centroid
	// https://en.wikipedia.org/wiki/Centroid#Of_a_finite_set_of_points
	// https://en.wikipedia.org/wiki/Eight-point_algorithm#How_it_can_be_solved
	// centroid = sum(xi)/k
	double t0x_ = 0, t0y_ = 0, t1x_ = 0, t1y_ = 0;
	for (int i = 0; i < kNumPoints; ++i) {
		// The origin of the new coordinate system should be centered (have its origin) at the centroid (center of gravity) of the image points. This is accomplished by a translation of the original origin to the new one.
		t0x_ += X_[0][i];
		t0y_ += X_[1][i];
		t1x_ += XPrime_[0][i];
		t1y_ += XPrime_[1][i];
	}
	t0x_ /= kNumPoints;
	t0y_ /= kNumPoints;
	t1x_ /= kNumPoints;
	t1y_ /= kNumPoints;

	// Translate
	/*for (int i = 0; i < kNumPoints; ++i) {
		X_[0][i] -= t0x_;
		X_[1][i] -= t0y_;
		XPrime_[0][i] -= t1x_;
		XPrime_[1][i] -= t1y_;
	}*/

	// Normalization, scaling
	// AFTER the translation the coordinates are uniformly scaled (Isotropic scaling) so that the mean distance from the origin to a point equals sqrt{2} .
	// Isotropic scaling -> scaling is invariant with respect to direction
	double mag0 = 0, mag1 = 0;
	for (int i = 0; i < kNumPoints; ++i) {
		mag0 += compv_hypot((X_[0][i] - t0x_), (X_[1][i] - t0y_));
		mag1 += compv_hypot((XPrime_[0][i] - t1x_), (XPrime_[1][i] - t1y_));
	}
	mag0 /= kNumPoints;
	mag1 /= kNumPoints;
	double s0 = COMPV_MATH_SQRT_2 / mag0;
	double s1 = COMPV_MATH_SQRT_2 / mag1;
	// Scale
	/*for (int i = 0; i < kNumPoints; ++i) {
		X_[0][i] *= s0;
		X_[1][i] *= s0;
		XPrime_[0][i] *= s1;
		XPrime_[1][i] *= s1;
	}*/

	// Translation(t) to centroid then scaling(s) operation:
	// -> b = (a+t)*s = a*s+t*s = a*s+t' with t'= t*s
	const double T1[3][3] = {
		{ s0, 0, -t0x_*s0 },
		{ 0, s0, -t0y_*s0 },
		{ 0, 0, 1 }
	};
	const double T2[3][3] = {
		{ s1, 0, -t1x_*s1 },
		{ 0, s1, -t1y_*s1 },
		{ 0, 0, 1 }
	};

	// Inverse operation
	// -> b = a*s+t'
	// -> a = b*(1/s)-t'*(1/s) = b*(1/s)+t'' whith t'' = -t'/s = -(t*s)/s = -t
	const double invT2[3][3] = {
		{ 1 / s1, 0, t1x_ },
		{ 0, 1 / s1, t1y_ },
		{ 0, 0, 1 }
	};

	// Normalize X: Xn = T1X
	double Xn_[3/*x,y,z*/][kNumPoints];
	matrixMulAB(&T1[0][0], 3, 3, &X_[0][0], 3, kNumPoints, &Xn_[0][0]);
	// Normalize Xprime: Xnprime = T2Xprime
	double XnPrime_[3/*x,y,z*/][kNumPoints];
	matrixMulAB(&T2[0][0], 3, 3, &XPrime_[0][0], 3, kNumPoints, &XnPrime_[0][0]);
	
	// homogeneous equation: Mh = 0

	// Build M (FIXME: z' = 1 in compv as we'll append it to Xprime)
	double M[2 * kNumPoints][9], Mt[9][2 * kNumPoints];
	for (int j = 0, k = 0; k < kNumPoints; j += 2, ++k) {
		M[j][0] = -Xn_[0][k]; // -x
		M[j][1] = -Xn_[1][k]; // -y
		M[j][2] = -1; // -1
		M[j][3] = 0;
		M[j][4] = 0;
		M[j][5] = 0;
		M[j][6] = (XnPrime_[0][k] * Xn_[0][k]) / XnPrime_[2][k]; // (x'x)/z'
		M[j][7] = (XnPrime_[0][k] * Xn_[1][k]) / XnPrime_[2][k]; // (x'y)/z'
		M[j][8] = XnPrime_[0][k] / XnPrime_[2][k]; // x'/z'

		M[j + 1][0] = 0;
		M[j + 1][1] = 0;
		M[j + 1][2] = 0;
		M[j + 1][3] = -Xn_[0][k]; // -x
		M[j + 1][4] = -Xn_[1][k]; // -y
		M[j + 1][5] = -1; // -1
		M[j + 1][6] = (XnPrime_[1][k] * Xn_[0][k]) / XnPrime_[2][k]; // (y'x)/z'
		M[j + 1][7] = (XnPrime_[1][k] * Xn_[1][k]) / XnPrime_[2][k]; // (y'y)/z'
		M[j + 1][8] = XnPrime_[1][k] / XnPrime_[2][k]; // y'/z'
	}

	// M* = transpose(M)
	for (int j = 0; j < 2 * kNumPoints; ++j) {
		for (int i = 0; i < 9; ++i) {
			Mt[i][j] = M[j][i];
		}
	}

	// S = M*M (M transpose M)
	double S[9][9] = { 0 }; // Symetric matrix
	matrixMulAB((const double*)Mt, 9, 2 * kNumPoints, (const double*)M, 2 * kNumPoints, 9, (double*)S);
	/*for (int i = 0; i < 9; ++i) { // m1 rows
		for (int j = 0; j < 9; ++j) { // m2 cols
			for (int k = 0; k < 2 * kNumPoints; ++k) { // m2 rows
				S[i][j] += Mt[i][k] * M[k][j];
			}
		}
	}*/
	// Check symmetry
	for (int i = 0; i < 9; ++i) {
		for (int j = 0; j < 9; ++j) {
			COMPV_ASSERT(S[i][j] == S[j][i]);
		}
	}
	matrixPrint((const double*)S, "S = M*M");

	// Eigenvalues and eigenvectors
	double D[ARRAY_ROWS][ARRAY_COLS];
	double Q[ARRAY_ROWS][ARRAY_COLS];

	JacobiIter((const double*)S, (double*)D, (double*)Q);

	// h
	double h[3][3];
	// FIXME: assumed EigenValues are sorted and the smallest one is on the last column
	h[0][0] = Q[0][ARRAY_COLS - 1];
	h[0][1] = Q[1][ARRAY_COLS - 1];
	h[0][2] = Q[2][ARRAY_COLS - 1];
	h[1][0] = Q[3][ARRAY_COLS - 1];
	h[1][1] = Q[4][ARRAY_COLS - 1];
	h[1][2] = Q[5][ARRAY_COLS - 1];
	h[2][0] = Q[6][ARRAY_COLS - 1];
	h[2][1] = Q[7][ARRAY_COLS - 1];
	h[2][2] = Q[8][ARRAY_COLS - 1]; // Should be #1 (up to a to scalar 1/Z)

	// HnAn = Bn, where Hn, An and Bn are normalized points
	// ->HnT1A = T2B
	// ->T2*HnT1A = T2*T2B = B
	// ->(T2*HnT1)A = B -> H'A = B whith H' = T2*HnT1 our final homography matrix
	double temp[3][1];
	matrixMulAB(&invT2[0][0], 3, 3, &h[0][0], 3, 3, &temp[0][0]);
	matrixMulAB(&temp[0][0], 3, 3, &T1[0][0], 3, 3, &(*H)[0][0]);

	// Scale H to make it homogeneous (Z = 1)
	double Z = 1.0 / (*H)[2][2];
	(*H)[0][0] *= Z;
	(*H)[0][1] *= Z;
	(*H)[0][2] *= Z;
	(*H)[1][0] *= Z;
	(*H)[1][1] *= Z;
	(*H)[1][2] *= Z;
	(*H)[2][0] *= Z;
	(*H)[2][1] *= Z;
	(*H)[2][2] *= Z; // should be #1

	// print H
	printf("H(expected) = ");
	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 3; ++i) {
			printf("%e, ", H_[j][i]);
		}
		printf("\n");
	}
	printf("H(computed) = ");
	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 3; ++i) {
			printf("%e, ", (*H)[j][i]);
		}
		printf("\n");
	}

	// TODO(dmi): limite H to an affine transform (last row = 0 0 1)

	// TODO(dmi): RANSAC check colinearity

	// TODO(dmi): Use SIMD_DP

	// TODO(dmi): Mul_AtA, Mul_AtB, Mul_ABt, Mul_AB...
	// TODO(dmi): Mul3x3_AB, Mul3x3_AtB...
	// TODO(dmi): Mul3x3_AtBC

	// Mh =?
	double Mh[2 * kNumPoints][1] = {0};
	matrixMulAB(&M[0][0], 2 * kNumPoints, 9, &h[0][0], 9, 1, &Mh[0][0]);
	/*for (int i = 0; i < 2 * kNumPoints; ++i) { // m1 rows
		for (int j = 0; j < 1; ++j) { // m2 cols
			for (int k = 0; k < 9; ++k) { // m2 rows
				Mh[i][j] += M[i][k] * h[k][j];
			}
		}
	}*/
	// print Mh
	printf("Mh=");
	for (int j = 0; j < 2 * kNumPoints; ++j) {
		for (int i = 0; i < 1; ++i) {
			printf("%.5f, ", Mh[j][i]);
		}
	}
	printf("\n");
	if (Mh[0]) {
		int kaka = 0;
	}
}

bool TestEigen()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

#if 0 // EigenValues and EigenVectors
#if ARRAY_ROWS == 3 && ARRAY_COLS == 3
	static const double SYMETRIC_MATRIX[ARRAY_ROWS][ARRAY_COLS] = {
#if 1
	{ 3, 1, 2 },
	{ 1, 2, 1 },
	{ 2, 1, 4 },
#elif 1
	{ 5, -2, 0 },
	{ -2, 5, 0 },
	{ 0, 0, 3 },
#elif 1
	{ 6, 5, 0 },
	{ 5, 1, 4 },
	{ 0, 4, 3 },
#endif
	};
#elif ARRAY_ROWS == 4 && ARRAY_COLS == 4
	static const double SYMETRIC_MATRIX[ARRAY_ROWS][ARRAY_COLS] = {
		// Sample from https://en.wikipedia.org/wiki/Jacobi_eigenvalue_algorithm
		{ 4, -30, 60, -35 },
		{ -30, 300, -675, 420 },
		{ 60, -675, 1620, -1050 },
		{ -35, 420, -1050, 700 }
	};
#endif
	double D[ARRAY_ROWS][ARRAY_COLS];
	double Q[ARRAY_ROWS][ARRAY_COLS];

	JacobiIter((const double*)SYMETRIC_MATRIX, (double*)D, (double*)Q);
#else
	double H[3][3];
	Homography(&H);
#endif

	return COMPV_ERROR_CODE_IS_OK(err_);
}