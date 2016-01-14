#include <compv/compv_api.h>

#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
//#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/3867257549_6ca855e08d_o.jpg" //paris (BIIIIG)

using namespace compv;

#define MODE_RGBA	0
#define MODE_ARGB	1
#define MODE_BGRA	2
#define MODE_ABGR	3
#define MODE_RGB	4
#define MODE_BGR	5

#define MODE		MODE_RGBA

static void rgbToRGBA(const CompVObjWrapper<CompVImage *>& jpegImage, void** rgbaPtr, int &height, int &width, int &stride)
{
#define WIDTH_OFFSET -1 // amount of pixels to remove to width to make it wired (not standard)
#define HEIGHT_OFFSET 0 // amount of pixels to remove to height to make it wired (not standard)	
    int jpegImageStride = jpegImage->getStride();
    width = jpegImage->getWidth() + WIDTH_OFFSET;
    height = jpegImage->getHeight() + HEIGHT_OFFSET;
    COMPV_CHECK_CODE_ASSERT(CompVImage::getBestStride(jpegImageStride, &stride));

#if MODE == MODE_RGBA || MODE == MODE_ARGB || MODE == MODE_BGRA || MODE == MODE_ABGR
    int compSize = 4;
#elif MODE == MODE_RGB || MODE == MODE_BGR
    int compSize = 3;
#else
#error "invalid mode"
#endif

    *rgbaPtr = CompVMem::malloc((stride * jpegImage->getHeight()) * compSize);
    COMPV_ASSERT(*rgbaPtr != NULL);
    const uint8_t *rgbPtr_ = (const uint8_t *)jpegImage->getDataPtr();
    uint8_t* rgbaPtr_ = (uint8_t*)*rgbaPtr;
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
#if MODE == MODE_RGBA
            rgbaPtr_[0] = rgbPtr_[0];
            rgbaPtr_[1] = rgbPtr_[1];
            rgbaPtr_[2] = rgbPtr_[2];
            rgbaPtr_[3] = 0xFF;
#elif MODE == MODE_ARGB
            rgbaPtr_[0] = 0xFF;
            rgbaPtr_[1] = rgbPtr_[0];
            rgbaPtr_[2] = rgbPtr_[1];
            rgbaPtr_[3] = rgbPtr_[2];
#elif MODE == MODE_BGRA
            rgbaPtr_[0] = rgbPtr_[2];
            rgbaPtr_[1] = rgbPtr_[1];
            rgbaPtr_[2] = rgbPtr_[0];
            rgbaPtr_[3] = 0xFF;
#elif MODE == MODE_ABGR
            rgbaPtr_[0] = 0xFF;
            rgbaPtr_[1] = rgbPtr_[2];
            rgbaPtr_[2] = rgbPtr_[1];
            rgbaPtr_[3] = rgbPtr_[0];
#elif MODE == MODE_RGB
            rgbaPtr_[0] = rgbPtr_[0];
            rgbaPtr_[1] = rgbPtr_[1];
            rgbaPtr_[2] = rgbPtr_[2];
#elif MODE == MODE_BGR
            rgbaPtr_[0] = rgbPtr_[2];
            rgbaPtr_[1] = rgbPtr_[1];
            rgbaPtr_[2] = rgbPtr_[0];
#elif
#error "invalid mode"
#endif
            rgbaPtr_ += compSize;
            rgbPtr_ += 3;
        }
        rgbaPtr_ += (stride - width) * compSize;
        rgbPtr_ += (jpegImageStride - width) * 3;
    }
}

bool TestRgba()
{
    CompVObjWrapper<CompVImage *> jpegImage;
    CompVObjWrapper<CompVImage *> i420Image;
    CompVObjWrapper<CompVImage *> rgbaImage;
    void* rgbaPtr = NULL;
    int width, height, stride;
    uint64_t timeStart, timeEnd;

    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_EQUIRECTANGULAR_FILE, &jpegImage));
    COMPV_ASSERT(jpegImage->getPixelFormat() == COMPV_PIXEL_FORMAT_R8G8B8);
    rgbToRGBA(jpegImage, &rgbaPtr, height, width, stride);
#if MODE == MODE_RGBA
    COMPV_ERROR_CODE(*toRGBA)(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* rgba) = CompVImageConv::i420ToRGBA;
    COMPV_ERROR_CODE(*toI420)(const uint8_t* rgbaPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* i420) = CompVImageConv::rgbaToI420;
#elif MODE == MODE_ARGB
#error "invalid mode"
#elif MODE == MODE_BGRA
#error "invalid mode"
#elif MODE == MODE_ABGR
#error "invalid mode"
#elif MODE == MODE_RGB
#error "invalid mode"
#elif MODE == MODE_BGR
#error "invalid mode"
#elif
#error "invalid mode"
#endif

#define loopCount  1
    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < loopCount; ++i) {
        COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_R8G8B8A8, rgbaPtr, width, height, stride, &rgbaImage));
        COMPV_CHECK_CODE_ASSERT(toI420((const uint8_t*)rgbaPtr, height, width, stride, &i420Image));
#if 1 // I420 to RGBA then RGBA to I420
        const uint8_t* yPtr = (const uint8_t*)i420Image->getDataPtr();
        const uint8_t* uPtr = yPtr + (i420Image->getHeight() * i420Image->getStride());
        const uint8_t* vPtr = uPtr + ((i420Image->getHeight() * i420Image->getStride()) >> 2);
        COMPV_CHECK_CODE_ASSERT(toRGBA(yPtr, uPtr, vPtr, i420Image->getHeight(), i420Image->getWidth(), i420Image->getStride(), &rgbaImage));
        COMPV_CHECK_CODE_ASSERT(toI420((const uint8_t*)rgbaImage->getDataPtr(), rgbaImage->getHeight(), rgbaImage->getWidth(), rgbaImage->getStride(), &i420Image));
#endif
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time =%llu", (timeEnd - timeStart));

    if (i420Image) {
        FILE* fileI420 = fopen("./out.yuv", "wb+");
        COMPV_ASSERT(fileI420 != NULL);
        fwrite(i420Image->getDataPtr(), 1, i420Image->getDataSize(), fileI420);
        fclose(fileI420);
    }
    if (rgbaImage) {
        FILE* fileRGBA = fopen("./out.rgba", "wb+"); // Open with imageMagick (MS-DOS): convert.exe -depth 8 -size 2048x1000 out.rgba out.png
        COMPV_ASSERT(fileRGBA != NULL);
        fwrite(rgbaImage->getDataPtr(), 1, rgbaImage->getDataSize(), fileRGBA);
        fclose(fileRGBA);
    }

    CompVMem::free(&rgbaPtr);
    return true;
}
