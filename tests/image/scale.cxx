#include <compv/compv_api.h>

#define JPEG_IMG  "C:/Projects/GitHub/pan360/images/opengl_programming_guide_8th_edition.jpg" // OpenGL livre
//#define JPEG_IMG	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture

using namespace compv;

// FIXME: use IMAGE_SCALE_WIDTH and IMAGE_SCALE_HEIGHT in this sample and use factor in pyramid
#define IMAGE_SCALE_FACTOR		.5f //0.83f
#define IMAGE_SCALE_TYPE		COMPV_SCALE_TYPE_BILINEAR
#define IMAGE_SACLE_LOOP_COUNT	1
#define IMAGE_MD5				1

extern void writeImgToFile(const CompVObjWrapper<CompVImage *>& img);

bool TestScale()
{
	CompVObjWrapper<CompVImage *> image;
	uint64_t timeStart, timeEnd;

	// Decode the jpeg image
	COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	int32_t outWidth = (int32_t)(image->getWidth() * IMAGE_SCALE_FACTOR);
	int32_t outHeight = (int32_t)(image->getHeight() * IMAGE_SCALE_FACTOR);
	// Convert image to GrayScale
	COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
	// Scale the image
	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < IMAGE_SACLE_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_ASSERT(image->scale(IMAGE_SCALE_TYPE, outWidth, outHeight, &image));
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO("Image scaling: factor=%f, outWidth=%d, outStride=%d, outHeight=%d", IMAGE_SCALE_FACTOR, image->getWidth(), image->getStride(), image->getHeight());

#if IMAGE_MD5
	COMPV_DEBUG_INFO("MD5(SCALED)=%s", CompVMd5::compute2(image->getDataPtr(), image->getDataSize()).c_str());
#endif

	// dump image to file
	writeImgToFile(image);

    return true;
}