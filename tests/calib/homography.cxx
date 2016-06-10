#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;


#define LOOP_COUNT		1
#define NUM_POINTS		5000 + 9 // +9 to make it SIMD-unfriendly for testing
#define ANGLE			COMPV_MATH_PI / 4
#define SCALEX			5.0
#define SCALEY			5.0
#define ERRORPX			0.0 // percent, within [0.f, 1.f]
#define ERRORPY			0.0 // percent, within [0.f, 1.f]
#define TRANSX			28.5
#define TRANSY			-10.0
#define TYPE			double  // double or float
#define TYPE_SZ			"%e"	// %e or %f
#define MD5_EXPECTED	"3248f48b27e7b01ee6db4ec2a20343e6"

COMPV_ERROR_CODE TestHomography()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	uint64_t timeStart, timeEnd;
	bool colinear = false;

	// expected H
	const TYPE H_expected[3][3] = {
		{ COMPV_MATH_COS(ANGLE)*SCALEX, -COMPV_MATH_SIN(ANGLE), TRANSX },
		{ COMPV_MATH_SIN(ANGLE), COMPV_MATH_COS(ANGLE)*SCALEY, TRANSY },
		{ 0, 0, 1 },
	};
	CompVPtrArray(TYPE) input;
	CompVPtrArray(TYPE) output;
	CompVPtrArray(TYPE) h;
	TYPE *x, *y, *z;
	COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::newObjAligned(&input, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::newObjAligned(&output, 3, NUM_POINTS));
	COMPV_CHECK_CODE_RETURN(CompVArray<TYPE>::copy(h, &H_expected[0][0], 3, 3));

	// Initialize input
	// These points must not be colinear
	x = const_cast<TYPE*>(input->ptr(0));
	y = const_cast<TYPE*>(input->ptr(1));
	z = const_cast<TYPE*>(input->ptr(2));
	for (signed i = 0; i < NUM_POINTS; ++i) {
		x[i] = (TYPE)((i & 1) ? i : -i) + 0.5;
		y[i] = ((TYPE)(x[i] * 0.2)) + i + 0.7;
		z[i] = 1; // required
	}

	// Check if input points are colinear
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::isColinear2D(input, colinear));
	COMPV_ASSERT(!colinear);

	// Compute output = H:input
	COMPV_CHECK_CODE_RETURN(CompVMatrix<TYPE>::mulAB(h, input, output)); // output = mul(H, input)
	// Add error
	x = const_cast<TYPE*>(output->ptr(0));
	y = const_cast<TYPE*>(output->ptr(1));
	z = const_cast<TYPE*>(output->ptr(2));
	for (size_t i = 0; i < NUM_POINTS; ++i) {
		x[i] += x[i] * ((i * ERRORPX) / NUM_POINTS);
		y[i] += y[i] * ((i * ERRORPY) / NUM_POINTS);
		// z[i] = 1; // required, but already set after mul(H, input)
	}

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVHomography<TYPE>::find(input, output, h, COMPV_MODELEST_TYPE_NONE)); // NONE to make sure we'll always have the same result (ransac is nondeterministic)
	}
	timeEnd = CompVTime::getNowMills();

	COMPV_DEBUG_INFO("H_expected:\n"
		"{\t"TYPE_SZ"\t"TYPE_SZ"\t"TYPE_SZ"\t}\n"
		"{\t"TYPE_SZ"\t"TYPE_SZ"\t"TYPE_SZ"\t}\n"
		"{\t"TYPE_SZ"\t"TYPE_SZ"\t"TYPE_SZ"\t}",
		H_expected[0][0], H_expected[0][1], H_expected[0][2],
		H_expected[1][0], H_expected[1][1], H_expected[1][2],
		H_expected[2][0], H_expected[2][1], H_expected[2][2]);
	COMPV_DEBUG_INFO("H_computed:\n"
		"{\t"TYPE_SZ"\t"TYPE_SZ"\t"TYPE_SZ"\t}\n"
		"{\t"TYPE_SZ"\t"TYPE_SZ"\t"TYPE_SZ"\t}\n"
		"{\t"TYPE_SZ"\t"TYPE_SZ"\t"TYPE_SZ"\t}",
		*h->ptr(0, 0), *h->ptr(0, 1), *h->ptr(0, 2),
		*h->ptr(1, 0), *h->ptr(1, 1), *h->ptr(1, 2),
		*h->ptr(2, 0), *h->ptr(2, 1), *h->ptr(2, 2));

	const std::string md5 = arrayMD5<TYPE>(h);
	COMPV_ASSERT(md5 == MD5_EXPECTED);

	COMPV_DEBUG_INFO("Elapsed time (TestHomography) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return err_;
}
