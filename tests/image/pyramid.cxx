#include <compv/compv_api.h>

#include "../common.h"

#define JPEG_IMG	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

using namespace compv;

#define IMAGE_PYRAMID_FACTOR		.83f
#define IMAGE_PYRAMID_LEVELS		8
#define IMAGE_PYRAMID_SCALE_TYPE	COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_PYRAMID_LOOP_COUNT	1000

static const std::string expectedMD5[IMAGE_PYRAMID_LEVELS] = 
{
	"a6457ae363e42d975b6c6017214c9e14",
	"9777b441875d4f75c24fae8c1816c50d",
	"88b4a2ff413bf8ddbb3a5c52a4bdba80",
	"7140acde40f8fc441867238bdb229ddb",
	"8984022417bc535f8d05ae4a67d432b3",
	"3ec2f638379ff1ae1694a8b7ba6c334f",
	"c4a06882d19e8b3cfb640cf7b938411c",
	"9c0896cd5e7787f4374b0fddd61fe5dd",
};

bool TestPyramid()
{
    CompVPtr<CompVImage *> image;
    CompVPtr<CompVImageScalePyramid*> pyramid;
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
    COMPV_DEBUG_INFO("Elapsed time(TestPyramid) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	// Check MD5 values
    for (int i = 0; i < pyramid->getLevels(); ++i) {
        COMPV_CHECK_CODE_ASSERT(pyramid->getImage(i, &image));
		//COMPV_DEBUG_INFO("%s", imageMD5(image).c_str());
		if (imageMD5(image) != expectedMD5[i]) {
			COMPV_DEBUG_ERROR("MD5 mismatch");
			COMPV_ASSERT(false);
			return false;
		}
    }

    // dump latest image to file
    COMPV_CHECK_CODE_ASSERT(pyramid->getImage(pyramid->getLevels() - 1, &image));
    writeImgToFile(image);

    return true;
}