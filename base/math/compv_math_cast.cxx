/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_cast.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_cast_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_cast_intrin_neon.h"

#define COMPV_MATH_CAST_STATIC_PIXEL8_SAMPLES_PER_THREAD	(50*50)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathCastProcess_static_pixel8_32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* src, COMPV_ALIGNED(SSE) uint8_t* dst, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathCastProcess_static_8u32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* src, COMPV_ALIGNED(SSE) compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathCastProcess_static_pixel8_32f_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* src, COMPV_ALIGNED(NEON) uint8_t* dst, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathCastProcess_static_8u32f_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* src, COMPV_ALIGNED(NEON) compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathCastProcess_static_pixel8_32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* src, COMPV_ALIGNED(NEON) uint8_t* dst, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathCastProcess_static_8u32f_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* src, COMPV_ALIGNED(NEON) compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM64 */

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathCast::process_static(const compv_float32_t* src, compv_float64_t* dst, const size_t width, const size_t height, const size_t stride)
{
	void(*CompVMathCastProcessStatic_32f64f)(const compv_float32_t* src, compv_float64_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= [](const compv_float32_t* src, compv_float64_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
	{
		COMPV_CHECK_CODE_NOP((CompVMathCast::process_static_C<compv_float32_t, compv_float64_t>(src, dst, width, height, stride)));
	};

#if COMPV_ARCH_X86
#elif COMPV_ARCH_ARM
#endif

	CompVMathCastProcessStatic_32f64f(src, dst, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));

	return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathCast::process_static(const compv_float64_t* src, compv_float32_t* dst, const size_t width, const size_t height, const size_t stride)
{
	void(*CompVMathCastProcessStatic_64f32f)(const compv_float64_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= [](const compv_float64_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
	{
		COMPV_CHECK_CODE_NOP((CompVMathCast::process_static_C<compv_float64_t, compv_float32_t>(src, dst, width, height, stride)));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(src) && COMPV_IS_ALIGNED_SSE(dst) && COMPV_IS_ALIGNED_SSE(stride * sizeof(compv_float64_t)) && COMPV_IS_ALIGNED_SSE(stride * sizeof(compv_float32_t))) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_64f32f = CompVMathCastProcess_static_64f32f_Intrin_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(src) && COMPV_IS_ALIGNED_NEON(dst) && COMPV_IS_ALIGNED_NEON(stride * sizeof(compv_float64_t)) && COMPV_IS_ALIGNED_NEON(stride * sizeof(compv_float32_t))) {
		COMPV_EXEC_IFDEF_INTRIN_ARM64(CompVMathCastProcessStatic_64f32f = CompVMathCastProcess_static_64f32f_Intrin_NEON64);
	}
#endif

	CompVMathCastProcessStatic_64f32f(src, dst, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));

	return COMPV_ERROR_CODE_S_OK;
}

template <> COMPV_BASE_API
COMPV_ERROR_CODE CompVMathCast::process_static(const uint8_t* src, compv_float32_t* dst, const size_t width, const size_t height, const size_t stride)
{
	void(*CompVMathCastProcessStatic_8u32f)(const uint8_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
		= [](const uint8_t* src, compv_float32_t* dst, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
	{
		COMPV_CHECK_CODE_NOP((CompVMathCast::process_static_C<uint8_t, compv_float32_t>(src, dst, width, height, stride)));
	};

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(src) && COMPV_IS_ALIGNED_SSE(dst) && COMPV_IS_ALIGNED_SSE(stride * sizeof(uint8_t)) && COMPV_IS_ALIGNED_SSE(stride * sizeof(compv_float32_t))) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcessStatic_8u32f = CompVMathCastProcess_static_8u32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVMathCastProcessStatic_8u32f = CompVMathCastProcess_static_8u32f_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(src) && COMPV_IS_ALIGNED_NEON(dst) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathCastProcessStatic_8u32f = CompVMathCastProcess_static_8u32f_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathCastProcessStatic_8u32f = CompVMathCastProcess_static_8u32f_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathCastProcessStatic_8u32f = CompVMathCastProcess_static_8u32f_Asm_NEON64);
	}
#endif

	CompVMathCastProcessStatic_8u32f(src, dst, static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride));

	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static void CompVMathCastProcess_static_pixel8(const CompVMatPtr& srcMat, CompVMatPtr& dstMat)
{
	// Private function, no need to check input parameters

	const size_t width = srcMat->cols();
	const size_t height = srcMat->rows();
	const size_t stride = srcMat->stride();

	const T* src = srcMat->ptr<const T>();
	uint8_t* dst = dstMat->ptr<uint8_t>();

	if (std::is_same<T, compv_float32_t>::value) {
		void (*CompVMathCastProcess_static_pixel8_32f)(const compv_float32_t* src, uint8_t* dst, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && srcMat->isAlignedSSE() && dstMat->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathCastProcess_static_pixel8_32f = CompVMathCastProcess_static_pixel8_32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathCastProcess_static_pixel8_32f = CompVMathCastProcess_static_pixel8_32f_Asm_X64_SSE2);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && srcMat->isAlignedNEON() && dstMat->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathCastProcess_static_pixel8_32f = CompVMathCastProcess_static_pixel8_32f_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathCastProcess_static_pixel8_32f = CompVMathCastProcess_static_pixel8_32f_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathCastProcess_static_pixel8_32f = CompVMathCastProcess_static_pixel8_32f_Asm_NEON64);
		}
#endif
		if (CompVMathCastProcess_static_pixel8_32f) {
			CompVMathCastProcess_static_pixel8_32f(
				reinterpret_cast<const compv_float32_t*>(src), dst,
				static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(height), static_cast<compv_uscalar_t>(stride)
			);
			return;
		}
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			dst[i] = static_cast<uint8_t>(COMPV_MATH_CLIP3(0, 255, src[i])); // TODO(dmi): SIMD, use saturation
		}
		src += stride;
		dst += stride;
	}
}

COMPV_ERROR_CODE CompVMathCast::process_static_pixel8(const CompVMatPtr& src, CompVMatPtrPtr dst)
{
	COMPV_CHECK_EXP_RETURN(!src || src->planeCount() != 1 || !dst, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const size_t width = src->cols();
	const size_t height = src->rows();
	const size_t stride = src->stride();

	CompVMatPtr dst_ = (*dst == src) ? nullptr : *dst;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&dst_, height, width, stride));

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(width - 1), // right
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr srcBind, dstBind;
		COMPV_CHECK_CODE_RETURN(src->bind(&srcBind, roi));
		COMPV_CHECK_CODE_RETURN(dst_->bind(&dstBind, roi));
		CompVGenericInvokeVoidRawType(srcBind->subType(), CompVMathCastProcess_static_pixel8, srcBind, dstBind);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_MATH_CAST_STATIC_PIXEL8_SAMPLES_PER_THREAD
	));

	*dst = dst_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
