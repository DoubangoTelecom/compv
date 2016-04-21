#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG								"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg"
#define GAUSS_SIGMA2_SIZE7_DIM2_IMG_MD5			"bc3bab9f6e14a29aa42a1614a845ce8f" // MD5 value after gaussian filter with sigma=2 and kernel size = 7, Dim = 2
#define GAUSS_SIGMA2_SIZE7_DIM1_IMG_MD5			"54ebaf15df9b9a6693b6375d71f2f9d4" // MD5 value after gaussian filter with sigma=2 and kernel size = 7, Dim = 1
#define GAUSS_SIGMA2_SIZE7_KERNEL_DIM2_MD5		"b450cff5c1540ca2602f0c21c245d50e" // MD5 value for the generated kernel with dim=2
#define GAUSS_SIGMA2_SIZE7_KERNEL_DIM1_MD5		"5b538cf89aace2657d8330f38859f20f" // MD5 value for the generated kernel with dim=1

#define GAUSS_LOOP_COUNT				1

// Sigma=2, Size=7, Expected kernel:
// keep this matrix here for weak comparison in case of rounding issues
static const double kGaussianKernelDim2Sigma2Size7[7][7] = {
	{ 0.00492233, 0.00919613, 0.01338028, 0.01516185, 0.01338028, 0.00919613, 0.00492233 },
	{ 0.00919613, 0.01718062, 0.02499766, 0.02832606, 0.02499766, 0.01718062, 0.00919613 },
	{ 0.01338028, 0.02499766, 0.03637138, 0.04121417, 0.03637138, 0.02499766, 0.01338028 },
	{ 0.01516185, 0.02832606, 0.04121417, 0.04670178, 0.04121417, 0.02832606, 0.01516185 },
	{ 0.01338028, 0.02499766, 0.03637138, 0.04121417, 0.03637138, 0.02499766, 0.01338028 },
	{ 0.00919613, 0.01718062, 0.02499766, 0.02832606, 0.02499766, 0.01718062, 0.00919613 },
	{ 0.00492233, 0.00919613, 0.01338028, 0.01516185, 0.01338028, 0.00919613, 0.00492233 }
};
static const double kGaussianKernelDim1Sigma2Size7[7] = { 0.07015933, 0.13107488, 0.19071282, 0.21610594, 0.19071282, 0.13107488, 0.07015933 };

