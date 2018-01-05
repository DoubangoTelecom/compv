#include "../tests_common.h"

#define TAG_TEST			"TestCalibCamera"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/calib"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/calib"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define COMPV_TEST_CANNY_THRESHOLD_LOW			2.41f
#define COMPV_TEST_CANNY_THRESHOLD_HIGH			(COMPV_TEST_CANNY_THRESHOLD_LOW * 2.f)

#define COMPV_TEST_HOUGH_RHO					(1.0f * 0.5f) // "rho-delta" (half-pixel)
#define COMPV_TEST_HOUGH_THETA					(kfMathTrigPiOver180 * 0.5f) // "theta-delta" (half-degree)
#define COMPV_TEST_HOUGH_THRESHOLD				10 // minumum number of aligned points to form a line (also used in NMS)

#define COMPV_TEST_NUM_IMAGES					11 // Number of images to process
#define COMPV_TEST_PATTERN_ROW_CORNERS_NUM		10 // Number of corners per row
#define COMPV_TEST_PATTERN_COL_CORNERS_NUM		8  // Number of corners per column
#define COMPV_TEST_PATTERN_CORNERS_NUM			(COMPV_TEST_PATTERN_ROW_CORNERS_NUM * COMPV_TEST_PATTERN_COL_CORNERS_NUM) // Total number of corners

COMPV_ERROR_CODE calib_camera()
{
	CompVEdgeDetePtr canny;
	CompVHoughPtr hough;

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&canny, COMPV_CANNY_ID, COMPV_TEST_CANNY_THRESHOLD_LOW, COMPV_TEST_CANNY_THRESHOLD_HIGH));
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&hough, COMPV_HOUGHKHT_ID, COMPV_TEST_HOUGH_RHO, COMPV_TEST_HOUGH_THETA, COMPV_TEST_HOUGH_THRESHOLD));

	for (size_t n = 1010047; n < (1010047 + COMPV_TEST_NUM_IMAGES); ++n) {
		CompVMatPtr image;
		std::string image_name = std::string("P") + CompVBase::to_string(n) +std::string("s_640x480_gray.yuv");
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, 640, 480, 640, COMPV_TEST_PATH_TO_FILE(image_name.c_str()).c_str(), &image));
	}
	return COMPV_ERROR_CODE_S_OK;
}
