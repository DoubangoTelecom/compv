#include <compv/compv_api.h>

#define JPEG_IMG  "C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg" // OpenGL livre
//#define JPEG_IMG  "C:/Projects/GitHub/pan360/images/mandekalou.JPG" // Mande Griots
//#define JPEG_IMG	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

using namespace compv;

#define IMAGE_PYRAMID_FACTOR		.83f
#define IMAGE_PYRAMID_LEVELS		8
#define IMAGE_PYRAMID_SCALE_TYPE	COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_PYRAMID_LOOP_COUNT	1000
#define IMAGE_MD5					1

extern void writeImgToFile(const CompVObjWrapper<CompVImage *>& img, COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL);
extern std::string imageMD5(const CompVObjWrapper<CompVImage *>& img, COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL);

bool TestPyramid()
{
    CompVObjWrapper<CompVImage *> image;
    CompVObjWrapper<CompVImageScalePyramid*> pyramid;
    uint64_t timeStart, timeEnd;

    // Decode the jpeg image
    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    // Convert image to GrayScale
    COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
    // Create the pyramid
    COMPV_CHECK_CODE_ASSERT(CompVImageScalePyramid::newObj(IMAGE_PYRAMID_FACTOR, IMAGE_PYRAMID_LEVELS, IMAGE_PYRAMID_SCALE_TYPE, &pyramid));
    // process
    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < IMAGE_PYRAMID_LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_ASSERT(pyramid->process(image));
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

#if IMAGE_MD5
    for (int i = 0; i < pyramid->getLevels(); ++i) {
        COMPV_CHECK_CODE_ASSERT(pyramid->getImage(i, &image));
        COMPV_DEBUG_INFO("MD5(level %d, s=%d,w=%d,h=%d)=%s", i, image->getStride(), image->getWidth(), image->getHeight(), imageMD5(image).c_str());
    }
#endif

    // dump latest image to file
    COMPV_CHECK_CODE_ASSERT(pyramid->getImage(pyramid->getLevels() - 1, &image));
    writeImgToFile(image);

    return true;
}