static COMPV_ERROR_CODE convlt2(uint8_t* img, int imgw, int imgs, int imgh, const double* ker, int ker_size, int img_border = 0)
{
	COMPV_CHECK_EXP_RETURN(!(ker_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Kernel size must be odd number

	uint8_t* outImg = (uint8_t*)CompVMem::malloc((imgh + (img_border << 1)) * (imgs + (img_border << 1)));
	const uint8_t *topleft, *img_ptr;
	double sum;
	const double *ker_ptr;
	int imgpad, i, j, row, col;
	int ker_size_div2 = ker_size >> 1;
	int start_idx = (img_border > ker_size_div2) ? ker_size_div2 : (ker_size_div2 - img_border);
	img_ptr = img;
	imgpad = (imgs - imgw) + ker_size_div2 + ker_size_div2;

	for (j = start_idx; j < imgh - start_idx; ++j) {
		for (i = start_idx; i < imgw - start_idx; ++i) {
			sum = 0;
			topleft = img_ptr;
			ker_ptr = ker;
			for (row = 0; row < ker_size; ++row) {
				for (col = 0; col < ker_size; ++col) {
					sum += topleft[col] * ker_ptr[col];
				}
				ker_ptr += ker_size;
				topleft += imgs;
			}
			outImg[(j * imgs) + i] = (uint8_t)sum;
			++img_ptr;
		}
		img_ptr += imgpad;
	}
	CompVMem::copy(img, outImg, imgh * imgs); // FIXME: garbage
	CompVMem::free((void**)&outImg);

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE convlt1(uint8_t* img, int imgw, int imgs, int imgh, const double* vker, const double* hker, int ker_size, int img_border = 0)
{
	COMPV_CHECK_EXP_RETURN(!(ker_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Kernel size must be odd number

	uint8_t *imgTmp;
	const uint8_t *topleft, *ptr_;
	double sum;
	int imgpad, i, j, row, col;
	int ker_size_div2 = ker_size >> 1;
	int start_idx = (img_border > ker_size_div2) ? ker_size_div2 : (ker_size_div2 - img_border);

	imgTmp = (uint8_t*)CompVMem::malloc((imgh + (img_border << 1)) * (imgs + (img_border << 1)));
	COMPV_CHECK_EXP_RETURN(!imgTmp, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	//CompVMem::copy(outImg, img, imgh * imgs);
	
	(row); (topleft); (col);

	// FIXME
	memset(imgTmp, 0, (imgh * imgs));

	// Horizontal
	topleft = img;
	imgpad = (imgs - imgw) + start_idx + start_idx;
	for (j = 0; j < imgh; ++j) {
		for (i = start_idx; i < imgw - start_idx; ++i) {
			sum = 0;
			for (col = 0; col < ker_size; ++col) {
				sum += topleft[col] * hker[col];
			}
			imgTmp[(j * imgs) + i] = (uint8_t)sum;
			++topleft;
		}
		topleft += imgpad;
	}

	// Vertical
	topleft = imgTmp; // output from hz filtering is now used as input
	imgpad = (imgs - imgw);
	for (j = start_idx; j < imgh - start_idx; ++j) {
		for (i = 0; i < imgw; ++i) {
			sum = 0;
			ptr_ = topleft;
			for (row = 0; row < ker_size; ++row, ptr_ += imgs) {
				sum += *ptr_ * vker[row];
			}
			img[(j * imgs) + i] = (uint8_t)sum;
			++topleft;
		}
		topleft += imgpad;
	}

	CompVMem::free((void**)&imgTmp);

	return COMPV_ERROR_CODE_S_OK;
}

bool TestGaussFilter()
{
	CompVObjWrapper<CompVImage *> image;
	CompVObjWrapper<CompVConvlt *> convlt;
	uint64_t timeStart, timeEnd;

	// Create convolution context
	COMPV_CHECK_CODE_ASSERT(CompVConvlt::newObj(&convlt));
	// Decode the jpeg image
	COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	// Convert image to GrayScale
	COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
	// Scale the image
	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < GAUSS_LOOP_COUNT; ++i) {
		//convlt2((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), (const double*)kGaussianKernelDim2Sigma2Size7, 7);
		convlt1((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), (const double*)kGaussianKernelDim1Sigma2Size7, (const double*)kGaussianKernelDim1Sigma2Size7, 7);
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

	if (imageMD5(image) != GAUSS_SIGMA2_SIZE7_DIM1_IMG_MD5) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		COMPV_ASSERT(false);
		return false;
	}

	// dump image to file
	writeImgToFile(image);

	return true;
}

bool TestGaussKernDim1Gen()
{
	CompVObjWrapper<CompVArray<double>* > kern1;
	COMPV_ERROR_CODE err_ = CompVGaussKern::buildKern1(&kern1, 7, 2.0);
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		COMPV_ASSERT(false);
		return false;
	}
	const double* ken1_ptr = kern1->getDataPtr();
	const std::string expectedMD5 = CompVMd5::compute2((const void*)kern1->getDataPtr(), kern1->getDataSizeInBytes());
	if (expectedMD5 != GAUSS_SIGMA2_SIZE7_KERNEL_DIM1_MD5) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		COMPV_ASSERT(false);
		return false;
	}

#if 0 // Print generated kernel
	printf("Gaussian kernel Dim1={\n");
	for (int x = 0; x < 7; ++x) {
		printf("%.8f, ", ken1_ptr[x]);
	}
	printf("}\n");
#endif

	COMPV_DEBUG_INFO("TestGaussKernDim1Gen() done!");

	return true;
}

bool TestGaussKernDim2Gen()
{
#define kernelAt(_y_, _x_) *(kern2_ + ((_y_) * 7) + (_x_))
	CompVObjWrapper<CompVArray<double>* > kern2;
	COMPV_ERROR_CODE err_ = CompVGaussKern::buildKern2(&kern2, 7, 2.0);
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		COMPV_ASSERT(false);
		return false;
	}
	const std::string expectedMD5 = CompVMd5::compute2((const void*)kern2->getDataPtr(), kern2->getDataSizeInBytes());
	if (expectedMD5 != GAUSS_SIGMA2_SIZE7_KERNEL_DIM2_MD5) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		COMPV_ASSERT(false);
		return false;
	}

#if 0 // Print generated kernel
	const double* kern2_ = kern2->getDataPtr();
	printf("Gaussian kernel Dim2={\n");
	for (int y = 0; y < 7; ++y) {
		printf("{ ");
		for (int x = 0; x < 7; ++x) {
			printf("%.8f, ", kernelAt(y, x));
		}
		printf("}\n");
	}
	printf("}");
#endif

#undef kernelAt

	COMPV_DEBUG_INFO("TestGaussKernDim2Gen() done!");

	return true;
}
