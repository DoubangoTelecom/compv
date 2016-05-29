#include <compv/compv_api.h>

using namespace compv;

#define LOOP_COUNT	100000000

// 9 for Homography, otherwise 3 or 4
#define ARRAY_ROWS				9
#define ARRAY_COLS				9
#define EIGEN_CLOSE_TO_ZERO		1e-5

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
#elif 0
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
static double MaxOffDiagVal(const double *M, int *row, int *col)
{
	double r0_ = 0.0, r1_;
	const double* M_;
	for (int j = 0; j < ARRAY_ROWS; ++j) {
		M_ = M + (j * ARRAY_COLS);
		for (int i = 0; i < j; ++i) {
			if ((r1_ = abs(M_[i])) > r0_) {
				r0_ = r1_;
				*row = j;
				*col = i;
			}
		}
	}
	return r0_;
}

// Requires symetric matrix
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
	while ((laxOffDiag = MaxOffDiagVal(matrixIn, &row, &col)) > EIGEN_CLOSE_TO_ZERO) {
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
				if (::abs(matrixIn[(row * ARRAY_COLS) + col]) > EIGEN_CLOSE_TO_ZERO) {
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

	const double H_[3][3] = { // expected H
#if 0
		{ ::cos(angle), -::sin(angle), 0 },
		{ ::sin(angle), ::cos(angle), 0 },
		{ 0, 0, 1 },
#else
		{ 0.70710678118654757, -0.70710678118654757, 0 },
		{ 0.70710678118654757, 0.70710678118654757, 0 },
		{ 0, 0, 1 },
#endif
	};
	// x' = Hx
	const double XPrime_[3/*x',y',z'*/][kNumPoints] = { // (X', Y', Z')
#if 0
		{ -2.1213, 0.7071, -2.8284, 2.8284 },
		{ 4.9497, 3.5355, 9.8995, 8.4853 },
		{ 1.0000, 1.0000, 1.0000, 1.0000 }
#else
		{ 2, 3, 5, 8 },
		{ 5, 2, 9, 4 },
		{ 1, 1, 1, 1 },
#endif
	};
	// x
	const double X_[3/*x,y,z*/][kNumPoints] = { // (X, Y, Z)
		{ 2, 3, 5, 8 },
		{ 5, 2, 9, 4 },
		{ 1, 1, 1, 1 },
	};
	
	// homogeneous equation: Mh = 0

	// Build M (FIXME: z' = 1 in compv as we'll append it to Xprime)
	double M[2 * kNumPoints][9], Mt[9][2 * kNumPoints];
	for (int j = 0, k = 0; k < kNumPoints; j += 2, ++k) {
		M[j][0] = -X_[0][k]; // -x
		M[j][1] = -X_[1][k]; // -y
		M[j][2] = -1; // -1
		M[j][3] = 0;
		M[j][4] = 0;
		M[j][5] = 0;
		M[j][6] = (XPrime_[0][k] * X_[0][k]) / XPrime_[2][k]; // (x'x)/z'
		M[j][7] = (XPrime_[0][k] * X_[1][k]) / XPrime_[2][k]; // (x'y)/z'
		M[j][8] = XPrime_[0][k] / XPrime_[2][k]; // x'/z'

		M[j + 1][0] = 0;
		M[j + 1][1] = 0;
		M[j + 1][2] = 0;
		M[j + 1][3] = -X_[0][k]; // -x
		M[j + 1][4] = -X_[1][k]; // -y
		M[j + 1][5] = -1; // -1
		M[j + 1][6] = (XPrime_[1][k] * X_[0][k]) / XPrime_[2][k]; // (y'x)/z'
		M[j + 1][7] = (XPrime_[1][k] * X_[1][k]) / XPrime_[2][k]; // (y'y)/z'
		M[j + 1][8] = XPrime_[1][k] / XPrime_[2][k]; // y'/z'
	}

	// M*
	for (int j = 0; j < 2 * kNumPoints; ++j) {
		for (int i = 0; i < 9; ++i) {
			Mt[i][j] = M[j][i];
		}
	}

	// S = M*M (M transpose M)
	double S[9][9] = { 0 }; // Symetric matrix
	for (int i = 0; i < 9; ++i) { // m1 rows
		for (int j = 0; j < 9; ++j) { // m2 cols
			for (int k = 0; k < 2 * kNumPoints; ++k) { // m2 rows
				S[i][j] += Mt[i][k] * M[k][j];
			}
		}
	}
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
	double h[9][1];
	for (int j = 0; j < 9; ++j) {
		for (int i = 0; i < 1; ++i) {
			h[j][i] = Q[j][ARRAY_COLS - 1]; // FIXME: assumed EigenValues are sorted and the smallest one is on the last column
		}
	}

	// Mh =?
	double Mh[2 * kNumPoints][1] = {0};
	for (int i = 0; i < 2 * kNumPoints; ++i) { // m1 rows
		for (int j = 0; j < 1; ++j) { // m2 cols
			for (int k = 0; k < 9; ++k) { // m2 rows
				Mh[i][j] += M[i][k] * h[k][j];
			}
		}
	}
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