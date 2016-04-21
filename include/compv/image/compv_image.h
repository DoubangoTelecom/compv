/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
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
    COMPV_ERROR_CODE convert(COMPV_PIXEL_FORMAT eDstPixelFormat, CompVObjWrapper<CompVImage*>* outImage);
    COMPV_ERROR_CODE scale(COMPV_SCALE_TYPE type, int32_t outWidth, int32_t outHeight, CompVObjWrapper<CompVImage*>* outImage);

    size_t getDataSize(COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL);
    const void* getDataPtr(COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL);
    int32_t getWidth();
    int32_t getHeight(COMPV_BORDER_POS bordersToExclude = COMPV_BORDER_POS_ALL);
    int32_t getStride();
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
    static COMPV_ERROR_CODE wrap(COMPV_PIXEL_FORMAT ePixelFormat, const void* dataPtr, int32_t width, int32_t height, int32_t stride, CompVObjWrapper<CompVImage*>* image);
    static COMPV_ERROR_CODE newObj(COMPV_IMAGE_FORMAT eImageFormat, COMPV_PIXEL_FORMAT ePixelFormat, CompVObjWrapper<CompVImage*>* image);
#if 0
    static COMPV_ERROR_CODE newObj(COMPV_PIXEL_FORMAT ePixelFormat, int32_t width, int32_t height, int32_t stride, CompVObjWrapper<CompVImage*>* image);
#endif

protected:
    COMPV_ERROR_CODE setBuffer(CompVObjWrapper<CompVBuffer*> & buffer, int32_t width, int32_t height, int32_t stride = 0);

protected:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVObjWrapper<CompVBuffer*> m_oData;
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

typedef COMPV_ERROR_CODE(*CompVDecodeFileFuncPtr)(const char* filePath, CompVObjWrapper<CompVImage*>* pImage);
typedef COMPV_ERROR_CODE(*CompVDecodeInfoFuncPtr)(const char* filePath, CompVImageInfo& info);

class COMPV_API CompVImageDecoder
{
public:
    static COMPV_ERROR_CODE decodeFile(const char* filePath, CompVObjWrapper<CompVImage*>* image);
    static COMPV_ERROR_CODE decodeInfo(const char* filePath, CompVImageInfo& info);
    static COMPV_ERROR_CODE init();

private:
    static CompVDecodeFileFuncPtr s_funcptrDecodeFileJpeg;
    static CompVDecodeInfoFuncPtr s_funcptrDecodeInfoJpeg;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_IMAGE_H_ */
