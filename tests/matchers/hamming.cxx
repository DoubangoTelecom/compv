#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG		"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
#define LOOP_COUNT		1
#define WIDTH_MINUS		0 // values used to unalign the width for testing (must be within [0, 3])

static const std::string expectedMD5[4/*WIDTH_MINUS*/] = 
{
	"5b0cb53ea712377f1c6991cbe38ced59",
	"856f29b2800ddce568fddb2bb98688eb",
	"007157ff4a485460b07e270b473ccf6b",
	"ddb55adf9bf32af942bc2881f633d27e"
};

bool TestHamming()
{
	CompVPtr<CompVImage *> image;
	uint64_t timeStart, timeEnd;
	int width, stride, height;
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	uint8_t* patch1xnPtr = NULL;
	compv_scalar_t* distPtr = NULL;

	// Decode the jpeg image
	COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	// Convert the image to grayscal (required by feture detectors)
	COMPV_CHECK_CODE_BAIL(err_ = image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	width = image->getWidth() - WIDTH_MINUS;
	stride = image->getStride();
	height = image->getHeight();

	patch1xnPtr = (uint8_t*)CompVMem::calloc(width, sizeof(uint8_t));
	COMPV_CHECK_EXP_BAIL(!patch1xnPtr, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

	distPtr = (compv_scalar_t*)CompVMem::calloc(width, sizeof(compv_scalar_t));
	COMPV_CHECK_EXP_BAIL(!distPtr, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

	// Detect keypoints
	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_BAIL(err_ = CompVHamming::distance((const uint8_t*)image->getDataPtr(), width, stride, height, patch1xnPtr, distPtr));
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time (TestHamming) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// Check MD5
	if (expectedMD5[WIDTH_MINUS] != CompVMd5::compute2(distPtr, width * sizeof(compv_scalar_t))) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		COMPV_ASSERT(false);
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}

bail:
	CompVMem::free((void**)&patch1xnPtr);
	CompVMem::free((void**)&distPtr);
	COMPV_CHECK_CODE_ASSERT(err_);
	return COMPV_ERROR_CODE_IS_OK(err_);
}
