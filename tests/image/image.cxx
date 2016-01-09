#include <compv/compv_api.h>
#include <tchar.h>

#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // voiture
//#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/3867257549_6ca855e08d_o.jpg" //paris (BIIIIG)

using namespace compv;

static void rgbToRGBA(const CompVObjWrapper<CompVImage *>& jpegImage, void** rgbaPtr, int &height, int &width, int &stride)
{
#define WIDTH_OFFSET -1 // amount of pixels to remove to width to make it wired (not standard)
#define HEIGHT_OFFSET 0 // amount of pixels to remove to height to make it wired (not standard)	
	int jpegImageStride = jpegImage->getStride();
	width = jpegImage->getWidth() + WIDTH_OFFSET;
	height = jpegImage->getHeight() + HEIGHT_OFFSET;
	COMPV_CHECK_CODE_ASSERT(CompVImage::getBestStride(jpegImageStride, &stride));

	*rgbaPtr = CompVMem::malloc((stride * jpegImage->getHeight()) * 4);
	COMPV_ASSERT(*rgbaPtr != NULL);
	const uint8_t *rgbPtr_ = (const uint8_t *)jpegImage->getDataPtr();
	uint8_t* rgbaPtr_ = (uint8_t*)*rgbaPtr;
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
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
	CompVObjWrapper<CompVImage *> rgbaImage;
	void* rgbaPtr = NULL;
	int width, height, stride;
	uint64_t timeStart, timeEnd;
#define loopCount  1
#define numThreads COMPV_NUM_THREADS_SINGLE

	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	COMPV_CHECK_CODE_ASSERT(CompVEngine::init(numThreads));
	COMPV_CHECK_CODE_ASSERT(CompVCpu::flagsDisable(kCpuFlagNone));
	COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_EQUIRECTANGULAR_FILE, &jpegImage));
	COMPV_ASSERT(jpegImage->getPixelFormat() == COMPV_PIXEL_FORMAT_R8G8B8);
	rgbToRGBA(jpegImage, &rgbaPtr, height, width, stride);

	timeStart = CompVTime::getNowMills();
	for (size_t i = 0; i < loopCount; ++i) {
		COMPV_CHECK_CODE_ASSERT(CompVImageConv::rgbaToI420((const uint8_t*)rgbaPtr, height, width, stride, &i420Image));
#if 1 // I420 to RGBA then RGBA to I420
		const uint8_t* yPtr = (const uint8_t*)i420Image->getDataPtr();
		const uint8_t* uPtr = yPtr + (i420Image->getHeight() * i420Image->getStride());
		const uint8_t* vPtr = uPtr + ((i420Image->getHeight() * i420Image->getStride()) >> 2);
		COMPV_CHECK_CODE_ASSERT(CompVImageConv::i420ToRGBA(yPtr, uPtr, vPtr, i420Image->getHeight(), i420Image->getWidth(), i420Image->getStride(), &rgbaImage));
		COMPV_CHECK_CODE_ASSERT(CompVImageConv::rgbaToI420((const uint8_t*)rgbaImage->getDataPtr(), rgbaImage->getHeight(), rgbaImage->getWidth(), rgbaImage->getStride(), &i420Image));
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
	

	getchar();
	return 0;
}

