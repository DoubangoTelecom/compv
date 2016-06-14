#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

// MD5 values for WIN32 and WIN64 are different and this is correct

#define TYPE_DOUBLE			0
#define TYPE_FLOAT			1

#define LOOP_COUNT			20 // sparse matrix, dense matrix slowly converge when using floats
#define NUM_POINTS			200 + 9 // +9 to make it SIMD-unfriendly for testing
#define TYPE				TYPE_DOUBLE  // double or float

#if TYPE == TYPE_DOUBLE
#	if COMPV_ARCH_X64
#		define MD5_D	"0c191d633cdf3f26b21c8badfd2ba3d4" // 200 + 9 points
#		define MD5_Q	"84722db880903f6a5fbfcfaf26e53998" // 200 + 9 points
#		define MD5_D9	"cbadb9f8c6be8aa7abf7172423f7d5bd" // 0 + 9 points
#		define MD5_Q9	"473597e9b6f33c6308893f68e715a674" // 0 + 9 points
#	else
#		define MD5_D	"e643f74657501e838bacaeba7287ed0f" // 200 + 9 points
#		define MD5_Q	"5c23dd9118db5e3e72465d4791984fae" // 200 + 9 points
#		define MD5_D9	"904d259aac76f1fa495f4fbcdc072a07" // 0 + 9 points
#		define MD5_Q9	"b974abccdb70eb65f80d16c66d675919" // 0 + 9 points
#	endif // ARCH
#else
#	if COMPV_ARCH_X64
#		define MD5_D	"3434888c193281e5985902003beaf481" // 200 + 9 points
#		define MD5_Q	"2631c71d337040e32ba2c1b91d33117d" // 200 + 9 points
#		define MD5_D9	"3e1b312669b2c8806eb04ddc00578d4c" // 0 + 9 points
#		define MD5_Q9	"476c0cc46432c9adeaa853b490440776" // 0 + 9 points
#	else
#		define MD5_D	"3434888c193281e5985902003beaf481" // 200 + 9 points
#		define MD5_Q	"0d82444216886adc07b30eb980a43ae3" // 200 + 9 points
#		define MD5_D9	"3e1b312669b2c8806eb04ddc00578d4c" // 0 + 9 points
#		define MD5_Q9	"476c0cc46432c9adeaa853b490440776" // 0 + 9 points
#	endif // ARCH
#endif

COMPV_ERROR_CODE TestEigen()
{
#if TYPE == TYPE_DOUBLE
#	define TYP double
#else
#	define TYP float
#endif
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	uint64_t timeStart, timeEnd;
	CompVPtrArray(TYP) array, S, D, Q;
	TYP *x, *y, *z;

	// Build a dense matrix
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::newObjAligned(&array, 3, NUM_POINTS));
	x = const_cast<TYP*>(array->ptr(0));
	y = const_cast<TYP*>(array->ptr(1));
	z = const_cast<TYP*>(array->ptr(2));
	for (signed i = 0; i < NUM_POINTS; ++i) {
#if TYPE == TYPE_DOUBLE
		// Dense matrix
		x[i] = (TYP)((i & 1) ? i : -i) + 0.5;
		y[i] = ((TYP)(i * 0.2)) + i + 0.7;
		z[i] = i*i;
#else
		// Sparse matrix (float is really bad for fast convergence)
		x[i] = (TYP)((i & 1) ? 0 : -1) + (TYP)0.0001;
		y[i] = ((TYP)(i * 0.2)) + 1 + (TYP)0.0002;
		z[i] = (TYP)0;
#endif
	}


	// FIXME
#if 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();

	TYP mal = ((TYP)34494.203125000000 * (TYP)0.029227419973866218);
	TYP kal = ((TYP)-0.99957278770566338 * (TYP)1008.6075440000000);
	TYP val = ((TYP)34494.203125000000 * (TYP)0.029227419973866218) + ((TYP)-0.99957278770566338 * (TYP)1008.6075440000000);

	const TYP dataF[3][3] = {
		{ (TYP)1008.607544, (TYP)-841.387085, (TYP)34494.203125 },
		{ (TYP)-841.387085, (TYP)701.890686, (TYP)-28775.292969 },
		{ (TYP)34494.203125, (TYP)-28775.292969, (TYP)1179695.875000 },
	};
	CompVPtrArray(TYP) SF, DF, QF;
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(SF, &dataF[0][0], 3, 3));
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::eigenS(SF, DF, QF, false, false, false));
	matrixPrint<TYP>(DF, "D");
#endif
	
	// Symmetric(S) = A*A
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::mulAtA(array, S));

	bool symmetric;
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isSymmetric(S, symmetric));
	COMPV_CHECK_EXP_RETURN(!symmetric, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		// D (diagonal) contains the eigenvalues
		// Q (square) contain eigenvectors (each col is an eigenvector)
		COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::eigenS(S, D, Q));
	}
	timeEnd = CompVTime::getNowMills();
	
	const std::string md5D = arrayMD5<TYP>(D);
	const std::string md5Q = arrayMD5<TYP>(Q);
#if NUM_POINTS == 0 + 9 // homography (3x3)
	COMPV_ASSERT(md5D == MD5_D9);
	COMPV_ASSERT(md5Q == MD5_Q9);
#else
	COMPV_ASSERT(md5D == MD5_D);
	COMPV_ASSERT(md5Q == MD5_Q);
#endif

	COMPV_DEBUG_INFO("Elapsed time (TestEigen) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return err_;
}