/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_gradient_fast.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/math/compv_math_trig.h"

#include "compv/base/intrin/x86/compv_gradient_fast_intrin_sse2.h"
#include "compv/base/intrin/arm/compv_gradient_fast_intrin_neon.h"

#define COMPV_GRADIENT_FAST_GRADX_8U16S_SAMPLES_PER_THREAD		(64 * 64)
#define COMPV_GRADIENT_FAST_GRADX_8U32F_SAMPLES_PER_THREAD		(64 * 64)
#define COMPV_GRADIENT_FAST_GRADX_32F32F_SAMPLES_PER_THREAD		(80 * 80)
#define COMPV_GRADIENT_FAST_GRADY_8U16S_SAMPLES_PER_THREAD		(64 * 64)
#define COMPV_GRADIENT_FAST_GRADY_8U32F_SAMPLES_PER_THREAD		(64 * 64)
#define COMPV_GRADIENT_FAST_GRADY_32F32F_SAMPLES_PER_THREAD		(80 * 80)

//
// Gradient computation using [-1, 0, 1] kernel without convolution (no mul, add/sub only)
//

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVGradientFastGradX_8u32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradX_8u16s_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradY_8u32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradY_8u16s_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* input, COMPV_ALIGNED(SSE) int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVGradientFastGradX_8u32f_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradX_8u16s_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradY_8u32f_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradY_8u16s_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVGradientFastGradX_8u32f_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradX_8u16s_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradY_8u32f_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVGradientFastGradY_8u16s_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* input, COMPV_ALIGNED(NEON) int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

