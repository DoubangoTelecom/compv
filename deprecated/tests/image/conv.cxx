#include <compv/compv_api.h>
#include "../common.h"

#define JPEG_EQUIRECTANGULAR_FILE "C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
//#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/3867257549_6ca855e08d_o.jpg" //paris (BIIIIG)

using namespace compv;

// preprocessor cannot evaluate an enum
// #if FORMAT_SRC == COMPV_PIXEL_FORMAT_R8G8B8A8 is always true
#define FORMAT_RGBA			3 // COMPV_PIXEL_FORMAT_R8G8B8A8
#define FORMAT_ARGB			6 // COMPV_PIXEL_FORMAT_A8R8G8B8
#define FORMAT_BGRA			4 // COMPV_PIXEL_FORMAT_B8G8R8A8
#define FORMAT_ABGR			5 // COMPV_PIXEL_FORMAT_A8B8G8R8
#define FORMAT_RGB			1 // COMPV_PIXEL_FORMAT_R8G8B8
#define FORMAT_BGR			2 // COMPV_PIXEL_FORMAT_B8G8R8

#define FORMAT_GRAYSCALE	7 // COMPV_PIXEL_FORMAT_GRAYSCALE

#define FORMAT_I420			8 // COMPV_PIXEL_FORMAT_I420

#define loopCount			1
#define MD5_PRINT			1
#define FORMAT_SRC			FORMAT_BGR // must be rgb or rgba family
#define FORMAT_DST			FORMAT_I420 // any format
#define STRIDE_ALIGN		true // false to test CompVImage::wrap and CompVImage::copy

