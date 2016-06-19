#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;


#define LOOP_COUNT			10000
#define NUM_POINTS			(5000 + 15) // +15 to make it SIMD-unfriendly for testing
#define ANGLE				COMPV_MATH_PI / 4
#define SCALEX				5.0
#define SCALEY				5.0
#define ERRORPX				0.0 // percent, within [0.f, 1.f]
#define ERRORPY				0.0 // percent, within [0.f, 1.f]
#define TRANSX				28.5
#define TRANSY				-10.0
#define TYP					double  // double or float
#define MODE_EST			COMPV_MODELEST_TYPE_RANSAC
#define TYP_SZ				"%e"	// %e or %f
#if COMPV_ARCH_X64
#	define MD5_EXPECTED_SSE2	"fdf685c7faca327ce21275321acef547"
#	define MD5_EXPECTED_AVX		"882e4a573ccdf258221d956cb60358fe"
#	define MD5_EXPECTED			"4ac20362aefe978d084369c2e39e5911" // Without SIMD
#else
#	define MD5_EXPECTED_SSE2	"da724a468db3cd717e699ed9a811c459"
#	define MD5_EXPECTED_AVX		"4b459b1df9b46dfa5781dc9d9985c0f9"
#	define MD5_EXPECTED			"86013aa6fc4ddc47dbbf094826fb6aec" // Without SIMD
#endif

COMPV_ERROR_CODE TestHomography()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	uint64_t timeStart, timeEnd;
	bool colinear = false;

	// expected H
	const TYP H_expected[3][3] = {
		{ COMPV_MATH_COS(ANGLE)*SCALEX, -COMPV_MATH_SIN(ANGLE), TRANSX },
		{ COMPV_MATH_SIN(ANGLE), COMPV_MATH_COS(ANGLE)*SCALEY, TRANSY },
		{ 0, 0, 1 },
	};
	CompVPtrArray(TYP) input;
	CompVPtrArray(TYP) output;
	CompVPtrArray(TYP) h;
	TYP *x, *y, *z;
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::newObjAligned(&input, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::newObjAligned(&output, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVArray<TYP>::copy(h, &H_expected[0][0], 3, 3));

	// Initialize input
	// These points must not be colinear
	x = const_cast<TYP*>(input->ptr(0));
	y = const_cast<TYP*>(input->ptr(1));
	z = const_cast<TYP*>(input->ptr(2));
	for (signed i = 0; i < NUM_POINTS; ++i) {
		x[i] = (TYP)((i & 1) ? i : -i) + 0.5;
		y[i] = ((TYP)(x[i] * 0.2)) + i + 0.7;
		z[i] = 1; // required
	}

	// Check if input points are colinear
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::isColinear2D(input, colinear));
	COMPV_ASSERT(!colinear);

	// Compute output = H:input
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYP>::mulAB(h, input, output)); // output = mul(H, input)
	// Add error
	x = const_cast<TYP*>(output->ptr(0));
	y = const_cast<TYP*>(output->ptr(1));
	z = const_cast<TYP*>(output->ptr(2));
	for (size_t i = 0; i < NUM_POINTS; ++i) {
		x[i] += x[i] * ((i * ERRORPX) / NUM_POINTS);
		y[i] += y[i] * ((i * ERRORPY) / NUM_POINTS);
		// z[i] = 1; // required, but already set after mul(H, input)
	}
	
	h = NULL;
	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVHomography<TYP>::find(input, output, h, MODE_EST)); // NONE to make sure we'll always have the same result (ransac is nondeterministic)
	}
	timeEnd = CompVTime::getNowMills();

	COMPV_DEBUG_INFO("H_expected:\n"
		"{\t"TYP_SZ"\t"TYP_SZ"\t"TYP_SZ"\t}\n"
		"{\t"TYP_SZ"\t"TYP_SZ"\t"TYP_SZ"\t}\n"
		"{\t"TYP_SZ"\t"TYP_SZ"\t"TYP_SZ"\t}",
		H_expected[0][0], H_expected[0][1], H_expected[0][2],
		H_expected[1][0], H_expected[1][1], H_expected[1][2],
		H_expected[2][0], H_expected[2][1], H_expected[2][2]);
	COMPV_DEBUG_INFO("H_computed:\n"
		"{\t"TYP_SZ"\t"TYP_SZ"\t"TYP_SZ"\t}\n"
		"{\t"TYP_SZ"\t"TYP_SZ"\t"TYP_SZ"\t}\n"
		"{\t"TYP_SZ"\t"TYP_SZ"\t"TYP_SZ"\t}",
		*h->ptr(0, 0), *h->ptr(0, 1), *h->ptr(0, 2),
		*h->ptr(1, 0), *h->ptr(1, 1), *h->ptr(1, 2),
		*h->ptr(2, 0), *h->ptr(2, 1), *h->ptr(2, 2));

	COMPV_DEBUG_INFO("Elapsed time (TestHomography) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// TODO(dmi): Use residual results instead of MD5
	const std::string md5 = arrayMD5<TYP>(h);
	if (CompVCpu::isEnabled(compv::kCpuFlagAVX)) {
		COMPV_CHECK_EXP_RETURN(md5 != MD5_EXPECTED_AVX, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}
	else if (CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
		COMPV_CHECK_EXP_RETURN(md5 != MD5_EXPECTED_SSE2, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}
	else {
		COMPV_CHECK_EXP_RETURN(md5 != MD5_EXPECTED, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}

	return err_;
}
