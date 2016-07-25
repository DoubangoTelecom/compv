#include <compv/compv_api.h>
#include "../common.h"

#define TEST_TYPE_GIRL			0
#define TEST_TYPE_EQUIRECT		1
#define TEST_TYPE_MANDEKALOU	2
#define TEST_TYPE_VALVE			3

#define TEST_TYPE				TEST_TYPE_EQUIRECT

#define HOUGH_LOOP_COUNT		1

#if TEST_TYPE == TEST_TYPE_GIRL
#	define HOUGH_JPEG_IMG			"C:/Projects/GitHub/compv/tests/girl.jpg"
#	define HOUGH_MD5				"c059d425a7f25a9a6826ca5f27fd3010"
#elif TEST_TYPE == TEST_TYPE_EQUIRECT
#	define HOUGH_JPEG_IMG			"C:/Projects/GitHub/compv/tests/equirectangular.jpg"
#	define HOUGH_MD5				"dde715f52957af8742c58d967c8dedc7"
#elif TEST_TYPE == TEST_TYPE_MANDEKALOU
#	define HOUGH_JPEG_IMG			"C:/Projects/GitHub/compv/tests/mandekalou.jpg"
#	define HOUGH_MD5				"b84fd7cf64dde9711c5432e8ead285f4"
#elif TEST_TYPE == TEST_TYPE_VALVE
#	define HOUGH_JPEG_IMG			"C:/Projects/GitHub/compv/tests/Valve_original.jpg"
#	define HOUGH_MD5				"46c533ec76a54a5c9ef75946870dc109"
#endif

#define RHO			1.f
#define THETA		kfMathTrigPiOver180 // radian(1d)
#define THRESHOLD	100

using namespace compv;

COMPV_ERROR_CODE TestHoughStd()
{
	CompVPtr<CompVEdgeDete*> canny;
	CompVPtrArray(uint8_t) edges;
	CompVPtr<CompVImage *> image;
	CompVPtr<CompVHough* >hough;
	CompVPtrArray(compv_float32x2_t) coords;

	COMPV_CHECK_CODE_RETURN(CompVImageDecoder::decodeFile(HOUGH_JPEG_IMG, &image));
	COMPV_CHECK_CODE_RETURN(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(COMPV_CANNY_ID, &canny));
	COMPV_CHECK_CODE_RETURN(canny->process(image, edges));

	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(COMPV_HOUGH_STANDARD_ID, &hough, RHO, THETA, THRESHOLD));

	uint64_t timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < HOUGH_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(hough->process(edges, coords));
	}
	uint64_t timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time (TestHoughStd) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// Write to file
	COMPV_CHECK_CODE_RETURN(arrayToFile(edges));

	// Check MD5
	const std::string md5 = arrayMD5(coords);
	COMPV_CHECK_EXP_RETURN(md5 != HOUGH_MD5, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	return COMPV_ERROR_CODE_S_OK;
}