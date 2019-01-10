/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_transform.h"
#include "compv/base/math/compv_math_matrix.h"
#include "compv/base/math/compv_math_utils.h"

#include "compv/base/math/intrin/x86/compv_math_transform_intrin_avx.h"
#include "compv/base/math/intrin/x86/compv_math_transform_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_transform_intrin_neon64.h"

#define COMPV_THIS_CLASSNAME	"CompVMathTransform"

COMPV_NAMESPACE_BEGIN()
#if COMPV_ASM
#	if COMPV_ARCH_X86
		COMPV_EXTERNC void CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_SSE2(const COMPV_ALIGNED(SSE) compv_float64_t* srcX, const COMPV_ALIGNED(SSE) compv_float64_t* srcY, const COMPV_ALIGNED(SSE) compv_float64_t* srcZ, COMPV_ALIGNED(SSE) compv_float64_t* dstX, COMPV_ALIGNED(SSE) compv_float64_t* dstY, compv_uscalar_t numPoints);
		COMPV_EXTERNC void CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_AVX(const COMPV_ALIGNED(AVX) compv_float64_t* srcX, const COMPV_ALIGNED(AVX) compv_float64_t* srcY, const COMPV_ALIGNED(AVX) compv_float64_t* srcZ, COMPV_ALIGNED(AVX) compv_float64_t* dstX, COMPV_ALIGNED(AVX) compv_float64_t* dstY, compv_uscalar_t numPoints);
#	endif /* COMPV_ARCH_X86 */
#	if COMPV_ARCH_ARM32
		COMPV_EXTERNC void CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_NEON32(const COMPV_ALIGNED(NEON) compv_float64_t* srcX, const COMPV_ALIGNED(NEON) compv_float64_t* srcY, const COMPV_ALIGNED(NEON) compv_float64_t* srcZ, COMPV_ALIGNED(NEON) compv_float64_t* dstX, COMPV_ALIGNED(NEON) compv_float64_t* dstY, compv_uscalar_t numPoints);
#   endif /* COMPV_ARCH_ARM32 */
#	if COMPV_ARCH_ARM64
		COMPV_EXTERNC void CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_NEON64(const COMPV_ALIGNED(NEON) compv_float64_t* srcX, const COMPV_ALIGNED(NEON) compv_float64_t* srcY, const COMPV_ALIGNED(NEON) compv_float64_t* srcZ, COMPV_ALIGNED(NEON) compv_float64_t* dstX, COMPV_ALIGNED(NEON) compv_float64_t* dstY, compv_uscalar_t numPoints);
#   endif /* COMPV_ARCH_ARM64 */
#endif /* COMPV_ASM */

//
//	CompVMathTransformGeneric
//
template<class T>
class CompVMathTransformGeneric {
public:
	// src = homogeneous 2D coordinate (X, Y, 1)
	// dst = cartesian 2D coordinate (x, y)
	// src *must* be different than dst
	// Operation: dst = mul(M, src)
	// For Homography (M = H): src must be homogeneous coordinates
	static COMPV_ERROR_CODE perspective2D(const CompVMatPtr &src, const CompVMatPtr &M, CompVMatPtrPtr dst)
	{
		// TODO(dmi): #4 points too common
		COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wdeprecated-declarations")
		COMPV_CHECK_CODE_RETURN(CompVMatrix::mulAB(M, src, dst));
		COMPV_GCC_DISABLE_WARNINGS_END()
		COMPV_CHECK_CODE_RETURN(CompVMathTransformGeneric<T>::homogeneousToCartesian2D(*dst, dst));

		return COMPV_ERROR_CODE_S_OK;
	}