static COMPV_ERROR_CODE rgbToSrc(const CompVPtr<CompVImage *>& jpegImage, void** srcPtr, int &height, int &width, int &stride)
{
    // TODO(dmi): WIDTH_OFFSET -1 doesn't work or YUV player is buggy -> https://github.com/DoubangoTelecom/compv/issues/49
#define WIDTH_OFFSET	-2 // amount of pixels to remove to width to make it wired (not standard)
#define HEIGHT_OFFSET	0 // amount of pixels to remove to height to make it wired (not standard)	
    int jpegImageStride = jpegImage->getStride();
    width = jpegImage->getWidth() + WIDTH_OFFSET;
    height = jpegImage->getHeight() + HEIGHT_OFFSET;
    if (STRIDE_ALIGN) {
        COMPV_CHECK_CODE_RETURN(CompVImage::getBestStride(jpegImageStride, &stride));
    }
    else {
        stride = jpegImageStride + 2;
    }

#if FORMAT_SRC == FORMAT_RGBA || FORMAT_SRC == FORMAT_ARGB || FORMAT_SRC == FORMAT_BGRA || FORMAT_SRC == FORMAT_ABGR
    int compSize = 4;
#elif FORMAT_SRC == FORMAT_RGB || FORMAT_SRC == FORMAT_BGR
    int compSize = 3;
#else
#error "invalid mode"
#endif

    *srcPtr = CompVMem::malloc((stride * jpegImage->getHeight()) * compSize);
    COMPV_ASSERT(*srcPtr != NULL);
    const uint8_t *rgbPtr_ = (const uint8_t *)jpegImage->getDataPtr();
    uint8_t* srcPtr_ = (uint8_t*)*srcPtr;
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
#if FORMAT_SRC == FORMAT_RGBA
            srcPtr_[0] = rgbPtr_[0];
            srcPtr_[1] = rgbPtr_[1];
            srcPtr_[2] = rgbPtr_[2];
            srcPtr_[3] = 0xFF;
#elif FORMAT_SRC == FORMAT_ARGB
            srcPtr_[0] = 0xFF;
            srcPtr_[1] = rgbPtr_[0];
            srcPtr_[2] = rgbPtr_[1];
            srcPtr_[3] = rgbPtr_[2];
#elif FORMAT_SRC == FORMAT_BGRA
            srcPtr_[0] = rgbPtr_[2];
            srcPtr_[1] = rgbPtr_[1];
            srcPtr_[2] = rgbPtr_[0];
            srcPtr_[3] = 0xFF;
#elif FORMAT_SRC == FORMAT_ABGR
            srcPtr_[0] = 0xFF;
            srcPtr_[1] = rgbPtr_[2];
            srcPtr_[2] = rgbPtr_[1];
            srcPtr_[3] = rgbPtr_[0];
#elif FORMAT_SRC == FORMAT_RGB
            srcPtr_[0] = rgbPtr_[0];
            srcPtr_[1] = rgbPtr_[1];
            srcPtr_[2] = rgbPtr_[2];
#elif FORMAT_SRC == FORMAT_BGR
            srcPtr_[0] = rgbPtr_[2];
            srcPtr_[1] = rgbPtr_[1];
            srcPtr_[2] = rgbPtr_[0];
#elif
#error "invalid mode"
#endif
            srcPtr_ += compSize;
            rgbPtr_ += 3;
        }
        srcPtr_ += (stride - width) * compSize;
        rgbPtr_ += (jpegImageStride - width) * 3;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE TestConv()
{
    CompVPtr<CompVImage *> jpegImage;
    CompVPtr<CompVImage *> dstImage;
    CompVPtr<CompVImage *> srcImage;
    void* srcPtr = NULL;
    int width, height, stride;
    uint64_t timeStart, timeEnd;

    // make sure the defs maps to the enums
    COMPV_ASSERT(FORMAT_RGBA == COMPV_PIXEL_FORMAT_R8G8B8A8);
    COMPV_ASSERT(FORMAT_ARGB == COMPV_PIXEL_FORMAT_A8R8G8B8);
    COMPV_ASSERT(FORMAT_BGRA == COMPV_PIXEL_FORMAT_B8G8R8A8);
    COMPV_ASSERT(FORMAT_ABGR == COMPV_PIXEL_FORMAT_A8B8G8R8);
    COMPV_ASSERT(FORMAT_RGB == COMPV_PIXEL_FORMAT_R8G8B8);
    COMPV_ASSERT(FORMAT_BGR == COMPV_PIXEL_FORMAT_B8G8R8);
    COMPV_ASSERT(FORMAT_I420 == COMPV_PIXEL_FORMAT_I420);

    COMPV_CHECK_CODE_RETURN(CompVImageDecoder::decodeFile(JPEG_EQUIRECTANGULAR_FILE, &jpegImage));
    COMPV_ASSERT(jpegImage->getPixelFormat() == COMPV_PIXEL_FORMAT_R8G8B8);
    COMPV_CHECK_CODE_RETURN(rgbToSrc(jpegImage, &srcPtr, height, width, stride));
    COMPV_CHECK_CODE_RETURN(CompVImage::wrap((COMPV_PIXEL_FORMAT)FORMAT_SRC, srcPtr, width, height, stride, &srcImage)); // FIXME: slooow

    COMPV_DEBUG_INFO("Converting from %s to %s", formatExtension((COMPV_PIXEL_FORMAT)FORMAT_SRC).c_str(), formatExtension((COMPV_PIXEL_FORMAT)FORMAT_DST).c_str());

    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < loopCount; ++i) {
        COMPV_CHECK_CODE_RETURN(srcImage->convert((COMPV_PIXEL_FORMAT)FORMAT_DST, &dstImage)); // e.g. RGBA -> I420
#if FORMAT_SRC == FORMAT_RGBA && 0 // only I420 -> RGBA is supported
        const uint8_t* yPtr = (const uint8_t*)dstImage->getDataPtr();
        const uint8_t* uPtr = yPtr + (dstImage->getHeight() * dstImage->getStride());
        const uint8_t* vPtr = uPtr + ((dstImage->getHeight() * dstImage->getStride()) >> 2);
        COMPV_CHECK_CODE_RETURN(dstImage->convert((COMPV_PIXEL_FORMAT)FORMAT_SRC, &srcImage)); // // I420 -> RGBA
        COMPV_CHECK_CODE_RETURN(srcImage->convert(COMPV_PIXEL_FORMAT_I420, &dstImage)); // // RGBA -> I420
#endif
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

#if MD5_PRINT
    COMPV_DEBUG_INFO("Conversion result:\nwithout borders->(md5=%s, w=%d, s=%d, h=%d)\nwith borders->(md5=%s, w=%d, s=%d, h=%d)",
                     imageMD5(dstImage, COMPV_BORDER_POS_ALL).c_str(), dstImage->getWidth(), dstImage->getStride(), dstImage->getHeight(COMPV_BORDER_POS_ALL),
                     imageMD5(dstImage, COMPV_BORDER_POS_NONE).c_str(), dstImage->getWidth(), dstImage->getStride(), dstImage->getHeight(COMPV_BORDER_POS_NONE)
                    );
#endif

    // Open with imageMagick (MS-DOS): convert.exe -depth 8 -size 2048x1000 out.rgba out.png

    writeImgToFile(srcImage);
    writeImgToFile(dstImage);

    CompVMem::free(&srcPtr);
    return COMPV_ERROR_CODE_S_OK;
}
