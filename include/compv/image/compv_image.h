/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_IMAGE_H_)
#define _COMPV_IMAGE_IMAGE_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_buffer.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVImage : public CompVObj
{
protected:
    CompVImage(COMPV_IMAGE_FORMAT eImageFormat, COMPV_PIXEL_FORMAT ePixelFormat);
public:
    virtual ~CompVImage();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVImage";
    };
    COMPV_ERROR_CODE convert(COMPV_PIXEL_FORMAT eDstPixelFormat, CompVPtr<CompVImage*>* outImage);
    COMPV_ERROR_CODE scale(COMPV_SCALE_TYPE type, int32_t outWidth, int32_t outHeight, CompVPtr<CompVImage*>* outImage);
    COMPV_ERROR_CODE clone(CompVPtr<CompVImage*>* outImage);
    COMPV_ERROR_CODE isEquals(const CompVPtr<CompVImage*>& image, bool &bEquals, int32_t rowStart = -1, int32_t rowCount = -1, int32_t colStart = -1, int32_t colCount = -1);

    size_t getDataSize(COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL);
    const void* getDataPtr(COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL)const;
    int32_t getWidth()const;
    int32_t getHeight(COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL)const;
    int32_t getStride()const;
    COMPV_INLINE COMPV_PIXEL_FORMAT getPixelFormat() {
        return m_ePixelFormat;
    }
    COMPV_INLINE COMPV_IMAGE_FORMAT getImageFormat() {
        return m_eImageFormat;
    }
    COMPV_INLINE COMPV_BORDER_TYPE getBorderType() {
        return m_eBorderType;
    }
    COMPV_INLINE int32_t getBorderWidth() {
        return m_nBorderWidth;
    }
    COMPV_INLINE int32_t getBorderWidthTimes2() {
        return getBorderWidth() << 1;
    }
    COMPV_INLINE int32_t getBorderStride() {
        return m_nBorderStride;
    }
    COMPV_INLINE int32_t getBorderStrideTimes2() {
        return getBorderStride() << 1;
    }
    COMPV_INLINE int32_t getBorderHeight() {
        return m_nBorderHeight;
    }
    COMPV_INLINE int32_t getBorderHeightTimes2() {
        return getBorderHeight() << 1;
    }

    static COMPV_ERROR_CODE getBestStride(int32_t stride, int32_t *bestStride);
    static COMPV_ERROR_CODE getSizeForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, int32_t width, int32_t height, int32_t *size);
    static COMPV_ERROR_CODE getBitsCountForPixelFormat(COMPV_PIXEL_FORMAT ePixelFormat, int32_t* bitsCount);
    static COMPV_ERROR_CODE copy(COMPV_PIXEL_FORMAT ePixelFormat, const void* inPtr, int32_t inWidth, int32_t inHeight, int32_t inStride, void* outPtr, int32_t outWidth, int32_t outHeight, int32_t outStride);
    static COMPV_ERROR_CODE wrap(COMPV_PIXEL_FORMAT ePixelFormat, const void* dataPtr, int32_t width, int32_t height, int32_t stride, CompVPtr<CompVImage*>* image);
    static COMPV_ERROR_CODE newObj(COMPV_IMAGE_FORMAT eImageFormat, COMPV_PIXEL_FORMAT ePixelFormat, CompVPtr<CompVImage*>* image);
#if 0
    static COMPV_ERROR_CODE newObj(COMPV_PIXEL_FORMAT ePixelFormat, int32_t width, int32_t height, int32_t stride, CompVPtr<CompVImage*>* image);
#endif

protected:
    COMPV_ERROR_CODE setBuffer(CompVPtr<CompVBuffer*> & buffer, int32_t width, int32_t height, int32_t stride = 0);

protected:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVPtr<CompVBuffer*> m_oData;
    int32_t m_nWidth;
    int32_t m_nHeight;
    int32_t m_nStride;
    int32_t m_nBorderWidth;
    int32_t m_nBorderStride;
    int32_t m_nBorderHeight;
    COMPV_PIXEL_FORMAT m_ePixelFormat;
    COMPV_PIXEL_FORMAT m_ePixelFormatComp0;
    COMPV_IMAGE_FORMAT m_eImageFormat;
    COMPV_BORDER_TYPE m_eBorderType;
    COMPV_DISABLE_WARNINGS_END()
};

typedef COMPV_ERROR_CODE(*CompVDecodeFileFuncPtr)(const char* filePath, CompVPtr<CompVImage*>* pImage);
typedef COMPV_ERROR_CODE(*CompVDecodeInfoFuncPtr)(const char* filePath, CompVImageInfo& info);

class COMPV_API CompVImageDecoder
{
public:
    static COMPV_ERROR_CODE decodeFile(const char* filePath, CompVPtr<CompVImage*>* image);
    static COMPV_ERROR_CODE decodeInfo(const char* filePath, CompVImageInfo& info);
    static COMPV_ERROR_CODE init();

private:
    static CompVDecodeFileFuncPtr s_funcptrDecodeFileJpeg;
    static CompVDecodeInfoFuncPtr s_funcptrDecodeInfoJpeg;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_IMAGE_H_ */
