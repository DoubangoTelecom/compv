#include <compv/compv_api.h>
#include <tchar.h>

#define JPEG_EQUIRECTANGULAR_FILE	"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg" // FIXME: voiture

using namespace compv;

static void rgbToRGBA(const CompVObjWrapper<CompVImage *>& jpegImage, void** rgbaPtr)
{
	size_t width = jpegImage->getWidth();
	size_t height = jpegImage->getHeight();
	size_t stride = jpegImage->getStride();

	*rgbaPtr = CompVMem::malloc((jpegImage->getStride() * jpegImage->getHeight()) * 4);
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
		rgbPtr_ += (stride - width) * 3;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	CompVObjWrapper<CompVImage *> jpegImage;
	CompVObjWrapper<CompVImage *> i420Image;
	void* rgbaPtr = NULL;
	FILE* file = NULL;
	COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_EQUIRECTANGULAR_FILE, &jpegImage));
	COMPV_ASSERT(jpegImage->getPixelFormat() == COMPV_PIXEL_FORMAT_R8G8B8);
	rgbToRGBA(jpegImage, &rgbaPtr);
	COMPV_CHECK_CODE_ASSERT(CompVImageConv::rgbaToI420((const uint8_t*)rgbaPtr, jpegImage->getWidth(), jpegImage->getHeight(), jpegImage->getStride(), &i420Image));

	file = fopen("./i420.yuv", "wb+");
	COMPV_ASSERT(file != NULL);
	fwrite(i420Image->getDataPtr(), 1, i420Image->getDataSize(), file);

	CompVMem::free(&rgbaPtr);
	fclose(file);
	return 0;
}