static void CompVGradientFastGradX_8u16s_C(const uint8_t* input, int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVGradientFastGradX_8u32f_C(const uint8_t* input, compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVGradientFastGradX_32f32f_C(const compv_float32_t* input, compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

static void CompVGradientFastGradY_8u16s_C(const uint8_t* input, int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVGradientFastGradY_8u32f_C(const uint8_t* input, compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVGradientFastGradY_32f32f_C(const compv_float32_t* input, compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);

COMPV_ERROR_CODE CompVGradientFast::gradX_8u16s(const CompVMatPtr& input, CompVMatPtrPtr outputX)
{
	// Private function: do not check input parameters

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr outputX_ = (*outputX == input) ? nullptr : *outputX;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int16_t>(&outputX_, height, width, stride));

	void(*CompVGradientFastGradX_8u16s)(const uint8_t* input, int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVGradientFastGradX_8u16s_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && input->isAlignedSSE() && outputX_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVGradientFastGradX_8u16s = CompVGradientFastGradX_8u16s_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVGradientFastGradX_8u16s = CompVGradientFastGradX_8u16s_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && input->isAlignedNEON() && outputX_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVGradientFastGradX_8u16s = CompVGradientFastGradX_8u16s_Intrin_NEON);
        COMPV_EXEC_IFDEF_ASM_ARM32(CompVGradientFastGradX_8u16s = CompVGradientFastGradX_8u16s_Asm_NEON32);
        COMPV_EXEC_IFDEF_ASM_ARM64(CompVGradientFastGradX_8u16s = CompVGradientFastGradX_8u16s_Asm_NEON64);
	}
#endif

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		int16_t* outPtr = outputX_->ptr<int16_t>(ystart);
		const size_t h = static_cast<compv_uscalar_t>(yend - ystart);
		const size_t widthMinus1 = width - 1;
		CompVGradientFastGradX_8u16s(input->ptr<const uint8_t>(ystart), outPtr,
			static_cast<compv_uscalar_t>(widthMinus1), static_cast<compv_uscalar_t>(h), static_cast<compv_uscalar_t>(stride));
		// Set lef/right borders to zero
		// SIMD code will write on the borders and this is why we reset them AFTER the op
		{
			const size_t maxh = h * stride;
			for (compv_uscalar_t j = 0; j < maxh; j += stride) {
				outPtr[j] = 0;
				outPtr[j + widthMinus1] = 0;
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_GRADIENT_FAST_GRADX_8U16S_SAMPLES_PER_THREAD
	));

	*outputX = outputX_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGradientFast::gradX_8u32f(const CompVMatPtr& input, CompVMatPtrPtr outputX)
{
	// Private function: do not check input parameters

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr outputX_ = (*outputX == input) ? nullptr : *outputX;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&outputX_, height, width, stride));

	void(*CompVGradientFastGradX_8u32f)(const uint8_t* input, compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVGradientFastGradX_8u32f_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && input->isAlignedSSE() && outputX_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVGradientFastGradX_8u32f = CompVGradientFastGradX_8u32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVGradientFastGradX_8u32f = CompVGradientFastGradX_8u32f_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
    if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && input->isAlignedNEON() && outputX_->isAlignedNEON()) {
        COMPV_EXEC_IFDEF_INTRIN_ARM(CompVGradientFastGradX_8u32f = CompVGradientFastGradX_8u32f_Intrin_NEON);
        COMPV_EXEC_IFDEF_ASM_ARM32(CompVGradientFastGradX_8u32f = CompVGradientFastGradX_8u32f_Asm_NEON32);
        COMPV_EXEC_IFDEF_ASM_ARM64(CompVGradientFastGradX_8u32f = CompVGradientFastGradX_8u32f_Asm_NEON64);
    }
#endif

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_float32_t* outPtr = outputX_->ptr<compv_float32_t>(ystart);
		const size_t h = static_cast<compv_uscalar_t>(yend - ystart);
		const size_t widthMinus1 = width - 1;
		CompVGradientFastGradX_8u32f(input->ptr<const uint8_t>(ystart), outPtr,
			static_cast<compv_uscalar_t>(widthMinus1), static_cast<compv_uscalar_t>(h), static_cast<compv_uscalar_t>(stride));
		// Set lef/right borders to zero
		// SIMD code will write on the borders and this is why we reset them AFTER the op
		{
			const size_t maxh = h * stride;
			for (compv_uscalar_t j = 0; j < maxh; j += stride) {
				outPtr[j] = 0;
				outPtr[j + widthMinus1] = 0;
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_GRADIENT_FAST_GRADX_8U32F_SAMPLES_PER_THREAD
	));

	*outputX = outputX_;
	return COMPV_ERROR_CODE_S_OK;
}


COMPV_ERROR_CODE CompVGradientFast::gradX_32f32f(const CompVMatPtr& input, CompVMatPtrPtr outputX)
{
	// Private function: do not check input parameters

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr outputX_ = (*outputX == input) ? nullptr : *outputX;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&outputX_, height, width, stride));

	void(*CompVGradientFastGradX_32f32f)(const compv_float32_t* input, compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVGradientFastGradX_32f32f_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && input->isAlignedSSE() && outputX_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVGradientFastGradX_32f32f = CompVGradientFastGradX_32f32f_Intrin_SSE2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVGradientFastGradX_32f32f = CompVGradientFastGradX_32f32f_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && input->isAlignedNEON() && outputX_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVGradientFastGradX_32f32f = CompVGradientFastGradX_32f32f_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVGradientFastGradX_32f32f = CompVGradientFastGradX_32f32f_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVGradientFastGradX_32f32f = CompVGradientFastGradX_32f32f_Asm_NEON64);
	}
#endif

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_float32_t* outPtr = outputX_->ptr<compv_float32_t>(ystart);
		const size_t h = static_cast<compv_uscalar_t>(yend - ystart);
		const size_t widthMinus1 = width - 1;
		CompVGradientFastGradX_32f32f(input->ptr<const compv_float32_t>(ystart), outPtr,
			static_cast<compv_uscalar_t>(widthMinus1), static_cast<compv_uscalar_t>(h), static_cast<compv_uscalar_t>(stride));
		// Set lef/right borders to zero
		// SIMD code will write on the borders and this is why we reset them AFTER the op
		{
			const size_t maxh = h * stride;
			for (compv_uscalar_t j = 0; j < maxh; j += stride) {
				outPtr[j] = 0;
				outPtr[j + widthMinus1] = 0;
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_GRADIENT_FAST_GRADX_32F32F_SAMPLES_PER_THREAD
	));

	*outputX = outputX_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGradientFast::gradY_8u16s(const CompVMatPtr& input, CompVMatPtrPtr outputY)
{
	// Private function: do not check input parameters

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr outputY_ = (*outputY == input) ? nullptr : *outputY;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int16_t>(&outputY_, height, width, stride));

	void(*CompVGradientFastGradY_8u16s)(const uint8_t* input, int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVGradientFastGradY_8u16s_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && input->isAlignedSSE() && outputY_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVGradientFastGradY_8u16s = CompVGradientFastGradY_8u16s_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVGradientFastGradY_8u16s = CompVGradientFastGradY_8u16s_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
    if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && input->isAlignedNEON() && outputY_->isAlignedNEON()) {
        COMPV_EXEC_IFDEF_INTRIN_ARM(CompVGradientFastGradY_8u16s = CompVGradientFastGradY_8u16s_Intrin_NEON);
        COMPV_EXEC_IFDEF_ASM_ARM32(CompVGradientFastGradY_8u16s = CompVGradientFastGradY_8u16s_Asm_NEON32);
        COMPV_EXEC_IFDEF_ASM_ARM64(CompVGradientFastGradY_8u16s = CompVGradientFastGradY_8u16s_Asm_NEON64);
    }
