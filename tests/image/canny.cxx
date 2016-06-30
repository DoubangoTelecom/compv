#include <compv/compv_api.h>

#include "../common.h"

#define CANNY_JPEG_IMG		"C:/Projects/GitHub/compv/tests/Bikesgray.jpg" //"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
#define CANNY_LOOP_COUNT	1

using namespace compv;

COMPV_ERROR_CODE Canny()
{
	static const int16_t ScharrGx_vt[3] = { 3, 10, 3 };
	static const int16_t ScharrGx_hz[3] = { -1, 0, 1 };
	CompVPtr<CompVImage *> image;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	int16_t *gx = NULL, *gy = NULL;

	COMPV_CHECK_CODE_RETURN(CompVImageDecoder::decodeFile(CANNY_JPEG_IMG, &image));
	COMPV_CHECK_CODE_RETURN(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
	COMPV_CHECK_CODE_BAIL((err = CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)image->getDataPtr(), (size_t)image->getWidth(), (size_t)image->getStride(), (size_t)image->getHeight(), &ScharrGx_vt[0], &ScharrGx_hz[0], (size_t)3, gx)));
	COMPV_CHECK_CODE_BAIL((err = CompVMathConvlt::convlt1<uint8_t, int16_t, int16_t>((const uint8_t*)image->getDataPtr(), (size_t)image->getWidth(), (size_t)image->getStride(), (size_t)image->getHeight(), &ScharrGx_hz[0], &ScharrGx_vt[0], (size_t)3, gy)));

	uint16_t *g = (uint16_t *)CompVMem::malloc(image->getStride() * image->getHeight() * sizeof(uint16_t));
	COMPV_CHECK_EXP_BAIL(!g, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	int16_t *gx_ = gx, *gy_ = gy;
	uint16_t *g_ = g;
	uint16_t max = 1;

	// compute gradient and find max
	for (int j = 0; j < image->getHeight(); ++j) {
		for (int i = 0; i < image->getWidth(); ++i) {
			g_[i] = abs(gx_[i]) + abs(gy_[i]);
			if (max < g_[i]) {
				max = g_[i];
			}
		}
		g_ += image->getStride();
		gx_ += image->getStride();
		gy_ += image->getStride();
	}

	// scale (normalization)
	float scale = 255.f / float(max);
	uint16_t* g16_ = g;
	uint8_t* g8_ = (uint8_t*)g;
	for (int j = 0; j < image->getHeight(); ++j) {
		for (int i = 0; i < image->getWidth(); ++i) {
			g8_[i] = (uint8_t)COMPV_MATH_CLIP3(0, 255, (g16_[i] * scale));
		}
		g16_ += image->getStride();
		g8_ += image->getStride();
	}
	COMPV_CHECK_CODE_RETURN(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, (uint8_t*)g, image->getWidth(), image->getHeight(), image->getStride(), &image));
	writeImgToFile(image);

bail:
	CompVMem::free((void**)&gx);
	CompVMem::free((void**)&gy);
	CompVMem::free((void**)&g);
	return err;
}

COMPV_ERROR_CODE TestCanny()
{
	// Detect edges
	uint64_t timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < CANNY_LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(Canny());
	}
	uint64_t timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time (TestCanny) = [[[ %llu millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}