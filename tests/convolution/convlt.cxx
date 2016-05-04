#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG				"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg"

#define CONVLT_KERN_SIZE		3	// for 1 to 8
#define CONVLT_DIM				1	// #1 or #2 dimension 
#define CONVLT_GAUSS_SIGMA		2
#define CONVLT_KERN_TYPE		float

#define CONVLT_LOOP_COUNT		1

static const std::string expectedMD5Values[2/*dim*/][8/*kenel size*/] =
{
	/* Dimension 1 */
	{
		"", // 1
		"", // 2
		"bf673b02400a9a1393bc37438474e34e", // 3
		"", // 4
		"c00b313e62541dc2d4fa963404ea2187", // 5
		"", // 6
		"6326fef13e8cf1e74eb837c13fc6a6ff", // 7
		"", // 8
	}
	/* Dimension 2 */
	,
	{
		"", // 1
		"", // 2
		"", // 3
		"", // 4
		"", // 5
		"", // 6
		"07f7ed1a8d7a103669b2ea5cdd499437", // 7
		"", // 8
	}
};

bool TestConvlt()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	CompVPtr<CompVArray<CONVLT_KERN_TYPE>* > kern;
	CompVPtr<CompVImage *> image;
	CompVPtr<CompVConvlt<CONVLT_KERN_TYPE> *> convlt;
	uint64_t timeStart, timeEnd;
	std::string expectedMD5;
	uint8_t* out_ptr;

	if (CONVLT_KERN_SIZE > 1 && CONVLT_KERN_SIZE & 1) {
		// For odd sizes, use Gaussian kernels
		if (CONVLT_DIM == 2) {
			COMPV_CHECK_CODE_BAIL(err_ = CompVGaussKern<CONVLT_KERN_TYPE>::buildKern2(&kern, CONVLT_KERN_SIZE, CONVLT_GAUSS_SIGMA));
		}
		else {
			COMPV_CHECK_CODE_BAIL(err_ = CompVGaussKern<CONVLT_KERN_TYPE>::buildKern1(&kern, CONVLT_KERN_SIZE, CONVLT_GAUSS_SIGMA));
		}
	}
	else {
		COMPV_CHECK_CODE_BAIL(err_ = CompVArray<CONVLT_KERN_TYPE>::newObj(&kern, CONVLT_DIM, CONVLT_KERN_SIZE, CONVLT_KERN_SIZE));
		CONVLT_KERN_TYPE sum;
		CONVLT_KERN_TYPE *kern_ptr = (CONVLT_KERN_TYPE*)kern->getDataPtr();
		for (int j = 0; j < CONVLT_DIM; ++j) {
			sum = 0;
			// random coeffs.
			for (int i = 0; i < CONVLT_KERN_SIZE; ++i) {
				kern_ptr[i] = ((CONVLT_KERN_TYPE)rand() / ((CONVLT_KERN_TYPE)rand() + 1));
				sum += kern_ptr[i];
			}
			// Normalize
			sum = 1 / sum;
			for (int i = 0; i < CONVLT_KERN_SIZE; ++i) {
				kern_ptr[i] *= sum;
			}
			kern_ptr += CONVLT_KERN_SIZE;
		}
	}

	COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	COMPV_CHECK_CODE_BAIL(err_ = image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

	COMPV_CHECK_CODE_BAIL(err_ = CompVConvlt<CONVLT_KERN_TYPE>::newObj(&convlt));

	// "out_ptr = in_ptr" to avoid allocating new buffer
	// set to NULL to force convlt  context to create a new buffer
	out_ptr = (uint8_t*)image->getDataPtr(); 

	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < CONVLT_LOOP_COUNT; ++i) {
#if CONVLT_DIM == 1
		convlt->convlt1((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern->getDataPtr(), kern->getDataPtr(), CONVLT_KERN_SIZE, out_ptr);
#else
		convlt->convlt2((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern->getDataPtr(), CONVLT_KERNEL_SIZE, out_ptr);
#endif
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time(TestConvlt) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// dump image to file
	COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, out_ptr, image->getWidth(), image->getHeight(), image->getStride(), &image));
	writeImgToFile(image);

	expectedMD5 = CompVMd5::compute2(convlt->getResultPtr(), convlt->getResultSize());
	if (expectedMD5 != expectedMD5Values[CONVLT_DIM - 1][CONVLT_KERN_SIZE - 1]) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	}

bail:
	COMPV_CHECK_CODE_ASSERT(err_);
	return COMPV_ERROR_CODE_IS_OK(err_);
}
