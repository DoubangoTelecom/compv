#include "../tests_common.h"

#define TAG_TEST								"TestHoughKht"

#define LOOP_COUNT				1

#define HOUGHKHT_RHO			1.f
#define HOUGHKHT_THETA			kfMathTrigPiOver180 // radian(1d)
#define HOUGHKHT_THRESHOLD		100

COMPV_ERROR_CODE houghkht()
{
	CompVHoughPtr houghkht;
	CompVMatPtr edges;
	CompVHoughLineVector lines;
	
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&houghkht, COMPV_HOUGHKHT_ID, HOUGHKHT_RHO, HOUGHKHT_THETA, HOUGHKHT_THRESHOLD));
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1020, 960, 1020, "C:/Projects/GitHub/data/hough/road_binary1020x960_gray.yuv", &edges));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(houghkht->process(edges, lines));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "HoughKht Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if 0
	compv_float32_t sum_rho = 0.f;
	compv_float32_t sum_theta = 0.f;
	size_t sum_strength = 0;
	for (CompVHoughLineVector::const_iterator i = lines.begin(); i < lines.end(); ++i) {
		sum_rho += i->rho;
		sum_theta += i->theta;
		sum_strength += i->strength;
	}

	COMPV_CHECK_EXP_RETURN(sum_rho != test->sum_rho, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghkht sum_rho mismatch");
	COMPV_CHECK_EXP_RETURN(COMPV_MATH_ABS(sum_theta - test->sum_theta) > 0.0009765625, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghkht sum_theta mismatch");
	COMPV_CHECK_EXP_RETURN(sum_strength != test->sum_strength, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghkht sum_strength mismatch");
#endif

	return COMPV_ERROR_CODE_S_OK;
}
