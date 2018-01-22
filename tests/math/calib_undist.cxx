#include "../tests_common.h"

#define TAG_TEST			"TestCalibUndist"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/calib"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/calib"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

// CameraInfo for (640x360) image [iPhone6]
/* Camera calibration info (generated using image with - 640x360) */
#define CAMERA_CALIB_WIDTH		640
#define CAMERA_CALIB_HEIGHT		360

#define CAMERA_INFO_INT_FU		546.26501464843750
#define CAMERA_INFO_INT_FV		408.30004882812500
#define CAMERA_INFO_INT_CX		321.85444091704994
#define CAMERA_INFO_INT_CY		175.33480867660546
#define CAMERA_INFO_INT_SKEW	0.0

#define CAMERA_INFO_DIST_K1		0.056450
#define CAMERA_INFO_DIST_K2		-0.111955
#define CAMERA_INFO_DIST_P1		0.
#define CAMERA_INFO_DIST_P2		0.

#define LOOP_COUNT				1
#define INTERP_TYPE				COMPV_INTERPOLATION_TYPE_BILINEAR

#define EXPECTED_MD5_BILINEAR	"2e41e846611ef2bb78914327a8f98480"
#define EXPECTED_MD5_NEAREST	"b727a212ead362cd276314e8ed067fab"

COMPV_ERROR_CODE calib_undist()
{
	CompVMatPtr distortedImage;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, CAMERA_CALIB_WIDTH, CAMERA_CALIB_HEIGHT, CAMERA_CALIB_WIDTH, COMPV_TEST_PATH_TO_FILE("distorted_640x360_gray.yuv").c_str(), &distortedImage));
	
	CompVMatPtr K, d;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&K, 3, 3));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&d, 4, 1));
	*K->ptr<double>(0, 0) = CAMERA_INFO_INT_FU;
	*K->ptr<double>(0, 1) = CAMERA_INFO_INT_SKEW;
	*K->ptr<double>(0, 2) = CAMERA_INFO_INT_CX;
	*K->ptr<double>(1, 0) = 0;
	*K->ptr<double>(1, 1) = CAMERA_INFO_INT_FV;
	*K->ptr<double>(1, 2) = CAMERA_INFO_INT_CY;
	*K->ptr<double>(2, 0) = 0;
	*K->ptr<double>(2, 1) = 0;
	*K->ptr<double>(2, 2) = 1;

	*d->ptr<double>(0, 0) = CAMERA_INFO_DIST_K1;
	*d->ptr<double>(1, 0) = CAMERA_INFO_DIST_K2;
	*d->ptr<double>(2, 0) = CAMERA_INFO_DIST_P1;
	*d->ptr<double>(3, 0) = CAMERA_INFO_DIST_P2;

	// Pre-build the map once (faster)
	CompVMatPtr undistortMap;
	COMPV_CHECK_CODE_RETURN(CompVCalibUtils::initUndistMap(CompVSizeSz(distortedImage->cols(), distortedImage->rows()), K, d, &undistortMap));

	// Process multiple times using the pre-built map
	CompVMatPtr undistortedImage;
	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVCalibUtils::undist2DImage(distortedImage, undistortMap, &undistortedImage, INTERP_TYPE));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(undist2DImage) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(undistortedImage).compare((INTERP_TYPE == COMPV_INTERPOLATION_TYPE_NEAREST) ? EXPECTED_MD5_NEAREST : EXPECTED_MD5_BILINEAR) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "MD5 mismatch");

	// dump latest image to file
#if COMPV_OS_WINDOWS && 1
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(undistortedImage, "undistorded.gray"));
#endif
	
	return COMPV_ERROR_CODE_S_OK;
}
