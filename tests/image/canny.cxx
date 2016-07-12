#include <compv/compv_api.h>

#include "../common.h"

// Depending on the number of threads, the mean value could be "+-1" compared to the single threaded version
// -> Unit-test -> use 8 threads

#define TEST_TYPE_GIRL			0
#define TEST_TYPE_EQUIRECT		1
#define TEST_TYPE_MANDEKALOU	2
#define TEST_TYPE_VALVE			3

#define TEST_TYPE				TEST_TYPE_EQUIRECT

#define CANNY_LOOP_COUNT		500

#if TEST_TYPE == TEST_TYPE_GIRL
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/girl.jpg"
#	define CANNY_MD5				"bb748a24b280717614dc43bf5bba2f2c"
#elif TEST_TYPE == TEST_TYPE_EQUIRECT
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/equirectangular.jpg"
#	define CANNY_MD5				"2476dd77aa998a90723a767ca56da4e3"
#elif TEST_TYPE == TEST_TYPE_MANDEKALOU
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/mandekalou.jpg"
#	define CANNY_MD5				"ec02f12a204dc0c9368f6decfd30842f"
#elif TEST_TYPE == TEST_TYPE_VALVE
#	define CANNY_JPEG_IMG			"C:/Projects/GitHub/compv/tests/Valve_original.jpg"
#	define CANNY_MD5				"bb1973a618532af24529d8e08d1184b7"
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