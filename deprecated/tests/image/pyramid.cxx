#include <compv/compv_api.h>

#include "../common.h"

#define JPEG_IMG	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

using namespace compv;

#define FILE_NAME_EQUIRECTANGULAR	"C:/Projects/GitHub/data/test_images/equirectangular_1282x720_gray.yuv" // "C:/Projects/compv/tests/7019363969_a80a5d6acc_o.jpg" // "/Users/mamadou/Documents/compv/tests/7019363969_a80a5d6acc_o.jpg" //  // voiture
#define FILE_NAME_OPENGLBOOK		"C:/Projects/GitHub/data/test_images/opengl_programming_guide_8th_edition_200x258_gray.yuv" // OpenGL book
#define FILE_NAME_GRIOTS			"C:/Projects/GitHub/data/test_images/mandekalou_480x640_gray.yuv" // Mande Griots

static const struct compv_unittest_pyramid {
	COMPV_SCALE_TYPE scaleType;
	float factor; // should be < 1
	size_t levels;
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_PYRAMID[] =
{
	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, 6, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "ea54d2b04018d355792add02535eae23" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, 8, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "c10b3b41a97e6416058a55cf71463409" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.99f, 3, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "a9f2631d847e92fa45638523fd508cb9" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.47f, 7, FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "6452b6e6bb410d9457560a8bb756dd3e" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, 6, FILE_NAME_OPENGLBOOK, 200, 258, 200, "b4a941a27b7d07a4ca9f3ef4e5bca69b" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, 8, FILE_NAME_OPENGLBOOK, 200, 258, 200, "70901e3e2c9107eda3b4403382f4b564" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.99f, 3, FILE_NAME_OPENGLBOOK, 200, 258, 200, "64e35be23429de29cd3b5ce2ea66c6cf" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.47f, 7, FILE_NAME_OPENGLBOOK, 200, 258, 200, "94eb334401cd781bdc079c398ef4c938" },

	{ COMPV_SCALE_TYPE_BILINEAR, 0.5f, 6, FILE_NAME_GRIOTS, 480, 640, 480, "66e2b3eb67b82b8e2bf8760ca6171dc5" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.83f, 8, FILE_NAME_GRIOTS, 480, 640, 480, "47178356656332c41bd1cc30ac04fcf4" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.99f, 3, FILE_NAME_GRIOTS, 480, 640, 480, "2db1f8f8fe5cd8b73eddf059126f364e" },
	{ COMPV_SCALE_TYPE_BILINEAR, 0.47f, 7, FILE_NAME_GRIOTS, 480, 640, 480, "e11f97f76b1ede2bf883ddddd05549c1" },
};
static const size_t COMPV_UNITTEST_PYRAMID_COUNT = sizeof(COMPV_UNITTEST_PYRAMID) / sizeof(COMPV_UNITTEST_PYRAMID[0]);

#define IMAGE_PYRAMID_FACTOR		.83f
#define IMAGE_PYRAMID_LEVELS		8
#define IMAGE_PYRAMID_SCALE_TYPE	COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_PYRAMID_FILENAME		FILE_NAME_EQUIRECTANGULAR

#define IMAGE_PYRAMID_LOOP_COUNT	1

static std::string pyramidMD5(const CompVPtr<CompVImageScalePyramid*>& pyramid)
{
	CompVPtr<CompVImage *> image;
	std::string md5_level;
	CompVPtr<CompVMd5*> md5;
	COMPV_CHECK_CODE_ASSERT(CompVMd5::newObj(&md5));
	for (int i = 0; i < pyramid->getLevels(); ++i) {
		COMPV_CHECK_CODE_ASSERT(pyramid->getImage(i, &image));
		md5_level = imageMD5(image);
		COMPV_CHECK_CODE_ASSERT(md5->update(reinterpret_cast<const uint8_t*>(md5_level.c_str()), md5_level.length()));
	}
	return md5->compute();
}

COMPV_ERROR_CODE TestPyramid()
{
    CompVPtr<CompVImage *> image;
    CompVPtr<CompVImageScalePyramid*> pyramid;
    uint64_t timeStart, timeEnd;
	const compv_unittest_pyramid* test = NULL;
	CompVPtr<CompVBuffer *> buffer;

	for (size_t i = 0; i < COMPV_UNITTEST_PYRAMID_COUNT; ++i) {
		if (
			COMPV_UNITTEST_PYRAMID[i].factor == IMAGE_PYRAMID_FACTOR
			&& COMPV_UNITTEST_PYRAMID[i].levels == IMAGE_PYRAMID_LEVELS
			&& COMPV_UNITTEST_PYRAMID[i].scaleType == IMAGE_PYRAMID_SCALE_TYPE
			&& std::string(COMPV_UNITTEST_PYRAMID[i].filename).compare(IMAGE_PYRAMID_FILENAME) == 0
			)
		{
			test = &COMPV_UNITTEST_PYRAMID[i];
			break;
		}
	}

	if (!test) {
		COMPV_DEBUG_ERROR_EX("TAG_TEST", "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	// Read image
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(test->filename, &buffer));
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, buffer->getPtr(), (int32_t)test->width, (int32_t)test->height, (int32_t)test->width, &image));
    // Create the pyramid
	COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(test->factor, (int32_t)test->levels, test->scaleType, &pyramid));
    // process
    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < IMAGE_PYRAMID_LOOP_COUNT; ++i) {
        COMPV_CHECK_CODE_RETURN(pyramid->process(image));
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time(TestPyramid) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    // Check MD5 values
	COMPV_CHECK_EXP_RETURN(pyramidMD5(pyramid).compare(test->md5) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED);
	
    // dump latest image to file
    COMPV_CHECK_CODE_RETURN(pyramid->getImage(pyramid->getLevels() - 1, &image));
    writeImgToFile(image);

    return COMPV_ERROR_CODE_S_OK;
}