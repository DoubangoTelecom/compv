/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#include "compv/math/compv_math_convlt.h"
#include "compv/compv_cpu.h"

#include "compv/intrinsics/x86/math/compv_math_convlt_intrin_sse2.h"
#include "compv/intrinsics/x86/math/compv_math_convlt_intrin_avx2.h"

COMPV_NAMESPACE_BEGIN()


template</* uint8_t, int16_t, int16_t */>
void CompVMathConvlt::convlt1VertHz(const uint8_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t stride, size_t pad, const int16_t* vhkernPtr, size_t kernSize)
{
#if COMPV_ARCH_X86
	void (*MathConvlt1VertHz_8u16i16i)(const uint8_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize) = NULL;
	if (width >= 16 && CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(MathConvlt1VertHz_8u16i16i = MathConvlt1VertHz_8u16i16i_Intrin_SSE2);
	}
	if (width >= 32 && CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(MathConvlt1VertHz_8u16i16i = MathConvlt1VertHz_8u16i16i_Intrin_AVX2);
	}
	if (MathConvlt1VertHz_8u16i16i) {
		MathConvlt1VertHz_8u16i16i(inPtr, outPtr, (compv_uscalar_t)width, (compv_uscalar_t)height, (compv_uscalar_t)stride, (compv_uscalar_t)pad, vhkernPtr, (compv_uscalar_t)kernSize);
		return;
	}
#endif /* COMPV_ARCH_X86 */
	CompVMathConvlt::convlt1VertHz_C<uint8_t, int16_t, int16_t>(inPtr, outPtr, width, height, stride, pad, vhkernPtr, kernSize);
}

template</* int16_t, int16_t, int16_t */>
void CompVMathConvlt::convlt1VertHz(const int16_t* inPtr, int16_t* outPtr, size_t width, size_t height, size_t stride, size_t pad, const int16_t* vhkernPtr, size_t kernSize)
{
#if COMPV_ARCH_X86
	const static size_t strideInBytes = stride * sizeof(int16_t);
	void(*MathConvlt1VertHz_16i16i16i)(const int16_t* inPtr, int16_t* outPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_uscalar_t pad, const int16_t* vhkernPtr, compv_uscalar_t kernSize) = NULL;
	if (width >= 8 && CompVCpu::isEnabled(compv::kCpuFlagSSE2) && COMPV_IS_ALIGNED_SSE(strideInBytes) && COMPV_IS_ALIGNED_SSE(outPtr) && COMPV_IS_ALIGNED_SSE(inPtr)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(MathConvlt1VertHz_16i16i16i = MathConvlt1VertHz_16i16i16i_Intrin_SSE2);
	}
	if (width >= 16 && CompVCpu::isEnabled(compv::kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX(strideInBytes) && COMPV_IS_ALIGNED_AVX(outPtr) && COMPV_IS_ALIGNED_AVX(inPtr)) {
		COMPV_EXEC_IFDEF_INTRIN_X86(MathConvlt1VertHz_16i16i16i = MathConvlt1VertHz_16i16i16i_Intrin_AVX2);
	}
	if (MathConvlt1VertHz_16i16i16i) {
		MathConvlt1VertHz_16i16i16i(inPtr, outPtr, (compv_uscalar_t)width, (compv_uscalar_t)height, (compv_uscalar_t)stride, (compv_uscalar_t)pad, vhkernPtr, (compv_uscalar_t)kernSize);
		return;
	}
#endif /* COMPV_ARCH_X86 */
	CompVMathConvlt::convlt1VertHz_C<int16_t, int16_t, int16_t>(inPtr, outPtr, width, height, stride, pad, vhkernPtr, kernSize);
}

COMPV_NAMESPACE_END()