#endif

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t ystart_ = ystart + (!ystart);
		const size_t yend_ = yend - (yend == height);
		CompVGradientFastGradY_8u16s(input->ptr<const uint8_t>(ystart_), outputY_->ptr<int16_t>(ystart_),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend_ - ystart_), static_cast<compv_uscalar_t>(stride));
		if (!ystart) {
			COMPV_CHECK_CODE_RETURN(outputY_->zero_row(0));
		}
		if (yend == height) {
			COMPV_CHECK_CODE_RETURN(outputY_->zero_row(height - 1));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_GRADIENT_FAST_GRADY_8U16S_SAMPLES_PER_THREAD
	));

	*outputY = outputY_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGradientFast::gradY_8u32f(const CompVMatPtr& input, CompVMatPtrPtr outputY)
{
	// Private function: do not check input parameters

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr outputY_ = (*outputY == input) ? nullptr : *outputY;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&outputY_, height, width, stride));

	void(*CompVGradientFastGradY_8u32f)(const uint8_t* input, compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVGradientFastGradY_8u32f_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && input->isAlignedSSE() && outputY_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVGradientFastGradY_8u32f = CompVGradientFastGradY_8u32f_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVGradientFastGradY_8u32f = CompVGradientFastGradY_8u32f_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
    if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && input->isAlignedNEON() && outputY_->isAlignedNEON()) {
        COMPV_EXEC_IFDEF_INTRIN_ARM(CompVGradientFastGradY_8u32f = CompVGradientFastGradY_8u32f_Intrin_NEON);
        COMPV_EXEC_IFDEF_ASM_ARM32(CompVGradientFastGradY_8u32f = CompVGradientFastGradY_8u32f_Asm_NEON32);
        COMPV_EXEC_IFDEF_ASM_ARM64(CompVGradientFastGradY_8u32f = CompVGradientFastGradY_8u32f_Asm_NEON64);
    }
#endif

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t ystart_ = ystart + (!ystart);
		const size_t yend_ = yend - (yend == height);
		CompVGradientFastGradY_8u32f(input->ptr<const uint8_t>(ystart_), outputY_->ptr<compv_float32_t>(ystart_),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend_ - ystart_), static_cast<compv_uscalar_t>(stride));
		if (!ystart) {
			COMPV_CHECK_CODE_RETURN(outputY_->zero_row(0));
		}
		if (yend == height) {
			COMPV_CHECK_CODE_RETURN(outputY_->zero_row(height - 1));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_GRADIENT_FAST_GRADY_8U16S_SAMPLES_PER_THREAD
	));

	*outputY = outputY_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGradientFast::gradY_32f32f(const CompVMatPtr& input, CompVMatPtrPtr outputY)
{
	// Private function: do not check input parameters

	const size_t width = input->cols();
	const size_t height = input->rows();
	const size_t stride = input->stride();
	CompVMatPtr outputY_ = (*outputY == input) ? nullptr : *outputY;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&outputY_, height, width, stride));

	void(*CompVGradientFastGradY_32f32f)(const compv_float32_t* input, compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
		= CompVGradientFastGradY_32f32f_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && input->isAlignedSSE() && outputY_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVGradientFastGradY_32f32f = CompVGradientFastGradY_32f32f_Intrin_SSE2);
		//COMPV_EXEC_IFDEF_ASM_X64(CompVGradientFastGradY_32f32f = CompVGradientFastGradY_32f32f_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && input->isAlignedNEON() && outputY_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVGradientFastGradY_32f32f = CompVGradientFastGradY_32f32f_Intrin_NEON);
		//COMPV_EXEC_IFDEF_ASM_ARM32(CompVGradientFastGradY_32f32f = CompVGradientFastGradY_32f32f_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(CompVGradientFastGradY_32f32f = CompVGradientFastGradY_32f32f_Asm_NEON64);
	}
