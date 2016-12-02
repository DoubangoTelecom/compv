/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/drawing/compv_viewport.h"
#include "compv/base/compv_base.h"

COMPV_NAMESPACE_BEGIN()

CompVViewport::CompVViewport()
    : m_nX(0)
    , m_nY(0)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_SizeFlags(CompViewportSizeFlags::makeDynamicAspectRatio())
{

}

CompVViewport::~CompVViewport()
{

}

COMPV_ERROR_CODE CompVViewport::reset(const CompViewportSizeFlags& sizeFlags, int x /*= 0*/, int y /*= 0*/, int width /*= 0*/, int height /*= 0*/)
{
    COMPV_CHECK_EXP_RETURN(width <= 0 && sizeFlags.width == COMPV_VIEWPORT_SIZE_FLAG_STATIC, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_CHECK_EXP_RETURN(height <= 0 && sizeFlags.height == COMPV_VIEWPORT_SIZE_FLAG_STATIC, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_nX = x;
    m_nY = y;
    m_nWidth = width;
    m_nHeight = height;
    m_SizeFlags = sizeFlags;
    m_SizeFlags = sizeFlags;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVViewport::setPixelAspectRatio(const CompVRatio& ratio)
{
    COMPV_CHECK_EXP_RETURN(ratio.numerator < 0 || ratio.denominator <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    m_PixelAspectRatio = ratio;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVViewport::toRect(const CompVViewportPtr& viewport, CompVRect* rect)
{
    COMPV_CHECK_EXP_RETURN(!viewport || !rect, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    rect->left = static_cast<int>(viewport->x());
    rect->top = static_cast<int>(viewport->y());
    rect->right = static_cast<int>(viewport->x() + viewport->width());
    rect->bottom = static_cast<int>(viewport->y() + viewport->height());
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVViewport::newObj(CompVViewportPtrPtr viewport, const CompViewportSizeFlags& sizeFlags, int x /*= 0*/, int y /*= 0*/, int width /*= 0*/, int height /*= 0*/)
{
    COMPV_CHECK_EXP_RETURN(!viewport, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVViewportPtr viewport_ = new CompVViewport();
    COMPV_CHECK_EXP_RETURN(!viewport_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(viewport_->reset(sizeFlags, x, y, width, height));

    *viewport = viewport_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVViewport::viewport(const CompVRect& rcSource, const CompVRect& rcDest, const CompVViewportPtr& currViewport, CompVRect* rcViewport)
{
    COMPV_CHECK_EXP_RETURN(!rcViewport || !currViewport, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    if (currViewport->sizeFlags().isStatic()) {
        COMPV_CHECK_CODE_RETURN(CompVViewport::toRect(currViewport, rcViewport));
        return COMPV_ERROR_CODE_S_OK;
    }

    const CompViewportSizeFlags& sizeFlags = currViewport->sizeFlags();
    CompVRect rcAspectRatio;
    int x, y, width, height;

    if (sizeFlags.x == COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO || sizeFlags.y == COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO || sizeFlags.width == COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO || sizeFlags.height == COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO) {
        CompVRect rcSrc;
        COMPV_CHECK_CODE_RETURN(CompVViewport::correctAspectRatio(rcSource, currViewport->aspectRatio(), rcSrc));
        COMPV_CHECK_CODE_RETURN(CompVViewport::letterBoxRect(rcSrc, rcDest, rcAspectRatio));
    }

    switch (sizeFlags.x) {
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO:
        x = rcAspectRatio.left;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MIN:
        x = rcDest.left;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MAX:
        x = rcDest.right;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_STATIC:
        x = currViewport->x();
        break;
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }
    switch (sizeFlags.y) {
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO:
        y = rcAspectRatio.top;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MIN:
        y = rcDest.top;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MAX:
        y = rcDest.bottom;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_STATIC:
        y = static_cast<int>(currViewport->y());
        break;
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }
    switch (sizeFlags.width) {
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO:
        width = (rcAspectRatio.right - rcAspectRatio.left);
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MIN:
        width = rcDest.left;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MAX:
        width = rcDest.right;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_STATIC:
        width = currViewport->width();
        break;
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }
    switch (sizeFlags.height) {
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_ASPECT_RATIO:
        height = (rcAspectRatio.bottom - rcAspectRatio.top);
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MIN:
        height = rcDest.top;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_DYNAMIC_MAX:
        height = rcDest.bottom;
        break;
    case COMPV_VIEWPORT_SIZE_FLAG_STATIC:
        height = currViewport->height();
        break;
    default:
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
        break;
    }

    rcViewport->left = x;
    rcViewport->top = y;
    rcViewport->right = x + width;
    rcViewport->bottom = y + height;

    return COMPV_ERROR_CODE_S_OK;
}

#if COMPV_OS_WINDOWS && 0 // FIXME(dmi): uncomment after full test
#define COMPV_MulDiv MulDiv
#else
static COMPV_INLINE int COMPV_MulDiv(int number, int numerator, int denominator)
{
    if (denominator != 0) {
        int64_t x = static_cast<int64_t>(number) * static_cast<int64_t>(numerator);
        x /= static_cast<int64_t>(denominator);
        return static_cast<int>(x);
    }
    return 0;
}
#endif

//-----------------------------------------------------------------------------
// CorrectAspectRatio
//
// Converts a rectangle from the source's pixel aspect ratio (PAR) to 1:1 PAR.
// Returns the corrected rectangle.
//
// For example, a 720 x 486 rect with a PAR of 9:10, when converted to 1x1 PAR,
// is stretched to 720 x 540.
// Copyright (C) Microsoft
//-----------------------------------------------------------------------------
COMPV_ERROR_CODE CompVViewport::correctAspectRatio(const CompVRect& rcSrc, const CompVRatio& srcPAR, CompVRect& rcResult)
{
    // Start with a rectangle the same size as src, but offset to the origin (0,0).
    rcResult = CompVRect(0, 0, rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top);

    if ((srcPAR.numerator != 1) || (srcPAR.denominator != 1)) {
        // Correct for the source's PAR.

        if (srcPAR.numerator > srcPAR.denominator) {
            // The source has "wide" pixels, so stretch the width.
            rcResult.right = COMPV_MulDiv(rcResult.right, srcPAR.numerator, srcPAR.denominator);
        }
        else if (srcPAR.numerator < srcPAR.denominator) {
            // The source has "tall" pixels, so stretch the height.
            rcResult.bottom = COMPV_MulDiv(rcResult.bottom, srcPAR.denominator, srcPAR.numerator);
        }
        // else: PAR is 1:1, which is a no-op.
    }
    return COMPV_ERROR_CODE_S_OK;
}

//-------------------------------------------------------------------
// LetterBoxDstRect
//
// Takes a src rectangle and constructs the largest possible
// destination rectangle within the specifed destination rectangle
// such thatthe video maintains its current shape.
//
// This function assumes that pels are the same shape within both the
// source and destination rectangles.
// Copyright (C) Microsoft
//-------------------------------------------------------------------
COMPV_ERROR_CODE CompVViewport::letterBoxRect(const CompVRect& rcSrc, const CompVRect& rcDst, CompVRect& rcResult)
{
    // figure out src/dest scale ratios
    int iSrcWidth = rcSrc.right - rcSrc.left;
    int iSrcHeight = rcSrc.bottom - rcSrc.top;

    int iDstWidth = rcDst.right - rcDst.left;
    int iDstHeight = rcDst.bottom - rcDst.top;

    int iDstLBWidth;
    int iDstLBHeight;

    if (COMPV_MulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth) {
        // Column letter boxing ("pillar box")
        iDstLBWidth = COMPV_MulDiv(iDstHeight, iSrcWidth, iSrcHeight);
        iDstLBHeight = iDstHeight;
    }
    else {
        // Row letter boxing.
        iDstLBWidth = iDstWidth;
        iDstLBHeight = COMPV_MulDiv(iDstWidth, iSrcHeight, iSrcWidth);
    }

    // Create a centered rectangle within the current destination rect
    rcResult.left = rcDst.left + ((iDstWidth - iDstLBWidth) >> 1);
    rcResult.top = rcDst.top + ((iDstHeight - iDstLBHeight) >> 1);
    rcResult.right = rcResult.left + iDstLBWidth;
    rcResult.bottom = rcResult.top + iDstLBHeight;

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

