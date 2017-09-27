/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_VIEWPORT_H_)
#define _COMPV_BASE_DRAWING_VIEWPORT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"

COMPV_NAMESPACE_BEGIN()

enum COMPV_VIEWPORT_SIZE_FLAG {
    COMPV_VIEWPORT_SIZE_FLAG_STATIC,
    COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO,
    COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MIN,
    COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MAX
};

struct CompViewportSizeFlags {
    COMPV_VIEWPORT_SIZE_FLAG x, y, width, height;
    CompViewportSizeFlags(
        COMPV_VIEWPORT_SIZE_FLAG x_ = COMPV_VIEWPORT_SIZE_FLAG_STATIC,
        COMPV_VIEWPORT_SIZE_FLAG y_ = COMPV_VIEWPORT_SIZE_FLAG_STATIC,
        COMPV_VIEWPORT_SIZE_FLAG width_ = COMPV_VIEWPORT_SIZE_FLAG_STATIC,
        COMPV_VIEWPORT_SIZE_FLAG height_ = COMPV_VIEWPORT_SIZE_FLAG_STATIC) : x(x_), y(y_), width(width_), height(height_) { }
    static CompViewportSizeFlags makeStatic() {
        return CompViewportSizeFlags(COMPV_VIEWPORT_SIZE_FLAG_STATIC, COMPV_VIEWPORT_SIZE_FLAG_STATIC, COMPV_VIEWPORT_SIZE_FLAG_STATIC, COMPV_VIEWPORT_SIZE_FLAG_STATIC);
    }
    static CompViewportSizeFlags makeDynamicAspectRatio() {
        return CompViewportSizeFlags(COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO, COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO, COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO, COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO);
    }
    static CompViewportSizeFlags makeDynamicFullscreen() {
        return CompViewportSizeFlags(COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MIN, COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MIN, COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MAX, COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MAX);
    }
    bool isStatic()const {
        return (x == COMPV_VIEWPORT_SIZE_FLAG_STATIC) && (y == COMPV_VIEWPORT_SIZE_FLAG_STATIC) && (width == COMPV_VIEWPORT_SIZE_FLAG_STATIC) && (height == COMPV_VIEWPORT_SIZE_FLAG_STATIC);
    }
    bool isDynamic()const {
        return (x != COMPV_VIEWPORT_SIZE_FLAG_STATIC) && (y != COMPV_VIEWPORT_SIZE_FLAG_STATIC) && (width != COMPV_VIEWPORT_SIZE_FLAG_STATIC) && (height != COMPV_VIEWPORT_SIZE_FLAG_STATIC);
    }
};


COMPV_OBJECT_DECLARE_PTRS(Viewport)

class COMPV_BASE_API CompVViewport : public CompVObj
{
protected:
    CompVViewport();
public:
    virtual ~CompVViewport();
    COMPV_OBJECT_GET_ID(CompVViewport);

    COMPV_INLINE int x()const {
        return m_nX;
    }
    COMPV_INLINE int y()const {
        return m_nY;
    }
    COMPV_INLINE int width()const {
        return m_nWidth;
    }
    COMPV_INLINE int height()const {
        return m_nHeight;
    }
    COMPV_INLINE const CompVRatio& aspectRatio()const {
        return m_PixelAspectRatio;
    }
    COMPV_INLINE const CompViewportSizeFlags& sizeFlags()const {
        return m_SizeFlags;
    }

    COMPV_ERROR_CODE reset(const CompViewportSizeFlags& sizeFlags, int x = 0, int y = 0, int width = 0, int height = 0);
	COMPV_ERROR_CODE reset(const CompViewportSizeFlags& sizeFlags, const CompVRectInt& rect);

    COMPV_ERROR_CODE setPixelAspectRatio(const CompVRatio& ratio);

    // Useful to convert "y" from OpenGL ([0,0] = bottom-left) to Doubango ([0,0] = top-left)
    // To be used with glViewport(x, fromFromBottomLeftToTopLeftY(), width, height)
    COMPV_INLINE static int yFromBottomLeftToTopLeft(int viewportHeight, int objHeight, int y) {
        return (viewportHeight - objHeight - y);
    }

    static COMPV_ERROR_CODE toRect(const CompVViewportPtr& viewport, CompVRectInt* rect);

    static COMPV_ERROR_CODE viewport(const CompVRectInt& rcSource, const CompVRectInt& rcDest, const CompVViewportPtr& currViewport, CompVRectInt* rcViewport);

    static COMPV_ERROR_CODE newObj(CompVViewportPtrPtr viewport, const CompViewportSizeFlags& sizeFlags, int x = 0, int y = 0, int width = 0, int height = 0);

private:
    static COMPV_ERROR_CODE letterBoxRect(const CompVRectInt& rcSrc, const CompVRectInt& rcDst, CompVRectInt& rcResult);
    static COMPV_ERROR_CODE correctAspectRatio(const CompVRectInt& rcSrc, const CompVRatio& srcPAR, CompVRectInt& rcResult);

private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    int m_nX;
    int m_nY;
    int m_nWidth;
    int m_nHeight;
    CompVRatio m_PixelAspectRatio;
    CompViewportSizeFlags m_SizeFlags;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_VIEWPORT_H_ */
