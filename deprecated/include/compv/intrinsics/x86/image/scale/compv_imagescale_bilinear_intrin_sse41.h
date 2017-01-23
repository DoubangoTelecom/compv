/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_IMAGE_SCALE_IMAGESCALE_BILINEAR_INTRIN_SSE41_H_)
#define _COMPV_IMAGE_SCALE_IMAGESCALE_BILINEAR_INTRIN_SSE41_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)

COMPV_NAMESPACE_BEGIN()

void ScaleBilinear_Intrin_SSE41(const uint8_t* inPtr, COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t inHeight, compv_uscalar_t inWidth, compv_uscalar_t inStride, compv_uscalar_t outHeight, compv_uscalar_t outWidth, COMPV_ALIGNED(SSE) compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y);
void ScaleBilinearGrayscale_Intrin_SSE41(const uint8_t* inPtr, COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_uscalar_t inStride, compv_uscalar_t outHeight, compv_uscalar_t outWidth, COMPV_ALIGNED(SSE) compv_uscalar_t outStride, compv_uscalar_t sf_x, compv_uscalar_t sf_y, COMPV_ALIGNED(SSE) int32_t *nearestX, COMPV_ALIGNED(SSE) int32_t *nearestY, COMPV_ALIGNED(SSE) int32_t *x0, COMPV_ALIGNED(SSE) int32_t *y0, COMPV_ALIGNED(SSE) int32_t *x1, COMPV_ALIGNED(SSE) int32_t *y1);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */

#endif /* _COMPV_IMAGE_SCALE_IMAGESCALE_BILINEAR_INTRIN_SSE41_H_ */
