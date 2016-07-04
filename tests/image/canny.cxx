#include <compv/compv_api.h>

#include "../common.h"

#define TEST_TYPE_GIRL			0
#define TEST_TYPE_EQUIRECT		1
#define TEST_TYPE_MANDEKALOU	2
#define TEST_TYPE_VALVE			3

#define TEST_TYPE				TEST_TYPE_GIRL

#define CANNY_LOOP_COUNT	1

#if TEST_TYPE == TEST_TYPE_GIRL
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/girl.jpg"
#	define CANNY_MD5				"8c186abac53615785ad81cc4e203bbaf"
#elif TEST_TYPE == TEST_TYPE_EQUIRECT
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/equirectangular.jpg"
#	define CANNY_MD5				"8cf08c0e06eea65b7554801db4f199fa"
#elif TEST_TYPE == TEST_TYPE_MANDEKALOU
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/mandekalou.jpg"
#	define CANNY_MD5				"9173ee8db078995d5e62eafbec9ed71d"
#elif TEST_TYPE == TEST_TYPE_VALVE
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/Valve_original.jpg"
#	define CANNY_MD5				"63b2b84cc318f51e42b9156dba6c03f1"
#endif

using namespace compv;

COMPV_ERROR_CODE TestCanny()
{
	CompVPtr<CompVEdgeDete*> canny;
	CompVPtrArray(uint8_t) edges;
	CompVPtr<CompVImage *> image;

	COMPV_CHECK_CODE_RETURN(CompVImageDecoder::decodeFile(CANNY_JPEG_IMG, &image));
	COMPV_CHECK_CODE_RETURN(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(COMPV_CANNY_ID, &canny));

	uint64_t timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < CANNY_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(canny->process(image, edges));
	}
	uint64_t timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time (TestCanny) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// Write to file
	COMPV_CHECK_CODE_RETURN(arrayToFile(edges));

	// Check MD5
	const std::string md5 = arrayMD5(edges);
	COMPV_CHECK_EXP_RETURN(md5 != CANNY_MD5, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	return COMPV_ERROR_CODE_S_OK;
}