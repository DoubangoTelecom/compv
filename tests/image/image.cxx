#include <compv/compv_api.h>
#include <tchar.h>

#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // FIXME: voiture

using namespace compv;

static void rgbToRGBA(const CompVObjWrapper<CompVImage *>& jpegImage, void** rgbaPtr, size_t &width, size_t &height, size_t &stride)
{
#define WIDTH_OFFSET -1 // amount of pixels to remove to width to make it wired (not standard)
#define HEIGHT_OFFSET 0 // amount of pixels to remove to height to make it wired (not standard)
	int32_t strideBestAlignment = (COMPV_SIMD_ALIGNV_DEFAULT << 2); // Most of the time we will process the pixels per pack of #4 this is why the stride need to be aligned to AVX*4
	size_t jpegImageStride = jpegImage->getStride();
	width = jpegImage->getWidth() + WIDTH_OFFSET;
	height = jpegImage->getHeight() + HEIGHT_OFFSET;
	stride = CompVMem::alignForward(jpegImageStride, strideBestAlignment);

	*rgbaPtr = CompVMem::malloc((stride * jpegImage->getHeight()) * 4);
	COMPV_ASSERT(*rgbaPtr != NULL);
	const uint8_t *rgbPtr_ = (const uint8_t *)jpegImage->getDataPtr();
	uint8_t* rgbaPtr_ = (uint8_t*)*rgbaPtr;
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			rgbaPtr_[0] = rgbPtr_[0];
			rgbaPtr_[1] = rgbPtr_[1];
			rgbaPtr_[2] = rgbPtr_[2];
			rgbaPtr_[3] = 0xFF;

			rgbaPtr_ += 4;
			rgbPtr_ += 3;
		}
		rgbaPtr_ += (stride - width) * 4;
		rgbPtr_ += (jpegImageStride - width) * 3;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	CompVObjWrapper<CompVImage *> jpegImage;
	CompVObjWrapper<CompVImage *> i420Image;
	CompVObjWrapper<CompVThreadDispatcher *> threadDisp;
	void* rgbaPtr = NULL;
	size_t width, height, stride;
	FILE* file = NULL;
	const size_t loopCount = 1;
	uint64_t timeStart, timeEnd;

	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(kCpuFlagNone));
	COMPV_CHECK_CODE_ASSERT(CompVThreadDispatcher::newObj(&threadDisp));
	COMPV_CHECK_CODE_ASSERT(CompVImageConv::multiThreadingEnable(threadDisp)); // enable multithreading for image convertion
	COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_EQUIRECTANGULAR_FILE, &jpegImage));
	COMPV_ASSERT(jpegImage->getPixelFormat() == COMPV_PIXEL_FORMAT_R8G8B8);
	rgbToRGBA(jpegImage, &rgbaPtr, width, height, stride);

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < loopCount; ++i) {
		COMPV_CHECK_CODE_ASSERT(CompVImageConv::rgbaToI420((const uint8_t*)rgbaPtr, width, height, stride, &i420Image));
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time =%llu", (timeEnd - timeStart));

	file = fopen("./i420.yuv", "wb+");
	COMPV_ASSERT(file != NULL);
	fwrite(i420Image->getDataPtr(), 1, i420Image->getDataSize(), file);

	CompVMem::free(&rgbaPtr);
	fclose(file);

	getchar();
	return 0;
}

