#if 0
#include <compv/compv_api.h>

#include "../common.h"

using namespace compv;

#define JPEG_IMG								"C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg"
#define GAUSS_SIGMA2_SIZE7_FILTER_DIM2_MD5		"a8f02ac8594857aeb77b12deb6d60bb0" // MD5 value after gaussian filter with sigma=2 and kernel size = 7, Dim = 2
#define GAUSS_SIGMA2_SIZE7_FILTER_DIM1_MD5		"53b29bfe0a1c95ac091668e9f5b87958" // MD5 value after gaussian filter with sigma=2 and kernel size = 7, Dim = 1
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

bool TestGaussFilter2()
{
    CompVPtr<CompVImage *> image;
    CompVPtr<CompVConvlt *> convlt;
    CompVPtr<CompVArray<double>* > kern2;
    uint64_t timeStart, timeEnd;

    // Create kernel
    COMPV_CHECK_CODE_ASSERT(CompVGaussKern::buildKern2(&kern2, 7, 2.0));
    // Create convolution context
    COMPV_CHECK_CODE_ASSERT(CompVConvlt::newObj(&convlt));
    // Decode the jpeg image
    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    // Convert image to GrayScale
    COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
    // process the image
    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < GAUSS_LOOP_COUNT; ++i) {
        convlt->convlt2((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern2->getDataPtr(), 7);
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time(TestGaussFilter2) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    const std::string expectedMD5 = CompVMd5::compute2(convlt->getResultPtr(), convlt->getResultSize());
    if (expectedMD5 != GAUSS_SIGMA2_SIZE7_FILTER_DIM2_MD5) {
        COMPV_DEBUG_ERROR("MD5 mismatch");
        COMPV_ASSERT(false);
        return false;
    }

    // dump image to file
    COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, convlt->getResultPtr(), image->getWidth(), image->getHeight(), image->getStride(), &image));
    writeImgToFile(image);

    return true;
}

bool TestGaussFilter1()
{
    CompVPtr<CompVImage *> image;
    CompVPtr<CompVConvlt *> convlt;
    CompVPtr<CompVArray<double>* > kern1;
    uint64_t timeStart, timeEnd;

    // Create kernel
    COMPV_CHECK_CODE_ASSERT(CompVGaussKern::buildKern1(&kern1, 7, 2.0));
    // Create convolution context
    COMPV_CHECK_CODE_ASSERT(CompVConvlt::newObj(&convlt));
    // Decode the jpeg image
    COMPV_CHECK_CODE_ASSERT(CompVImageDecoder::decodeFile(JPEG_IMG, &image));
    // Convert image to GrayScale
    COMPV_CHECK_CODE_ASSERT(image->convert(COMPV_PIXEL_FORMAT_GRAYSCALE, &image));
    // Process the image
    timeStart = CompVTime::getNowMills();
    for (int i = 0; i < GAUSS_LOOP_COUNT; ++i) {
        convlt->convlt1((uint8_t*)image->getDataPtr(), image->getWidth(), image->getStride(), image->getHeight(), kern1->getDataPtr(), kern1->getDataPtr(), 7);
    }
    timeEnd = CompVTime::getNowMills();
    COMPV_DEBUG_INFO("Elapsed time(TestGaussFilter1) = [[[ %llu millis ]]]", (timeEnd - timeStart));

    const std::string expectedMD5 = CompVMd5::compute2(convlt->getResultPtr(), convlt->getResultSize());
    if (expectedMD5 != GAUSS_SIGMA2_SIZE7_FILTER_DIM1_MD5) {
        COMPV_DEBUG_ERROR("MD5 mismatch");
        COMPV_ASSERT(false);
        return false;
    }

    // dump image to file
    COMPV_CHECK_CODE_ASSERT(CompVImage::wrap(COMPV_PIXEL_FORMAT_GRAYSCALE, convlt->getResultPtr(), image->getWidth(), image->getHeight(), image->getStride(), &image));
    writeImgToFile(image);

    return true;
}

bool TestGaussKernDim1Gen()
{
    CompVPtr<CompVArray<double>* > kern1;
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
    CompVPtr<CompVArray<double>* > kern2;
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
#endif