#include <compv/compv_api.h>

#if !defined(COMPV_TESTS_COMMON_H_)
#define COMPV_TESTS_COMMON_H_

using namespace compv;

static const std::string formatExtension(COMPV_PIXEL_FORMAT pixelFormat)
{
    switch (pixelFormat) {
    case COMPV_PIXEL_FORMAT_R8G8B8A8:
        return "rgba";
    case COMPV_PIXEL_FORMAT_A8R8G8B8:
        return "argb";
    case COMPV_PIXEL_FORMAT_B8G8R8A8:
        return "bgra";
    case COMPV_PIXEL_FORMAT_A8B8G8R8:
        return "abgr";
    case COMPV_PIXEL_FORMAT_R8G8B8:
        return "rgb";
    case COMPV_PIXEL_FORMAT_B8G8R8:
        return "bgr";
    case COMPV_PIXEL_FORMAT_I420:
        return "i420";
    case COMPV_PIXEL_FORMAT_GRAYSCALE:
        return "gray";
    default:
        return "unknown";
    }
}

static void writeImgToFile(const CompVPtr<CompVImage *>& img, COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL)
{
    if (img) {
        std::string fileName = "./out." + formatExtension(img->getPixelFormat());
        FILE* file = fopen(fileName.c_str(), "wb+");
        COMPV_ASSERT(file != NULL);
        if (file) {
            COMPV_DEBUG_INFO("Writing %s file... (w=%d, h=%d, s=%d)", fileName.c_str(), img->getWidth(), img->getHeight(), img->getStride());

#if 0
            fwrite(img->getDataPtr(COMPV_BORDER_POS_NONE), 1, img->getDataSize(COMPV_BORDER_POS_NONE), file);
#else
            int width = img->getWidth();
            int stride = img->getStride();
            int height = img->getHeight(bordersToExclude);
            const uint8_t* ptr = (const uint8_t*)img->getDataPtr(bordersToExclude);

            switch (img->getPixelFormat()) {
            case COMPV_PIXEL_FORMAT_R8G8B8:
            case COMPV_PIXEL_FORMAT_B8G8R8:
            case COMPV_PIXEL_FORMAT_R8G8B8A8:
            case COMPV_PIXEL_FORMAT_B8G8R8A8:
            case COMPV_PIXEL_FORMAT_A8B8G8R8:
            case COMPV_PIXEL_FORMAT_A8R8G8B8:
            case COMPV_PIXEL_FORMAT_GRAYSCALE: {
                CompVImage::getSizeForPixelFormat(img->getPixelFormat(), stride, 1, &stride);
                CompVImage::getSizeForPixelFormat(img->getPixelFormat(), width, 1, &width);
                for (int i = 0; i < height; ++i) {
                    fwrite(ptr, 1, width, file);
                    ptr += stride;
                }
                break;
            }
            case COMPV_PIXEL_FORMAT_I420: {
                // Y
                for (int i = 0; i < height; ++i) {
                    fwrite(ptr, 1, width, file);
                    ptr += stride;
                }
                height = (height + 1) >> 1;
                width = (width + 1) >> 1;
                stride = (stride + 1) >> 1;
                // U
                for (int i = 0; i < height; ++i) {
                    fwrite(ptr, 1, width, file);
                    ptr += stride;
                }
                // V
                for (int i = 0; i < height; ++i) {
                    fwrite(ptr, 1, width, file);
                    ptr += stride;
                }
                break;
            }
            default: {
                COMPV_DEBUG_ERROR("Not implemented");
                COMPV_ASSERT(false);
                break;
            }
            }
#endif

            fclose(file);
        }
    }
}

static const std::string imageMD5(const CompVPtr<CompVImage *>& img, COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL)
{
    if (img) {
        CompVPtr<CompVMd5*> md5;
        COMPV_CHECK_CODE_ASSERT(CompVMd5::newObj(&md5));
        int width = img->getWidth();
        int stride = img->getStride();
        int height = img->getHeight(bordersToExclude);
        CompVImage::getSizeForPixelFormat(img->getPixelFormat(), stride, 1, &stride);
        CompVImage::getSizeForPixelFormat(img->getPixelFormat(), width, 1, &width);
        const uint8_t* ptr = (const uint8_t*)img->getDataPtr(bordersToExclude);
        for (int i = 0; i < height; ++i) {
            COMPV_CHECK_CODE_ASSERT(md5->update(ptr, width));
            ptr += stride;
        }
        return md5->compute();
    }
    return "";
}

template<typename T>
static const std::string arrayMD5(const CompVPtrArray(T)& array)
{
    if (array) {
        CompVPtr<CompVMd5*> md5;
        COMPV_CHECK_CODE_ASSERT(CompVMd5::newObj(&md5));
        const size_t rowInBytes = array->rowInBytes();
        for (size_t row = 0; row < array->rows(); ++row) {
            COMPV_CHECK_CODE_ASSERT(md5->update((const uint8_t*)array->ptr(row), rowInBytes));
        }
        return md5->compute();
    }
    return "";
}

template <typename T>
static COMPV_ERROR_CODE arrayToFile(const CompVPtrArray(T)& data, const char* filename = "./out.gray")
{
    FILE* file = fopen(filename, "wb+");
    COMPV_CHECK_EXP_RETURN(!file, COMPV_ERROR_CODE_E_FILE_NOT_FOUND);
    COMPV_DEBUG_INFO("Writing %s file... (cols=%d, rows=%d)", filename, data->cols(), data->rows());
    for (size_t i = 0; i < data->rows(); ++i) {
        fwrite(data->ptr(i), 1, data->rowInBytes(), file);
    }
    fclose(file);
    return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static void matrixPrint(const CompVPtrArray(T)& M, const char* name)
{
    const T* row;
    const char* type_sz = std::is_same<T, float>::value ? "%f" : (std::is_same<T, double>::value ? "%e" : "%d");
    printf("%s={", name);
    for (size_t i = 0; i < M->rows(); ++i) {
        row = M->ptr(i);
        printf("{");
        for (size_t j = 0; j < M->cols(); ++j) {
            printf(type_sz, row[j]);
            printf(", ");
        }
        printf("}\n");
    }
    printf("}\n");
}

#endif /* COMPV_TESTS_COMMON_H_ */
