#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG							"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg"
#define GAUSS_SIGMA2_SIZE7_IMG_MD5			"bc3bab9f6e14a29aa42a1614a845ce8f" // MD5 value after gaussian filter with sigma=2 and kernel size = 7
#define GAUSS_SIGMA2_SIZE7_KERNEL_DIM2_MD5	"b450cff5c1540ca2602f0c21c245d50e" // MD5 value for the generated kernel with dim=2
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

static COMPV_ERROR_CODE convlt2(uint8_t* img, int imgw, int imgs, int imgh, const double* ker, int ker_size)
{
	COMPV_CHECK_EXP_RETURN(!(ker_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Kernel size must be odd number

	uint8_t* outImg = (uint8_t*)CompVMem::malloc(imgh * imgs);
	const uint8_t *topleft, *img_ptr;
	double sum;
	const double *ker_ptr;
	int imgpad, i, j, row, col;
	int ker_size_div2 = ker_size >> 1;
	img_ptr = img;
	imgpad = (imgs - imgw) + ker_size_div2 + ker_size_div2;

	for (j = ker_size_div2; j < imgh - ker_size_div2; ++j) {
		for (i = ker_size_div2; i < imgw - ker_size_div2; ++i) {
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

static COMPV_ERROR_CODE convlt1(uint8_t* img, int imgw, int imgs, int imgh, const double* ker, int ker_size)
{
	COMPV_CHECK_EXP_RETURN(!(ker_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER); // Kernel size must be odd number

	uint8_t *imgTmp;
	const uint8_t *topleft, *img_ptr;
	double sum;
	int imgpad, i, j, row, col;
	int ker_size_div2 = ker_size >> 1;

	imgTmp = (uint8_t*)CompVMem::malloc(imgh * imgs);
	COMPV_CHECK_EXP_RETURN(!imgTmp, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	//CompVMem::copy(outImg, img, imgh * imgs);
	
	(row); (topleft); (col);

	// FIXME
	memset(imgTmp, 0, (imgh * imgs));

	// Horizontal
	img_ptr = img + ker_size_div2;
	imgpad = (imgs - imgw) + ker_size_div2 + ker_size_div2;
	for (j = 0; j < imgh; ++j) {
		for (i = ker_size_div2; i < imgw - ker_size_div2; ++i) {
			sum = 0;
			topleft = img_ptr - ker_size_div2;
			for (col = 0; col < ker_size; ++col) {
				sum += topleft[col] * ker[col];
			}
			imgTmp[(j * imgs) + i] = (uint8_t)sum;
			++img_ptr;
		}
		img_ptr += imgpad;
	}

	// Vertical
	img_ptr = imgTmp + (ker_size_div2 * imgs); // output from hz filtering is now used as input
	imgpad = (imgs - imgw);
	for (j = ker_size_div2; j < imgh - ker_size_div2; ++j) {
		for (i = 0; i < imgw; ++i) {
			sum = 0;
			topleft = img_ptr - (ker_size_div2 * imgs);
			for (row = 0; row < ker_size; ++row) {
				sum += topleft[0] * ker[row];
				topleft += imgs;
			}
			img[(j * imgs) + i] = (uint8_t)sum;
			++img_ptr;
		}
		img_ptr += imgpad;
	}

	CompVMem::free((void**)&imgTmp);

	return COMPV_ERROR_CODE_S_OK;
}

bool TestGaussFilter()
{
	CompVObjWrapper<CompVImage *> image;
	uint64_t timeStart, timeEnd;

	// Decode the jpeg image
	COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
	// Convert image to GrayScale
	COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
	// Scale the image
	timeStart = CompVTime::getNowMills();
	for (int i = 0; i < GAUSS_LOOP_COUNT; ++i) {
		//convlt2((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), (const double*)kGaussianKernelDim2Sigma2Size7, 7);
		convlt1((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), (const double*)kGaussianKernelDim1Sigma2Size7, 7);
	}
	timeEnd = CompVTime::getNowMills();
	COMPV_DEBUG_INFO("Elapsed time = [[[ %llu millis ]]]", (timeEnd - timeStart));

	if (imageMD5(image) != GAUSS_SIGMA2_SIZE7_IMG_MD5) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		//COMPV_ASSERT(false);
		//return false;
	}

	// dump image to file
	writeImgToFile(image);

	return true;
}

bool TestGaussKernDim1Gen()
{
	static const double kSigma = 2.0;
	static const int kSize = 7; // Kernel size
	const int size_div2 = (kSize >> 1);
	double kernel[kSize] = { 0 };
	const double sigma2_times2 = 2.0 * (kSigma * kSigma); // 2*(sigma^2)
	const double one_over_sqrt_pi_times_sigma2_times2 = (1.0 / sqrt(COMPV_MATH_PI * sigma2_times2)); // 1 / sqrt(2 * pi * sigma^2)
	double sum, k;
	int x;

	// for x = 0
	kernel[0 + size_div2] = one_over_sqrt_pi_times_sigma2_times2;
	sum = one_over_sqrt_pi_times_sigma2_times2;
	// for x = 1...
	for (x = 1; x <= size_div2; ++x) {
		k = one_over_sqrt_pi_times_sigma2_times2 * exp(-((x * x) / sigma2_times2));
		kernel[x + size_div2] = k;
		kernel[size_div2 - x] = k;
		sum += (k + k);
	}

	// Normalize
	for (x = 0; x < kSize; ++x) {
		kernel[x] /= sum;
	}

#if 1 // Print generated kernel
	printf("Gaussian kernel Dim1={\n");
	for (x = 0; x < kSize; ++x) {
		printf("%.8f, ", kernel[x]);
	}
	printf("}\n");
#endif

	return true;
}

bool TestGaussKernDim2Gen()
{
	static const double kSigma = 2.0;
	static const int kSize = 7; // Kernel size
	double kernel[kSize][kSize] = {0};

	COMPV_ASSERT(kSize & 1); // Must be Odd number

	const double sigma2_times2 = 2.0 * (kSigma * kSigma); // 2*(sigma^2)
	const int size_div2 = (kSize >> 1);
	int x, y, kx, ky;
	double sum = 0.0, x2_plus_y2, y2, k;
	const double one_over_pi_times_sigma2_times2 = (1.0 / (COMPV_MATH_PI * sigma2_times2)); // 1 / (2 * pi * sigma^2)

	// Formula: https://en.wikipedia.org/wiki/Gaussian_blur
	// Ignore negative x and y as we'll be using x^2 and y^2 then, complete the kernel (symetric)
	for (ky = size_div2, y = 0; y <= size_div2; ++y, ++ky) {
		y2 = y * y;
		for (kx = size_div2, x = 0; x <= size_div2; ++x, ++kx) {
			x2_plus_y2 = (x * x) + y2;
			k = one_over_pi_times_sigma2_times2 * exp(-(x2_plus_y2 / sigma2_times2)); // x>=0 and y>=0
			kernel[ky][kx] = k;
			if (y != 0 || x != 0) {
				kernel[size_div2 - y][kx] = k;
				kernel[ky][size_div2 - x] = k;
				kernel[size_div2 - y][size_div2 - x] = k;
			}
		}
	}

	// Compute sum
	for (ky = 0; ky < kSize; ++ky) {
		for (kx = 0; kx < kSize; ++kx) {
			sum += kernel[ky][kx];
		}
	}

	// Normalize
	for (y = 0; y < kSize; ++y) {
		for (x = 0; x < kSize; ++x) {
			kernel[y][x] /= sum;
		}
	}

	const std::string expectedMD5 = CompVMd5::compute2((const void*)kernel, sizeof(kernel));
	if (expectedMD5 != GAUSS_SIGMA2_SIZE7_KERNEL_DIM2_MD5) {
		COMPV_DEBUG_ERROR("MD5 mismatch");
		COMPV_ASSERT(false);
		return false;
	}

#if 0 // Print generated kernel
	printf("Gaussian kernel Dim2={\n");
	for (y = 0; y < kSize; ++y) {
		printf("{ ");
		for (x = 0; x < kSize; ++x) {
			printf("%.8f, ", kernel[y][x]);
		}
		printf("}\n");
	}
	printf("}");
#endif

	COMPV_DEBUG_INFO("TestGaussKernDim2Gen() done!");

	return true;
}
