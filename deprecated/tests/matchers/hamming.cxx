#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG		"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
#define WIDTH_MINUS		0 // values used to unalign the width for testing (must be within [0, 3])

static const std::string expectedMD5[4/*WIDTH_MINUS*/] = {
    "a3e0c0f04f2018e90cbc52963ddb4ba9",
    "2c89aecb3b1eeea6486574a473f03c6a",
    "80e5e5ec382607a5fb1d10a5a6248930",
    "fb7ba81a253c89416aa88ae9a2b7c389"
};

#define LOOP_COUNT		1

bool TestHamming()
{
    CompVPtr<CompVImage *> image;
    uint64_t timeStart, timeEnd;
    int width, stride, height;
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    uint8_t* patch1xnPtr = NULL;
    int32_t* distPtr = NULL;

    // Decode the jpeg image
    COMPV_CHECK_CODE_BAIL(err_ = CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    // Convert the image to grayscal (required by feture detectors)
    COMPV_CHECK_CODE_BAIL(err_ = image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));

    width = image->getWidth() - WIDTH_MINUS;
    stride = image->getStride();
    height = image->getHeight();

    patch1xnPtr = (uint8_t*)CompVMem::calloc(width, sizeof(uint8_t));
    COMPV_CHECK_EXP_BAIL(!patch1xnPtr, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

    distPtr = (int32_t*)CompVMem::calloc(width, sizeof(int32_t));
    COMPV_CHECK_EXP_BAIL(!distPtr, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

    // Detect keypoints
    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_BAIL(err_ = CompVHamming::distance((const uint8_t*)image->getDataPtr(), width, stride, height, patch1xnPtr, distPtr));
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time (TestHamming) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    // Check MD5
    if (expectedMD5[WIDTH_MINUS] != CompVMd5::compute2(distPtr, width * sizeof(int32_t))) {
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
