#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
#define LOOP_COUNT	1

bool TestHamming()
{
	CompVPtr<CompVImage *> image;
	uint64_t timeStart, timeEnd;
	size_t dataSize;
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	void* memPtr = NULL;

	// Decode the jpeg image
	COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	// Convert the image to grayscal (required by feture detectors)
	COMPV_CHECK_CODE_BAIL(err_ = image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	dataSize = image->getDataSize();
	memPtr = CompVMem::calloc(dataSize, 1);
	COMPV_CHECK_EXP_BAIL(!memPtr, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

	// Detect keypoints
	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time (TestHamming) = [[[ %llu millis ]]]", (timeEnd - timeStart));

bail:
	CompVMem::free(&memPtr);
	COMPV_CHECK_CODE_ASSERT(err_);
	return COMPV_ERROR_CODE_IS_OK(err_);
}
