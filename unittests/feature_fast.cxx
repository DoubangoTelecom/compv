#include "../tests/tests_common.h"

#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define TAG_UNITTESTS "UnitTestFeatureFast"

static const struct compv_unittest_feature_fast {
	int threshold;
	bool nonmax;
	int fasdId;
	int maxFeatures;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	size_t corners;
	compv_float32_t scores;
	compv_float32_t xf;
	compv_float32_t yf;
}
COMPV_UNITTESTS_FEATURE_FAST[] =
{
	{ 20, false, COMPV_FAST_TYPE_9, -1, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 9583, 381080.f, 5826908.f, 3722280.f },
	{ 20, true, COMPV_FAST_TYPE_9, -1, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 2757, 115673.f, 1749686.f, 1065867.f },
	{ 20, false, COMPV_FAST_TYPE_12, -1, "equirectangular_1282x720_gray.yuv", 1282,720, 1282, 3953, 148518.f, 2431991.f, 1549595.f },
	{ 20, true, COMPV_FAST_TYPE_12, -1, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 1456, 56445.f, 938167.f, 561473.f },
	{ 20, false, COMPV_FAST_TYPE_9, 2000, "equirectangular_1282x720_gray.yuv", 1282,720, 1282, 2000, 158611.f, 1187679.f, 782137.f },
	{ 20, true, COMPV_FAST_TYPE_9, 2000, "equirectangular_1282x720_gray.yuv", 1282,720, 1282, 2000, 99205.f, 1252153.f, 766739.f },
	{ 20, false, COMPV_FAST_TYPE_12, 2000, "equirectangular_1282x720_gray.yuv", 1282, 720, 1282, 2000, 102310.f, 1205833.f, 783399.f },
	{ 20, true, COMPV_FAST_TYPE_12, 2000, "equirectangular_1282x720_gray.yuv", 1282,720, 1282, 1456, 56445.f, 938167.f, 561473.f },

	{ 20, false, COMPV_FAST_TYPE_9, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 3493, 144778.f, 310525.f, 435337.f },
	{ 20, true, COMPV_FAST_TYPE_9, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 1011, 49064.f, 90965.f, 122555.f },
	{ 20, false, COMPV_FAST_TYPE_12, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 1673, 62092.f, 147160.f, 202160.f },
	{ 20, true, COMPV_FAST_TYPE_12, -1, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 672, 27580.f, 59806.f, 79401.f },
	{ 20, false, COMPV_FAST_TYPE_9, 2000, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 2000, 106676.f, 175436.f, 234176.f },
	{ 20, true, COMPV_FAST_TYPE_9, 2000, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 1011, 49064.f, 90965.f, 122555.f },
	{ 20, false, COMPV_FAST_TYPE_12, 2000, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 1673, 62092.f, 147160.f, 202160.f },
	{ 20, true, COMPV_FAST_TYPE_12, 2000, "opengl_programming_guide_8th_edition_200x258_gray.yuv", 200, 258, 200, 672, 27580.f, 59806.f, 79401.f },

	{ 20, false, COMPV_FAST_TYPE_9, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480, 13625, 535125.f, 2868526.f, 4554394.f },
	{ 20, true, COMPV_FAST_TYPE_9, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480,3096, 144996.f, 650016.f, 1057869.f },
	{ 20, false, COMPV_FAST_TYPE_12, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480, 5536, 207029.f, 1215982.f, 1696520.f },
	{ 20, true, COMPV_FAST_TYPE_12, -1, "mandekalou_480x640_gray.yuv", 480, 640, 480, 1700, 72175.f, 368078.f, 532697.f },
	{ 20, false, COMPV_FAST_TYPE_9, 2000, "mandekalou_480x640_gray.yuv", 480, 640, 480, 2000, 155490.f, 457457.f, 554480.f },
	{ 20, true, COMPV_FAST_TYPE_9, 2000, "mandekalou_480x640_gray.yuv", 480, 640, 480, 2000, 117822.f, 428745.f, 628554.f },
	{ 20, false, COMPV_FAST_TYPE_12, 2000, "mandekalou_480x640_gray.yuv", 480, 640,480, 2000, 111486.f, 455286.f, 537950.f },
	{ 20, true, COMPV_FAST_TYPE_12, 2000, "mandekalou_480x640_gray.yuv", 480, 640, 480, 1700, 72175.f, 368078.f, 532697.f },
};
size_t COMPV_UNITTESTS_FEATURE_FAST_COUNT = sizeof(COMPV_UNITTESTS_FEATURE_FAST) / sizeof(COMPV_UNITTESTS_FEATURE_FAST[0]);

static std::string unittest_feature_fast_tostring(const compv_unittest_feature_fast* test)
{
	return
		std::string("threshold:") + CompVBase::to_string(test->threshold) + std::string(", ")
		+ std::string("nonmax:") + CompVBase::to_string(test->nonmax) + std::string(", ")
		+ std::string("fasdId:") + std::string(test->fasdId == COMPV_FAST_TYPE_9 ? "FAST9" : "FAST12") + std::string(", ")
		+ std::string("maxFeatures:") + CompVBase::to_string(test->maxFeatures) + std::string(", ")
		+ std::string("filename:") + std::string(test->filename) + std::string(", ");
}

COMPV_ERROR_CODE unittest_feature_fast()
{
	CompVCornerDetePtr fast;
	CompVMatPtr image;
	CompVBoxInterestPointPtr interestPoints;
	const compv_unittest_feature_fast* test;
	float sum_scores;
	float xf;
	float yf;

	COMPV_CHECK_CODE_RETURN(CompVCornerDete::newObj(&fast, COMPV_FAST_ID));
	for (size_t i = 0; i < COMPV_UNITTESTS_FEATURE_FAST_COUNT; ++i) {
		test = &COMPV_UNITTESTS_FEATURE_FAST[i];
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "== Trying new test: Fast feature detection -> %s ==", unittest_feature_fast_tostring(test).c_str());
		COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
		COMPV_CHECK_CODE_RETURN(fast->setInt(COMPV_FAST_SET_INT_THRESHOLD, test->threshold));
		COMPV_CHECK_CODE_RETURN(fast->setInt(COMPV_FAST_SET_INT_FAST_TYPE, test->fasdId));
		COMPV_CHECK_CODE_RETURN(fast->setInt(COMPV_FAST_SET_INT_MAX_FEATURES, test->maxFeatures));
		COMPV_CHECK_CODE_RETURN(fast->setBool(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, test->nonmax));
		COMPV_CHECK_CODE_RETURN(fast->process(image, &interestPoints));
		COMPV_CHECK_EXP_RETURN(interestPoints->size() != test->corners, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Number of corners mismatch");
		sum_scores = 0.f;
		xf = 0.f;
		yf = 0.f;
		for (size_t i = 0; i < interestPoints->size(); ++i) {
			sum_scores += interestPoints->ptr(i)->strength;
			xf += interestPoints->ptr(i)->x;
			yf += interestPoints->ptr(i)->y;
		}
		COMPV_CHECK_EXP_RETURN(interestPoints->size() != test->corners, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Num corners mismatch");
		COMPV_CHECK_EXP_RETURN(sum_scores != test->scores, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of scores mismatch");
		COMPV_CHECK_EXP_RETURN(xf != test->xf, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of xf mismatch");
		COMPV_CHECK_EXP_RETURN(yf != test->yf, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Sum of yf mismatch");
		COMPV_DEBUG_INFO_EX(TAG_UNITTESTS, "** Test OK **");
	}
	return COMPV_ERROR_CODE_S_OK;
}
