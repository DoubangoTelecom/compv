#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define FILE_NAME_EQUIRECTANGULAR	"C:/Projects/GitHub/data/test_images/equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK		"C:/Projects/GitHub/data/test_images/opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS			"C:/Projects/GitHub/data/test_images/mandekalou_480x640_gray.yuv"
#define CONVLT_KERN_TYPE			float

static const struct compv_unittest_convlt_fxp {
	size_t kernelSize; // must be odd number
	CONVLT_KERN_TYPE sigma;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_CONVLT_FXP[] =
{
	{ 3, 0.99f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "22377634ec505f2a85724a1edf849516" },
	{ 5, 3.99f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "5050e51df31d1224fc4e6aee2562e3b2" },
	{ 7, 2.0f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "4651c698d65246b282e4ab3115edc144" },
	{ 15, 4.47f, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "316d849c1cd4316ed004625b3f84c6f9" },

	{ 3, 0.99f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "7da3788edec4a5c0bf7f94ff4a34aff2" },
	{ 5, 3.99f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "e54beeb2e542909e511f74d45522f100" },
	{ 7, 2.0f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "fe3f2045e26e603009abddb70e1d9088" },
	{ 15, 4.47f, FILE_NAME_OPENGLBOOK, 200, 258, 200, "153ec5c617a51abf20541117e55cee66" },

	{ 3, 0.99f, FILE_NAME_GRIOTS, 480, 640, 480, "4a24503ea152991bbc24e94871802acb" },
	{ 5, 3.99f, FILE_NAME_GRIOTS, 480, 640, 480, "4c87638972648000ed52db79bbf01bca" },
	{ 7, 2.0f, FILE_NAME_GRIOTS, 480, 640, 480, "176d72fdf5d9e166ca501418c29bb30b" },
	{ 15, 4.47f, FILE_NAME_GRIOTS, 480, 640, 480, "e56988253306d6ff1bc464dd0945e8e2" },
};
static const size_t COMPV_UNITTEST_CONVLT_FXP_COUNT = sizeof(COMPV_UNITTEST_CONVLT_FXP) / sizeof(COMPV_UNITTEST_CONVLT_FXP[0]);

#define CONVLT_KERN_SIZE		7	// odd number
#define CONVLT_GAUSS_SIGMA		2
#define CONVLT_LOOP_COUNT		1

COMPV_ERROR_CODE TestConvlt_fxp()
{
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVPtr<CompVArray<uint16_t>* > kern_fxp;
    CompVPtr<CompVImage *> image;
    CompVPtr<CompVConvlt<CONVLT_KERN_TYPE> *> convlt;
    uint64_t timeStart, timeEnd;
    std::string expectedMD5;
    uint8_t* out_ptr;
	CompVPtr<CompVBuffer *> buffer;
	const compv_unittest_convlt_fxp* test = NULL;

    (out_ptr);
    (timeStart);
    (timeEnd);

	for (size_t i = 0; i < COMPV_UNITTEST_CONVLT_FXP_COUNT; ++i) {
		test = &COMPV_UNITTEST_CONVLT_FXP[i];
		COMPV_CHECK_CODE_RETURN(CompVGaussKern<CONVLT_KERN_TYPE>::buildKern1_fxp(&kern_fxp, (int)test->kernelSize, test->sigma));

		COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(test->filename, &buffer));
		COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, buffer->getPtr(), (int32_t)test->width, (int32_t)test->height, (int32_t)test->width, &image));

		COMPV_CHECK_CODE_RETURN(CompVConvlt<CONVLT_KERN_TYPE>::newObj(&convlt));

		// "out_ptr = in_ptr" to avoid allocating new buffer
		// set to NULL to force convlt  context to create a new buffer
		out_ptr = (uint8_t*)image->getDataPtr();
		
		convlt->convlt1_fxp((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern_fxp->ptr(), kern_fxp->ptr(), (int)test->kernelSize, out_ptr);

		// dump image to file
		COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, out_ptr, image->getWidth(), image->getHeight(), image->getStride(), &image));

		//COMPV_DEBUG_INFO("MD5: %s", CompVMd5::compute2(convlt->getResultPtr(), convlt->getResultSize()).c_str());
		COMPV_DEBUG_INFO("MD5: %s", imageMD5(image).c_str());
		//writeImgToFile(image);
	}

#if 0
    COMPV_CHECK_CODE_BAIL(err_ = CompVGaussKern<CONVLT_KERN_TYPE>::buildKern1_fxp(&kern_fxp, CONVLT_KERN_SIZE, CONVLT_GAUSS_SIGMA));

    COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    COMPV_CHECK_CODE_BAIL(err_ = image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

    COMPV_CHECK_CODE_BAIL(err_ = CompVConvlt<CONVLT_KERN_TYPE>::newObj(&convlt));

    // "out_ptr = in_ptr" to avoid allocating new buffer
    // set to NULL to force convlt  context to create a new buffer
    out_ptr = (uint8_t*)image->getDataPtr();

    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < CONVLT_LOOP_COUNT; ++i) {
        convlt->convlt1_fxp((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern_fxp->ptr(), kern_fxp->ptr(), CONVLT_KERN_SIZE, out_ptr);
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
#endif
	
    return err_;
}
