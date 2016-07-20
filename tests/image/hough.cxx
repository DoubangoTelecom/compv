#include <compv/compv_api.h>
#include "../common.h"

#define JPEG_IMG		"C:/Projects/GitHub/compv/tests/line_hz.jpg"
#define LOOP_COUNT		1

#define RHO			1
#define THETA		kfMathTrigPiOver180 // radian(1d)
#define THRESHOLD	1

using namespace compv;

COMPV_ERROR_CODE TestHoughStd()
{
	CompVPtr<CompVEdgeDete*> canny;
	CompVPtrArray(uint8_t) edges;
	CompVPtr<CompVImage *> image;
	CompVPtr<CompVHough* >hough;

	COMPV_CHECK_CODE_RETURN(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	COMPV_CHECK_CODE_RETURN(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(COMPV_CANNY_ID, &canny));
	COMPV_CHECK_CODE_RETURN(canny->process(image, edges));

	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(COMPV_HOUGH_STANDARD_ID, &hough));

	uint64_t timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(hough->process(edges));
	}
	uint64_t timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time (TestHoughStd) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// Write to file
	COMPV_CHECK_CODE_RETURN(arrayToFile(edges));

	// Check MD5
	//const std::string md5 = arrayMD5(edges);
	//COMPV_CHECK_EXP_RETURN(md5 != CANNY_MD5, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	return COMPV_ERROR_CODE_S_OK;
}