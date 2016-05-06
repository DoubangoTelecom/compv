#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG				"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg"

#define CONVLT_KERN_SIZE		3	// for 1 to 8
#define CONVLT_GAUSS_SIGMA		2
#define CONVLT_KERN_TYPE		float

#define CONVLT_LOOP_COUNT		1

static const std::string expectedMD5Values[8/*kenel size*/] =
{
	"", // 1
	"", // 2
	"e5b02d375e05d872440a2027cd9478e6", // 3
	"", // 4
	"adc35f0fe2d470d2ff4877f84456f711", // 5
	"", // 6
	"2ac7fb5afeeb12e3281446e70aef1f1d", // 7
	"", // 8
};

bool TestConvlt_fxp()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVPtr<CompVArray<uint16_t>* > kern_fxp;
	CompVPtr<CompVImage *> image;
	CompVPtr<CompVConvlt<CONVLT_KERN_TYPE> *> convlt;
	uint64_t timeStart, timeEnd;
	std::string expectedMD5;
	uint8_t* out_ptr;

	(out_ptr); (timeStart); (timeEnd);

	COMPV_ASSERT(CONVLT_KERN_SIZE & 1);
	COMPV_CHECK_CODE_BAIL(err_ = CompVGaussKern<CONVLT_KERN_TYPE>::buildKern1_fxp(&kern_fxp, CONVLT_KERN_SIZE, CONVLT_GAUSS_SIGMA));

	COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	COMPV_CHECK_CODE_BAIL(err_ = image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	COMPV_CHECK_CODE_BAIL(err_ = CompVConvlt<CONVLT_KERN_TYPE>::newObj(&convlt));

	// "out_ptr = in_ptr" to avoid allocating new buffer
	// set to NULL to force convlt  context to create a new buffer
	out_ptr = (uint8_t*)image->getDataPtr(); 

	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < CONVLT_LOOP_COUNT; ++i) {
		convlt->convlt1_fxp((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern_fxp->getDataPtr(), kern_fxp->getDataPtr(), CONVLT_KERN_SIZE, out_ptr);
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time(TestConvlt_fxp) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// dump image to file
	COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, out_ptr, image->getWidth(), image->getHeight(), image->getStride(), &image));
	writeImgToFile(image);

	expectedMD5 = CompVMd5::compute2(convlt->getResultPtr(), convlt->getResultSize());
	if (expectedMD5 != expectedMD5Values[CONVLT_KERN_SIZE - 1]) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}

bail:
	COMPV_CHECK_CODE_ASSERT(err_);
	return COMPV_ERROR_CODE_IS_OK(err_);
}
