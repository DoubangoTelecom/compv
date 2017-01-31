#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define FILE_NAME_EQUIRECTANGULAR	"C:/Projects/GitHub/data/test_images/equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK		"C:/Projects/GitHub/data/test_images/opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS			"C:/Projects/GitHub/data/test_images/mandekalou_480x640_gray.yuv"
#define CONVLT_KERN_TYPE			float

// FIXME: add support for 2d convolution
static const struct compv_unittest_convlt_flp {
	size_t kernelSize; // must be odd number
	CONVLT_KERN_TYPE sigma;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_CONVLT_FLP[] =
{
	{ 3, 0.99f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "cb6418ebd364c6016f704dc0c1762894" },
	{ 5, 3.99f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "b0f947dd48ef1c785ab1d456b2efc63f" },
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "96339db794cc1b8840603b9eac5b76fb" },
	{ 15, 4.47f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "61bbd16a6f0f6228794fa62b4f590516" },

	{ 3, 0.99f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "28abcce8232f2a95352a98f556eaa2c1" },
	{ 5, 3.99f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "83a17daee2a21eb179b67aa02f443656" },
	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "265ebac5d9e4597019afefc5fdc894ff" },
	{ 15, 4.47f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "aa4d3d834210cd09f8addcdc0d8766d7" },

	{ 3, 0.99f, FILE_NAME_GRIOTS, 480, 640, 480, "e6d2aaa8ceb0e0ab676f6f80dfb23724" },
	{ 5, 3.99f, FILE_NAME_GRIOTS, 480, 640, 480, "3878efacc846069e4ff79925f206578a" },
	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "cca863dab8612e3074d0d437703e5970" },
	{ 15, 4.47f, FILE_NAME_GRIOTS, 480, 640, 480, "5650ac3cd1361879f66ee307d3150330" },
};
static const size_t COMPV_UNITTEST_CONVLT_FLP_COUNT = sizeof(COMPV_UNITTEST_CONVLT_FLP) / sizeof(COMPV_UNITTEST_CONVLT_FLP[0]);

COMPV_ERROR_CODE TestConvlt_float()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVPtr<CompVArray<CONVLT_KERN_TYPE>* > kern;
    CompVPtr<CompVImage *> image;
    CompVPtr<CompVConvlt<CONVLT_KERN_TYPE> *> convlt;
    //uint64_t timeStart, timeEnd;
    std::string expectedMD5;
    uint8_t* out_ptr;
	CompVPtr<CompVBuffer *> buffer;
	const compv_unittest_convlt_flp* test = NULL;

	for (size_t i = 0; i < COMPV_UNITTEST_CONVLT_FLP_COUNT; ++i) {
		test = &COMPV_UNITTEST_CONVLT_FLP[i];
		COMPV_CHECK_CODE_RETURN(CompVGaussKern<CONVLT_KERN_TYPE>::buildKern1(&kern, (int)test->kernelSize, test->sigma));

		COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(test->filename, &buffer));
		COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, buffer->getPtr(), (int32_t)test->width, (int32_t)test->height, (int32_t)test->width, &image));

		COMPV_CHECK_CODE_RETURN(CompVConvlt<CONVLT_KERN_TYPE>::newObj(&convlt));

		// "out_ptr = in_ptr" to avoid allocating new buffer
		// set to NULL to force convlt  context to create a new buffer
		out_ptr = (uint8_t*)image->getDataPtr();

		convlt->convlt1((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern->ptr(), kern->ptr(), (int)test->kernelSize, out_ptr);

		// dump image to file
		COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, out_ptr, image->getWidth(), image->getHeight(), image->getStride(), &image));

		//COMPV_DEBUG_INFO("MD5: %s", CompVMd5::compute2(convlt->getResultPtr(), convlt->getResultSize()).c_str());
		COMPV_DEBUG_INFO("MD5: %s", imageMD5(image).c_str());
		//writeImgToFile(image);
	}

#if 0
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
        COMPV_CHECK_CODE_BAIL(err_ = CompVArray<CONVLT_KERN_TYPE>::newObj(&kern, CONVLT_KERN_SIZE, CONVLT_KERN_SIZE, false));
        CONVLT_KERN_TYPE sum;
        CONVLT_KERN_TYPE *kern_ptr = (CONVLT_KERN_TYPE*)kern->ptr();
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

	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(FILE_NAME_EQUIRECTANGULAR, &buffer));
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, buffer->getPtr(), 1282, 720, 1282, &image));

    COMPV_CHECK_CODE_BAIL(err_ = CompVConvlt<CONVLT_KERN_TYPE>::newObj(&convlt));

    // "out_ptr = in_ptr" to avoid allocating new buffer
    // set to NULL to force convlt  context to create a new buffer
    out_ptr = (uint8_t*)image->getDataPtr();

    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < CONVLT_LOOP_COUNT; ++i) {
#if CONVLT_DIM == 1
        convlt->convlt1((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern->ptr(), kern->ptr(), CONVLT_KERN_SIZE, out_ptr);
#else
        convlt->convlt2((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern->ptr(), CONVLT_KERNEL_SIZE, out_ptr);
#endif
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time(TestConvlt_float) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    // dump image to file
    COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, out_ptr, image->getWidth(), image->getHeight(), image->getStride(), &image));
    writeImgToFile(image);

    expectedMD5 = CompVMd5::compute2(convlt->getResultPtr(), convlt->getResultSize());
    if (expectedMD5 != expectedMD5Values[CONVLT_DIM - 1][CONVLT_KERN_SIZE - 1]) {
        COMPV_DEBUG_ERROR("MD5 mismatch");
        COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_UNITTEST_FAILED);
    }

#endif
    return err_;
}
