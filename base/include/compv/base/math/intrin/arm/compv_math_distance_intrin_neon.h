/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_DISTANCE_INTRIN_NEON_H_)
#define _COMPV_BASE_MATH_DISTANCE_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVMathDistanceHamming_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
void CompVMathDistanceHamming32_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);

void CompVMathDistanceLine_32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
void CompVMathDistanceParabola_32f_Intrin_NEON(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* A, const compv_float32_t* B, const compv_float32_t* C, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_DISTANCE_INTRIN_NEON_H_ */
