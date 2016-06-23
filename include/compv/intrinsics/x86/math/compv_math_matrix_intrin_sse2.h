/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATH_MATRIX_INTRIN_SSE2_H_)
#define _COMPV_MATH_MATRIX_INTRIN_SSE2_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void MatrixMulGA_float64_Intrin_SSE2(COMPV_ALIGNED(SSE) compv_float64_t* ri, COMPV_ALIGNED(SSE) compv_float64_t* rj, const compv_float64_t* c1, const compv_float64_t* s1, compv_uscalar_t count);
void MatrixMulGA_float32_Intrin_SSE2(COMPV_ALIGNED(SSE) compv_float32_t* ri, COMPV_ALIGNED(SSE) compv_float32_t* rj, const compv_float32_t* c1, const compv_float32_t* s1, compv_uscalar_t count);
void MatrixMaxAbsOffDiagSymm_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* S, compv_uscalar_t *row, compv_uscalar_t *col, compv_float64_t* max, compv_uscalar_t rowStart, compv_uscalar_t rowEnd, compv_uscalar_t strideInBytes);
void MatrixIsEqual_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t rows, compv_uscalar_t cols, compv_uscalar_t strideInBytes, compv_scalar_t *equal);
void MatrixMulABt_float64_minpack1_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, const COMPV_ALIGNED(SSE) compv_float64_t* B, compv_uscalar_t aRows, compv_uscalar_t bRows, compv_uscalar_t bCols, compv_uscalar_t aStrideInBytes, compv_uscalar_t bStrideInBytes, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t rStrideInBytes);
void MatrixBuildHomographyEqMatrix_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* dstX, const COMPV_ALIGNED(SSE) compv_float64_t* dstY, COMPV_ALIGNED(SSE) compv_float64_t* M, COMPV_ALIGNED(SSE)compv_uscalar_t M_strideInBytes, compv_uscalar_t numPoints);
void MatrixTranspose_float64_Intrin_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* A, COMPV_ALIGNED(SSE) compv_float64_t* R, compv_uscalar_t a_rows, compv_uscalar_t a_cols, COMPV_ALIGNED(SSE) compv_uscalar_t a_strideInBytes, COMPV_ALIGNED(SSE) compv_uscalar_t r_strideInBytes);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_MATH_MATRIX_INTRIN_SSE2_H_ */