	// Change from 2D homogeneous coordinates (X, Y, Z) to cartesian 2D coordinates (x, y) = (X/Z, Y/Z)
	static COMPV_ERROR_CODE homogeneousToCartesian2D(const CompVMatPtr &src, CompVMatPtrPtr dst)
	{
		CompVMatPtr dst_ = (src == *dst) ? nullptr : *dst;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&dst_, 2, src->cols()));
		const T* srcX = src->ptr<const T>(0);
		const T* srcY = src->ptr<const T>(1);
		const T* srcZ = src->ptr<const T>(2);
		T* dstX = dst_->ptr<T>(0);
		T* dstY = dst_->ptr<T>(1);
		const size_t cols = src->cols();

		// #4 points is too common (rectangle)

		if (std::is_same<T, compv_float64_t>::value) {
			void(*CompVMathTransformHomogeneousToCartesian2D_64f)(const COMPV_ALIGNED(X) compv_float64_t* srcX, const COMPV_ALIGNED(X) compv_float64_t* srcY, const COMPV_ALIGNED(X) compv_float64_t* srcZ, COMPV_ALIGNED(X) compv_float64_t* dstX, COMPV_ALIGNED(X) compv_float64_t* dstY, compv_uscalar_t numPoints) = NULL;
#if COMPV_ARCH_X86
			if (cols > 1 && CompVCpu::isEnabled(kCpuFlagSSE2) && src->isAlignedSSE() && dst_->isAlignedSSE()) {
				COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_64f_Intrin_SSE2);
				if (cols == 4) {
					COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_4_64f_Intrin_SSE2);
					COMPV_EXEC_IFDEF_ASM_X86(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_SSE2);
				}
			}
			if (cols == 4 && CompVCpu::isEnabled(kCpuFlagAVX) && src->isAlignedAVX() && dst_->isAlignedAVX()) {
				COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_4_64f_Intrin_AVX);
				COMPV_EXEC_IFDEF_ASM_X86(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_X86_AVX);
			}
#elif COMPV_ARCH_ARM
			if (cols > 1 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && src->isAlignedNEON() && dst_->isAlignedNEON()) {
				COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_64f_Intrin_NEON64);
				if (cols == 4) {
					COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_NEON32);
					COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_4_64f_Intrin_NEON64);
					COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathTransformHomogeneousToCartesian2D_64f = CompVMathTransformHomogeneousToCartesian2D_4_64f_Asm_NEON64);
				}
			}
#endif
			if (CompVMathTransformHomogeneousToCartesian2D_64f) {
				CompVMathTransformHomogeneousToCartesian2D_64f(reinterpret_cast<const compv_float64_t*>(srcX), reinterpret_cast<const compv_float64_t*>(srcY), reinterpret_cast<const compv_float64_t*>(srcZ), reinterpret_cast<compv_float64_t*>(dstX), reinterpret_cast<compv_float64_t*>(dstY), static_cast<compv_uscalar_t>(cols));
				*dst = dst_;
				return COMPV_ERROR_CODE_S_OK;
			}
		}

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		T scale;
		for (size_t i = 0; i < cols; ++i) {
			scale = static_cast<T>(1) / srcZ[i]; // z = 0 -> point at infinity (no division error is T is floating point numer)
			dstX[i] = srcX[i] * scale;
			dstY[i] = srcY[i] * scale;
		}
		*dst = dst_;

		return COMPV_ERROR_CODE_S_OK;
	}
};

//
//	CompVMathTransform
//

COMPV_ERROR_CODE CompVMathTransform::perspective2D(const CompVMatPtr &src, const CompVMatPtr &M, CompVMatPtrPtr dst)
{
	COMPV_CHECK_EXP_RETURN(!src || !M || !dst || src->isEmpty() || M->isEmpty() || src->rows() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (src->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVMathTransformGeneric<compv_float32_t>::perspective2D(src, M, dst))); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVMathTransformGeneric<compv_float64_t>::perspective2D(src, M, dst))); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathTransform::homogeneousToCartesian2D(const CompVMatPtr &src, CompVMatPtrPtr dst)
{
	COMPV_CHECK_EXP_RETURN(!src || !dst || src->rows() != 3 || !src->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (src->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN((CompVMathTransformGeneric<compv_float32_t>::homogeneousToCartesian2D(src, dst))); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN((CompVMathTransformGeneric<compv_float64_t>::homogeneousToCartesian2D(src, dst))); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);  break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END() 
