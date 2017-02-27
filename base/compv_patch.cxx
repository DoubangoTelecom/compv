/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_patch.h"
#include "compv/base/compv_mem.h"
#include "compv/base/math/compv_math_utils.h"

#include "compv/base/intrin/x86/compv_patch_intrin_sse2.h"
#include "compv/base/intrin/x86/compv_patch_intrin_avx2.h"
#include "compv/base/intrin/arm/compv_patch_intrin_neon.h"

COMPV_NAMESPACE_BEGIN()

#if 0
static void Moments0110_C(COMPV_ALIGNED(DEFAULT) const uint8_t* top, COMPV_ALIGNED(DEFAULT)const uint8_t* bottom, COMPV_ALIGNED(DEFAULT)const int16_t* x, COMPV_ALIGNED(DEFAULT) const int16_t* y, compv_uscalar_t count, compv_uscalar_t* s01, compv_uscalar_t* s10);
#endif

#if COMPV_ASM
#	if COMPV_ARCH_X86
	COMPV_EXTERNC void CompVPatchRadiusLte64Moments0110_Asm_X86_SSE2(COMPV_ALIGNED(SSE) const uint8_t* top, COMPV_ALIGNED(SSE) const uint8_t* bottom, COMPV_ALIGNED(SSE) const int16_t* x, COMPV_ALIGNED(SSE) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
	COMPV_EXTERNC void CompVPatchRadiusLte64Moments0110_Asm_X86_AVX2(COMPV_ALIGNED(AVX) const uint8_t* top, COMPV_ALIGNED(AVX) const uint8_t* bottom, COMPV_ALIGNED(AVX) const int16_t* x, COMPV_ALIGNED(AVX) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
#	endif /* COMPV_ARCH_X86 */
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVPatchRadiusLte64Moments0110_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* top, COMPV_ALIGNED(SSE) const uint8_t* bottom, COMPV_ALIGNED(SSE) const int16_t* x, COMPV_ALIGNED(SSE) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
	COMPV_EXTERNC void CompVPatchRadiusLte64Moments0110_Asm_X64_AVX2(COMPV_ALIGNED(AVX) const uint8_t* top, COMPV_ALIGNED(AVX) const uint8_t* bottom, COMPV_ALIGNED(AVX) const int16_t* x, COMPV_ALIGNED(AVX) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
#	endif /* COMPV_ARCH_X64 */
#	if COMPV_ARCH_ARM32
    COMPV_EXTERNC void CompVPatchMoments0110_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* top, COMPV_ALIGNED(NEON) const uint8_t* bottom, COMPV_ALIGNED(NEON) const int16_t* x, COMPV_ALIGNED(NEON) const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
#	endif /* COMPV_ARCH_X64 */
#endif /* COMPV_ASM */

CompVPatch::CompVPatch()
	: m_pMaxAbscissas(NULL)
	, m_pX(NULL)
	, m_pY(NULL)
	, m_pTop(NULL)
	, m_pBottom(NULL)
	, m_Moments0110(NULL)
{

}

CompVPatch::~CompVPatch()
{
	CompVMem::free(reinterpret_cast<void**>(&m_pMaxAbscissas));
	CompVMem::free(reinterpret_cast<void**>(&m_pX));
	CompVMem::free(reinterpret_cast<void**>(&m_pY));
	CompVMem::free(reinterpret_cast<void**>(&m_pTop));
	CompVMem::free(reinterpret_cast<void**>(&m_pBottom));
}

COMPV_ERROR_CODE CompVPatch::moments0110(const uint8_t* ptr, int center_x, int center_y, size_t img_width_, size_t img_height_, size_t img_stride_, int* m01, int* m10)
{
	int i, j, img_width = static_cast<int>(img_width_), img_stride = static_cast<int>(img_stride_), img_height = static_cast<int>(img_height_);
	bool closeToBorder = (center_x < m_nRadius || (center_x + m_nRadius) >= img_width || (center_y < m_nRadius) || (center_y + m_nRadius) >= img_height);

	if (closeToBorder) {
		const uint8_t* img_ptr;
		int s01 = 0, s10 = 0;
		int img_y, i, j, minI, maxI, minJ, maxJ, dX;
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("Code never called be used for interop. against OpenCV"); // Code never called be used for interop. against OpenCV
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found."); // It's useless to compute moments for these points because it won't be possible to have a description (partial circle)
		// This code must never be called as we remove elements close to the border before computing the moments
		// Compute minJ and maxJ
		minJ = -m_nRadius;
		if ((center_y + minJ) < 0) {
			minJ = -center_y;
		}
		maxJ = +m_nRadius;
		if ((center_y + maxJ) >= img_height) {
			maxJ = (img_height - center_y - 1);
			maxJ = COMPV_MATH_CLIP3(minJ, m_nRadius, maxJ);
		}

		for (j = minJ, img_y = (center_y + j); j <= maxJ; ++j, ++img_y) {
			// Pythagorean theorem: x = sqrt(r**2 - y**2)
			// dX = ((int)sqrt(patch_radius_pow2 - (j * j)));
			dX = m_pMaxAbscissas[j < 0 ? -j : +j];

			// Compute minI and maxI
			minI = -dX;
			if ((center_x + minI) < 0) {
				minI = -center_x;
			}
			maxI = +dX;
			if ((center_x + maxI) >= img_width) {
				maxI = (img_width - center_x - 1);
				maxI = COMPV_MATH_CLIP3(minI, dX, maxI);
			}

			img_ptr = &ptr[(img_y * img_stride) + (center_x + minI)];
			for (i = minI; i <= maxI; ++i, ++img_ptr) {
				s10 += (i **img_ptr); // i^p * j^q * I(x, y) = i^1 * j^0 * I(x, y) = i * I(x, y)
				s01 += j **img_ptr; // i^p * j^q * I(x, y) = i^0 * j^1 * I(x, y) = j * I(x, y)
			}
		}
		*m01 = (int)s01;
		*m10 = (int)s10;
	}
	else {
		const uint8_t *img_center, *img_top, *img_bottom, *t, *b;
		uint8_t top, bottom;
		compv_scalar_t s10 = 0, s01 = 0;
		const int16_t *dX;

		img_center = &ptr[(center_y * img_stride) + center_x];

		/* Handle 'j==0' case */
		{
			for (i = -m_nRadius; i <= +m_nRadius; ++i) {
				s10 += (i * img_center[i]);
			}
		}
		/* Handle 'j==patch_radius' case */
		{
			top = *(img_center + (img_stride * m_nRadius));
			bottom = *(img_center - (img_stride * m_nRadius));
			s01 += (m_nRadius * top) - (m_nRadius * bottom);
		}

		img_top = img_center + img_stride;
		img_bottom = img_center - img_stride;

		if (m_Moments0110) {
			uint8_t *t_ = m_pTop, *b_ = m_pBottom;
			for (j = 1, dX = &m_pMaxAbscissas[j]; j < m_nRadius; ++j, ++dX) {
				// TODO(dmi): next code not optimized
				for (i = -*dX, t = &img_top[i], b = &img_bottom[i]; i <= *dX - 8; i += 8, t += 8, b += 8, t_ += 8, b_ += 8) {
					*reinterpret_cast<uint64_t*>(t_) = *reinterpret_cast<const uint64_t*>(t), *reinterpret_cast<uint64_t*>(b_) = *reinterpret_cast<const uint64_t*>(b);
				}
				for (; i <= *dX - 4; i += 4, t += 4, b += 4, t_ += 4, b_ += 4) {
					*reinterpret_cast<uint32_t*>(t_) = *reinterpret_cast<const uint32_t*>(t), *reinterpret_cast<uint32_t*>(b_) = *reinterpret_cast<const uint32_t*>(b);
				}
				for (; i <= *dX; ++i, ++t, ++b, ++t_, ++b_) {
					*t_ = *t, *b_ = *b;
				}
				img_top += img_stride;
				img_bottom -= img_stride;
			}
			m_Moments0110(m_pTop, m_pBottom, m_pX, m_pY, m_nCount, &s01, &s10);
		}
		else {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
			int sj;
			// Handle j=1... cases
			for (j = 1, dX = &m_pMaxAbscissas[j]; j < m_nRadius; ++j, ++dX) {
				sj = 0;
				for (i = -*dX, t = &img_top[i], b = &img_bottom[i]; i <= *dX; ++i, ++t, ++b) {
					s10 += i * (*t + *b); // (i * t) + (i * b)
					sj += (*t - *b);
				}
				s01 += j * sj; // for SIMD: move inside the loop and s01 = (j * t) - (j * b) = j * (t - b)
				img_top += img_stride;
				img_bottom -= img_stride;
			}
		}
		*m01 = static_cast<int>(s01);
		*m10 = static_cast<int>(s10);
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVPatch::newObj(CompVPatchPtrPtr patch, int diameter)
{
	COMPV_CHECK_EXP_RETURN(!patch || diameter < 2, COMPV_ERROR_CODE_E_INVALID_PARAMETER); // (diameter > 128) check for Q16 math, max(radius) is 64
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	int radius_ = diameter >> 1, radius2_ = radius_ * radius_;
	int i, j, k;
	const int16_t* dX;
	int16_t* pMaxAbscissas_ = NULL, *pX_ = NULL, *pY_ = NULL;
	uint8_t *pTop_ = NULL, *p_Bottom_ = NULL;
	size_t count_ = 0, stride_ = 0;
	CompVPatchPtr patch_;
	void(*Moments0110_)(const uint8_t* top, const uint8_t* bottom, const int16_t* x, const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10) 
		= NULL; // must not be C version which is slooow and used model for SIMD implementations
	// radius_ <= 64 is required to make sure (radius * (top +- bottom)) is € [-0x7fff, +0x7fff]
	// this is a condition to allow epi16 mullo without overflow
	
#if COMPV_ARCH_X86
	if (radius_ <= 64) {
		if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(Moments0110_ = CompVPatchRadiusLte64Moments0110_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X86(Moments0110_ = CompVPatchRadiusLte64Moments0110_Asm_X86_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(Moments0110_ = CompVPatchRadiusLte64Moments0110_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(Moments0110_ = CompVPatchRadiusLte64Moments0110_Intrin_AVX2);
			COMPV_EXEC_IFDEF_ASM_X86(Moments0110_ = CompVPatchRadiusLte64Moments0110_Asm_X86_AVX2);
			COMPV_EXEC_IFDEF_ASM_X64(Moments0110_ = CompVPatchRadiusLte64Moments0110_Asm_X64_AVX2);
		}
	}
#elif COMPV_ARCH_ARM
	// No restriction on radius for ARM archs
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(Moments0110_ = CompVPatchMoments0110_Intrin_NEON);
        COMPV_EXEC_IFDEF_ASM_ARM32(Moments0110_ = CompVPatchMoments0110_Asm_NEON32);
	}
#endif

	pMaxAbscissas_ = reinterpret_cast<int16_t*>(CompVMem::malloc((radius_ + 1) * sizeof(int16_t)));
	COMPV_CHECK_EXP_BAIL(!pMaxAbscissas_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	for (int i = 0; i <= radius_; ++i) {
		// Pythagorean theorem: x = sqrt(r**2 - y**2)
		pMaxAbscissas_[i] = static_cast<int16_t>(sqrt(radius2_ - (i * i)));
	}

	if (Moments0110_) {
		// Count
		for (j = 1, dX = &pMaxAbscissas_[j]; j < radius_; ++j, ++dX) {
			for (i = -*dX; i <= *dX; ++i) {
				++count_;
			}
		}
		stride_ = CompVMem::alignForward(count_, COMPV_ALIGNV_SIMD_DEFAULT);

		pX_ = reinterpret_cast<int16_t*>(CompVMem::calloc(stride_, sizeof(int16_t)));
		COMPV_CHECK_EXP_BAIL(!pX_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
		pY_ = reinterpret_cast<int16_t*>(CompVMem::calloc(stride_, sizeof(int16_t)));
		COMPV_CHECK_EXP_BAIL(!pY_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
		pTop_ = reinterpret_cast<uint8_t*>(CompVMem::calloc(stride_, sizeof(uint8_t)));
		COMPV_CHECK_EXP_BAIL(!pTop_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
		p_Bottom_ = reinterpret_cast<uint8_t*>(CompVMem::calloc(stride_, sizeof(uint8_t)));
		COMPV_CHECK_EXP_BAIL(!p_Bottom_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

		// Set X and Y values
		k = 0;
		for (j = 1, dX = &pMaxAbscissas_[j]; j < radius_; ++j, ++dX) {
			for (i = -*dX; i <= *dX; ++i) {
				pX_[k] = i;
				pY_[k] = j;
				++k;
			}
		}
	}

	patch_ = new CompVPatch();
	COMPV_CHECK_EXP_BAIL(!patch_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	patch_->m_nRadius = radius_;
	patch_->m_pMaxAbscissas = pMaxAbscissas_;
	patch_->m_pX = pX_;
	patch_->m_pY = pY_;
	patch_->m_pTop = pTop_;
	patch_->m_pBottom = p_Bottom_;
	patch_->m_nCount = count_;
	patch_->m_nStride = stride_;
	patch_->m_Moments0110 = Moments0110_;

	*patch = patch_;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		CompVMem::free(reinterpret_cast<void**>(&pMaxAbscissas_));
		CompVMem::free(reinterpret_cast<void**>(&pX_));
		CompVMem::free(reinterpret_cast<void**>(&pY_));
		CompVMem::free(reinterpret_cast<void**>(&pTop_));
		CompVMem::free(reinterpret_cast<void**>(&p_Bottom_));
	}
	return err_;
}

#if 0
// top, bottom, x, y are allocated with padding which means you can read up to align_fwd(count, alignv)
static void Moments0110_C(COMPV_ALIGNED(DEFAULT) const uint8_t* top, COMPV_ALIGNED(DEFAULT)const uint8_t* bottom, COMPV_ALIGNED(DEFAULT)const int16_t* x, COMPV_ALIGNED(DEFAULT) const int16_t* y, compv_uscalar_t count, compv_uscalar_t* s01, compv_uscalar_t* s10)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
	COMPV_DEBUG_INFO_CODE_FOR_TESTING(); // For testing only

	compv_scalar_t s10_ = *s10;
	compv_scalar_t s01_ = *s01;

	for (compv_scalar_t i = 0; i < count; ++i, ++top, ++bottom, ++x, ++y) {
		s10_ += *x * (*top + *bottom);
		s01_ += *y * (*top - *bottom);
	}
	*s10 = s10_;
	*s01 = s01_;
}
#endif

COMPV_NAMESPACE_END()
