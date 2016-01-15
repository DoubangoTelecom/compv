#include <compv/compv_api.h>

#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
//#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/3867257549_6ca855e08d_o.jpg" //paris (BIIIIG)

using namespace compv;

// preprocessor cannot evaluate an enum
// #if FORMAT == COMPV_PIXEL_FORMAT_R8G8B8A8 is always true
#define FORMAT_RGBA	3 // COMPV_PIXEL_FORMAT_R8G8B8A8
#define FORMAT_ARGB	6 // COMPV_PIXEL_FORMAT_A8R8G8B8
#define FORMAT_BGRA	4 // COMPV_PIXEL_FORMAT_B8G8R8A8
#define FORMAT_ABGR	5 // COMPV_PIXEL_FORMAT_A8B8G8R8
#define FORMAT_RGB	1 // COMPV_PIXEL_FORMAT_R8G8B8
#define FORMAT_BGR	2 // COMPV_PIXEL_FORMAT_B8G8R8

#define FORMAT			FORMAT_BGR
#define STRIDE_ALIGN	true // false to test CompVImage::wrap and CompVImage::copy

static void rgbToRGBA(const CompVObjWrapper<CompVImage *>& jpegImage, void** rgbaPtr, int &height, int &width, int &stride)
{
#define WIDTH_OFFSET -1 // amount of pixels to remove to width to make it wired (not standard)
#define HEIGHT_OFFSET 0 // amount of pixels to remove to height to make it wired (not standard)	
    int jpegImageStride = jpegImage->getStride();
    width = jpegImage->getWidth() + WIDTH_OFFSET;
    height = jpegImage->getHeight() + HEIGHT_OFFSET;
    if (STRIDE_ALIGN) {
        COMPV_CHECK_CODE_ASSERT(CompVImage::getBestStride(jpegImageStride, &stride));
    }
    else {
        stride = jpegImageStride + 2;
    }

#if FORMAT == FORMAT_RGBA || FORMAT == FORMAT_ARGB || FORMAT == FORMAT_BGRA || FORMAT == FORMAT_ABGR
    int compSize = 4;
#elif FORMAT == FORMAT_RGB || FORMAT == FORMAT_BGR
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
#if FORMAT == FORMAT_RGBA
            rgbaPtr_[0] = rgbPtr_[0];
            rgbaPtr_[1] = rgbPtr_[1];
            rgbaPtr_[2] = rgbPtr_[2];
            rgbaPtr_[3] = 0xFF;
#elif FORMAT == FORMAT_ARGB
            rgbaPtr_[0] = 0xFF;
            rgbaPtr_[1] = rgbPtr_[0];
            rgbaPtr_[2] = rgbPtr_[1];
            rgbaPtr_[3] = rgbPtr_[2];
#elif FORMAT == FORMAT_BGRA
            rgbaPtr_[0] = rgbPtr_[2];
            rgbaPtr_[1] = rgbPtr_[1];
            rgbaPtr_[2] = rgbPtr_[0];
            rgbaPtr_[3] = 0xFF;
#elif FORMAT == FORMAT_ABGR
            rgbaPtr_[0] = 0xFF;
            rgbaPtr_[1] = rgbPtr_[2];
            rgbaPtr_[2] = rgbPtr_[1];
            rgbaPtr_[3] = rgbPtr_[0];
#elif FORMAT == FORMAT_RGB
            rgbaPtr_[0] = rgbPtr_[0];
            rgbaPtr_[1] = rgbPtr_[1];
            rgbaPtr_[2] = rgbPtr_[2];
#elif FORMAT == FORMAT_BGR
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

    COMPV_ASSERT(FORMAT_RGBA == COMPV_PIXEL_FORMAT_R8G8B8A8);
    COMPV_ASSERT(FORMAT_ARGB == COMPV_PIXEL_FORMAT_A8R8G8B8);
    COMPV_ASSERT(FORMAT_BGRA == COMPV_PIXEL_FORMAT_B8G8R8A8);
    COMPV_ASSERT(FORMAT_ABGR == COMPV_PIXEL_FORMAT_A8B8G8R8);
    COMPV_ASSERT(FORMAT_RGB == COMPV_PIXEL_FORMAT_R8G8B8);
    COMPV_ASSERT(FORMAT_BGR == COMPV_PIXEL_FORMAT_B8G8R8);

    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_EQUIRECTANGULAR_FILE, &jpegImage));
    COMPV_ASSERT(jpegImage->getPixelFormat() == COMPV_PIXEL_FORMAT_R8G8B8);
    rgbToRGBA(jpegImage, &rgbaPtr, height, width, stride);

#define loopCount  1
    timeStart = CompVTime::getNowMills();
    for (size_t i = 0; i < loopCount; ++i) {
        COMPV_CHECK_CODE_ASSERT(CompVImage::wrap((COMPV_PIXEL_FORMAT)FORMAT, rgbaPtr, width, height, stride, &rgbaImage));
        COMPV_CHECK_CODE_ASSERT(rgbaImage->convert(COMPV_PIXEL_FORMAT_I420, &i420Image)); // RGBA -> I420
#if FORMAT == FORMAT_RGBA // only I420 -> RGBA is supported
        const uint8_t* yPtr = (const uint8_t*)i420Image->getDataPtr();
        const uint8_t* uPtr = yPtr + (i420Image->getHeight() * i420Image->getStride());
        const uint8_t* vPtr = uPtr + ((i420Image->getHeight() * i420Image->getStride()) >> 2);
        COMPV_CHECK_CODE_ASSERT(i420Image->convert((COMPV_PIXEL_FORMAT)FORMAT, &rgbaImage)); // // I420 -> RGBA
        COMPV_CHECK_CODE_ASSERT(rgbaImage->convert(COMPV_PIXEL_FORMAT_I420, &i420Image)); // // RGBA -> I420
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
