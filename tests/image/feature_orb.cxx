#include "../tests_common.h"

// This sample is for testing multithreading only, do not integrate in unittests or test on ARM

#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define TEST_TYPE_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define TEST_TYPE_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define TEST_TYPE_GRIOTS				"mandekalou_480x640_gray.yuv"

#define LOOP_COUNT		1
#define TEST_TYPE		TEST_TYPE_EQUIRECTANGULAR
#define NONMAXIMA		true
#define THRESHOLD		20
#define FASTID			COMPV_FAST_TYPE_9
#define MAXFEATURES		-1
#define PYRAMID_LEVELS			8
#define PYRAMID_SCALE_FACTOR	0.83f // (1 / 1.2)
#if COMPV_ARCH_X86
#	define EXPECTED_MD5			"966fcd4061e9deeb42259dd2b119af09" // AVX2, FixedPoint, FMA, ::sin, ::cos... (this is really *my* local test to check multithreading)
#elif COMPV_ARCH_ARM // ::sin, ::cos results on Android and iOS are different
#   if COMPV_OS_IPHONE
#       define EXPECTED_MD5         "1cc349eecf2aa8b7b682c922854b4daa"
#   else
#       define EXPECTED_MD5			"dd35e1c1d92e3151d2ae35a5a2fb78d8"
#   endif
#endif

COMPV_ERROR_CODE feature_orb()
{
	CompVCornerDetePtr dete;
	CompVCornerDescPtr dec;
	CompVMatPtr image, descriptions;
	std::vector<CompVInterestPoint> interestPoints;
	uint64_t timeStart, timeEnd;

	// Read file
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(TEST_TYPE_EQUIRECTANGULAR).c_str(), &image));

	// ORB detector
	COMPV_CHECK_CODE_RETURN(CompVCornerDete::newObj(&dete, COMPV_ORB_ID));
	COMPV_CHECK_CODE_RETURN(dete->setInt(COMPV_ORB_SET_INT_FAST_THRESHOLD, THRESHOLD));
	COMPV_CHECK_CODE_RETURN(dete->setInt(COMPV_ORB_SET_INT_INTERNAL_DETE_ID, FASTID));
	COMPV_CHECK_CODE_RETURN(dete->setInt(COMPV_ORB_SET_INT_MAX_FEATURES, MAXFEATURES));
	COMPV_CHECK_CODE_RETURN(dete->setBool(COMPV_ORB_SET_BOOL_FAST_NON_MAXIMA_SUPP, NONMAXIMA));
	COMPV_CHECK_CODE_RETURN(dete->setInt(COMPV_ORB_SET_INT_PYRAMID_LEVELS, PYRAMID_LEVELS));
	COMPV_CHECK_CODE_RETURN(dete->setFloat32(COMPV_ORB_SET_FLT32_PYRAMID_SCALE_FACTOR, PYRAMID_SCALE_FACTOR));

	// ORB descriptor
	COMPV_CHECK_CODE_RETURN(CompVCornerDesc::newObj(&dec, COMPV_ORB_ID, dete));
	
	timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(dete->process(image, interestPoints)); // detect
	}
	timeEnd = CompVTime::nowMillis();
	
	COMPV_DEBUG_INFO("Elapsed time (TestORB) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_CODE_RETURN(dec->process(image, interestPoints, &descriptions)); // describe

	COMPV_DEBUG_INFO("MD5:%s", compv_tests_md5(descriptions).c_str());

#if LOOP_COUNT == 1
	// Check result (Important: you have to disable MT in convolution to have same MD5, because in ORB the convoltion change the input image)
	COMPV_CHECK_EXP_RETURN(std::string(EXPECTED_MD5).compare(compv_tests_md5(descriptions)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "ORB: MD5 mismatch");
#endif

	return COMPV_ERROR_CODE_S_OK;
}
