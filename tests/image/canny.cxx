#include <compv/compv_api.h>

#include "../common.h"

#define CANNY_JPEG_IMG		"C:/Projects/GitHub/compv/tests/canny.jpg" //C:/Projects/GitHub/compv/tests/Bikesgray.jpg" // "C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
#define CANNY_MD5			"08739c6f92a579f08cc7b417ec20e243"
#define CANNY_LOOP_COUNT	1

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
	//const std::string md5 = arrayMD5(edges);
	//COMPV_CHECK_EXP_RETURN(md5 != CANNY_MD5, COMPV_ERROR_CODE_E_UNITTEST_FAILED);

	return COMPV_ERROR_CODE_S_OK;
}