#endif

	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t ystart_ = ystart + (!ystart);
		const size_t yend_ = yend - (yend == height);
		CompVGradientFastGradY_32f32f(input->ptr<const compv_float32_t>(ystart_), outputY_->ptr<compv_float32_t>(ystart_),
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend_ - ystart_), static_cast<compv_uscalar_t>(stride));
		if (!ystart) {
			COMPV_CHECK_CODE_RETURN(outputY_->zero_row(0));
		}
		if (yend == height) {
			COMPV_CHECK_CODE_RETURN(outputY_->zero_row(height - 1));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		COMPV_GRADIENT_FAST_GRADY_32F32F_SAMPLES_PER_THREAD
	));

	*outputY = outputY_;
	return COMPV_ERROR_CODE_S_OK;
}


COMPV_ERROR_CODE CompVGradientFast::magnitude(const CompVMatPtr& input, CompVMatPtrPtr mag)
{
	CompVMatPtr gx, gy;
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradX<compv_float32_t>(input, &gx));
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradY<compv_float32_t>(input, &gy));
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::magnitude(gx, gy, mag));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGradientFast::magnitude(const CompVMatPtr& gradX, const CompVMatPtr& gradY, CompVMatPtrPtr mag)
{
	COMPV_CHECK_CODE_RETURN(CompVMathTrig::hypot_naive(gradX, gradY, mag)); // No overflow/underflow -> use naive implementatio (more info: https://improbable.io/games/blog/thanos-prometheus-at-scale)
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGradientFast::direction(const CompVMatPtr& input, CompVMatPtrPtr dir, const bool angleInDeg)
{
	CompVMatPtr gx, gy;
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradX<compv_float32_t>(input, &gx));
	COMPV_CHECK_CODE_RETURN(CompVGradientFast::gradY<compv_float32_t>(input, &gy));
	COMPV_CHECK_CODE_RETURN(CompVMathTrig::fastAtan2(gx, gy, dir, angleInDeg));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGradientFast::direction(const CompVMatPtr& gradX, const CompVMatPtr& gradY, CompVMatPtrPtr dir, const bool angleInDeg)
{
	COMPV_CHECK_CODE_RETURN(CompVMathTrig::fastAtan2(gradY, gradX, dir, angleInDeg));
	return COMPV_ERROR_CODE_S_OK;
}

template<typename InType, typename OutType>
static void CompVGradientFastGradX_C(const InType* input, OutType* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			dx[i] = static_cast<OutType>(input[i + 1] - input[i - 1]);
		}
		input += stride;
		dx += stride;
	}
}
static void CompVGradientFastGradX_8u16s_C(const uint8_t* input, int16_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVGradientFastGradX_C<uint8_t, int16_t>(input, dx, width, height, stride);
}
static void CompVGradientFastGradX_8u32f_C(const uint8_t* input, compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVGradientFastGradX_C<uint8_t, compv_float32_t>(input, dx, width, height, stride);
}
static void CompVGradientFastGradX_32f32f_C(const compv_float32_t* input, compv_float32_t* dx, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVGradientFastGradX_C<compv_float32_t, compv_float32_t>(input, dx, width, height, stride);
}


template<typename InType, typename OutType>
static void CompVGradientFastGradY_C(const InType* input, OutType* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	const InType* inputPlus1 = input + stride;
	const InType* inputMinus1 = input - stride;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			dy[i] = static_cast<OutType>(inputPlus1[i] - inputMinus1[i]);
		}
		inputPlus1 += stride;
		inputMinus1 += stride;
		dy += stride;
	}
}
static void CompVGradientFastGradY_8u16s_C(const uint8_t* input, int16_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVGradientFastGradY_C<uint8_t, int16_t>(input, dy, width, height, stride);
}
static void CompVGradientFastGradY_8u32f_C(const uint8_t* input, compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVGradientFastGradY_C<uint8_t, compv_float32_t>(input, dy, width, height, stride);
}
static void CompVGradientFastGradY_32f32f_C(const compv_float32_t* input, compv_float32_t* dy, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVGradientFastGradY_C<compv_float32_t, compv_float32_t>(input, dy, width, height, stride);
}

COMPV_NAMESPACE_END()
