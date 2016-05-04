/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/compv_convlt.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_math.h"
#include "compv/compv_cpu.h"
#include "compv/compv_debug.h"

#include "compv/intrinsics/x86/compv_convlt_intrin_sse.h"
#include "compv/intrinsics/x86/compv_convlt_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void Convlt1_hz_float32_minpack4_Asm_X86_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t pad, const float* hkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_hz_float32_minpack16_Asm_X86_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t pad, const float* hkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_hz_float32_minpack16_Asm_X86_FMA3_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t pad, const float* hkern_ptr, compv::compv_scalar_t kern_size);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void Convlt1_hz_float32_minpack4_Asm_X64_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t pad, const float* hkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_hz_float32_minpack16_Asm_X64_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t pad, const float* hkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_hz_float32_minpack16_Asm_X64_FMA3_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t pad, const float* hkern_ptr, compv::compv_scalar_t kern_size);
#endif /* COMPV_ARCH_X64 && COMPV_ASM */

COMPV_NAMESPACE_BEGIN()

template class CompVConvlt<double >;
template class CompVConvlt<float >;

template<class T>
CompVConvlt<T>::CompVConvlt()
    : m_pDataPtr(NULL)
    , m_pDataPtr0(NULL)
    , m_nDataSize(0)
    , m_nDataSize0(0)
	, m_pResultPtr(NULL)
	, m_nResultSize(0)
{

}

template<class T>
CompVConvlt<T>::~CompVConvlt()
{
    CompVMem::free((void**)&m_pDataPtr);
    CompVMem::free((void**)&m_pDataPtr0);
}

template<class T>
COMPV_ERROR_CODE CompVConvlt<T>::convlt2(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const double* kern_ptr, int kern_size, uint8_t* out_ptr /*= NULL*/, int img_border /*= 0*/)
{
	// Multi-threading and SIMD acceleration
	// Check if the kernel is separable (matrice rank = 1) and use convlt1 instead
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

    // Check inputs
    COMPV_CHECK_EXP_RETURN(!img_ptr || !img_width || (img_stride < img_width) || !kern_ptr || !(kern_size & 1) || img_border < 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	
	// Make sure we're not sharing the internal memory across threads
	CompVPtr<CompVThreadDispatcher* >threadDip = CompVEngine::getThreadDispatcher();
	if (!out_ptr && threadDip && threadDip->isMotherOfTheCurrentThread()) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_CALL);
	}
	bool bUseInternalMemory = !out_ptr;

    // Alloc memory
    size_t neededSize = (img_height + (img_border << 1)) * (img_stride + (img_border << 1));
	if (bUseInternalMemory && m_nDataSize < neededSize) {
        m_pDataPtr = CompVMem::realloc(m_pDataPtr, neededSize);
        if (!m_pDataPtr) {
            m_nDataSize = 0;
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        }
        m_nDataSize = neededSize;
    }

    // Init variables
	uint8_t* outImg = (bUseInternalMemory ? ((uint8_t*)m_pDataPtr) : out_ptr) + (img_border * img_stride) + img_border;
    const uint8_t *topleft, *img_ptr_;
    double sum;
    const double *ker_ptr;
    int imgpad, i, j, row, col;
    int ker_size_div2 = kern_size >> 1;
    int start_margin = (img_border >= ker_size_div2) ? -ker_size_div2 : -img_border;
    int start_center = start_margin + ker_size_div2;
    img_ptr_ = img_ptr + (start_margin * img_stride) + start_margin;
    imgpad = (img_stride - img_width) + start_center + start_center;

    // Process
    for (j = start_center; j < img_height - start_center; ++j) {
        for (i = start_center; i < img_width - start_center; ++i) {
            sum = 0;
            topleft = img_ptr_;
            ker_ptr = kern_ptr;
            for (row = 0; row < kern_size; ++row) {
                for (col = 0; col < kern_size; ++col) {
                    sum += topleft[col] * ker_ptr[col];
                }
                ker_ptr += kern_size;
                topleft += img_stride;
            }
            outImg[(j * img_stride) + i] = (uint8_t)sum;  // TODO(dmi): do not mul() but add()
            ++img_ptr_;
        }
        img_ptr_ += imgpad;
    }

	if (bUseInternalMemory && out_ptr) {
		CompVMem::copy(out_ptr, outImg, neededSize);
	}
	m_pResultPtr = (const void*)outImg;
	m_nResultSize = neededSize;

    return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVConvlt<T>::convlt1(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const T* vkern_ptr, const T* hkern_ptr, int kern_size, uint8_t* out_ptr /*= NULL*/, int img_border /*= 0*/)
{
    // Check inputs
	COMPV_CHECK_EXP_RETURN(!img_ptr || (img_width < kern_size * 2) || (img_height < kern_size * 2) || (img_stride < img_width) || !vkern_ptr || !hkern_ptr || img_border < 0 || !(kern_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Make sure we're not sharing the internal memory across threads
	CompVPtr<CompVThreadDispatcher* >threadDip = CompVEngine::getThreadDispatcher();
	if (!out_ptr && threadDip && threadDip->isMotherOfTheCurrentThread()) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_CALL);
	}
	bool bUseInternalMemory = !out_ptr;

    // Alloc memory
    size_t neededSize = (img_height + (img_border << 1)) * (img_stride + (img_border << 1));
	if (bUseInternalMemory && m_nDataSize < neededSize) {
        m_pDataPtr = CompVMem::realloc(m_pDataPtr, neededSize);
        if (!m_pDataPtr) {
            m_nDataSize = 0;
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        }
        m_nDataSize = neededSize;
    }
    if (m_nDataSize0 < neededSize) {
        m_pDataPtr0 = CompVMem::realloc(m_pDataPtr0, neededSize);
        if (!m_pDataPtr0) {
            m_nDataSize0 = 0;
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        }
        m_nDataSize0 = neededSize;
    }

    uint8_t *imgTmp, *imgOut, *imgPtr;
    const uint8_t *topleft;
    int imgpad;
    int ker_size_div2 = kern_size >> 1;
    int start_margin = (img_border >= ker_size_div2) ? -ker_size_div2 : -img_border;
    int start_center = start_margin + ker_size_div2;

	imgTmp = ((uint8_t*)m_pDataPtr0) + (img_border * img_stride) + img_border;
	imgOut = (bUseInternalMemory ? ((uint8_t*)m_pDataPtr) : out_ptr) + (img_border * img_stride) + img_border;

    // Process

    // Horizontal
    topleft = img_ptr + start_margin;
    imgpad = (img_stride - img_width) + start_center + start_center;
	imgPtr = imgTmp + start_center;
#if 1
	CompVConvlt::convlt1_hz(topleft, imgPtr, (img_width - start_center - start_center), img_height, imgpad, hkern_ptr, kern_size);
#else
	{
		COMPV_DEBUG_INFO_CODE_FOR_TESTING();
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
		int col, i, j;
		double sum;
		for (j = 0; j < img_height; ++j) {
			for (i = start_center; i < img_width - start_center; ++i) {
				sum = 0;
				for (col = 0; col < kern_size; ++col) {
					sum += topleft[col] * hkern_ptr[col];
				}
				*imgPtr = (uint8_t)sum; // TODO(dmi): do not mul() but add()
				++topleft;
				++imgPtr;
			}
			topleft += imgpad;
			imgPtr += imgpad;
		}
	}
#endif

    // Vertical
    topleft = imgTmp + (start_margin * img_stride); // output from hz filtering is now used as input
    imgpad = (img_stride - img_width);
	imgPtr = imgOut + (start_center * img_stride);
#if 1
	CompVConvlt::convlt1_vert(topleft, imgPtr, img_width, img_height - start_center - start_center, img_stride, imgpad, vkern_ptr, kern_size);
#else
	{
		COMPV_DEBUG_INFO_CODE_FOR_TESTING();
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
		int row, i, j;
		const uint8_t *ptr_;
		double sum;
		for (j = start_center; j < img_height - start_center; ++j) {
			for (i = 0; i < img_width; ++i) {
				sum = 0;
				ptr_ = topleft;
				for (row = 0; row < kern_size; ++row, ptr_ += img_stride) {
					sum += *ptr_ * vkern_ptr[row];
				}
				*imgPtr = (uint8_t)sum; // TODO(dmi): do not mul() but add()
				++topleft;
				++imgPtr;
			}
			topleft += imgpad;
			imgPtr += imgpad;
		}
	}
#endif

	if (bUseInternalMemory && out_ptr) {
		CompVMem::copy(out_ptr, imgOut, neededSize);
	}
	m_pResultPtr = (const void*)imgOut;
	m_nResultSize = neededSize;

    return COMPV_ERROR_CODE_S_OK;
}

// Private function: do not check input parameters
template <typename T>
void CompVConvlt<T>::convlt1_hz(const uint8_t* in_ptr, uint8_t* out_ptr, int width, int height, int pad, const T* hkern_ptr, int kern_size)
{
	static const bool size_of_float_is4 = (sizeof(float) == 4); // ASM and INTRIN code require it
	int minpack = 0; // Minimum number of pixels the function can handle for each operation (must be pof 2)
	
	// Floating point implementation
	if (std::is_same<T, float>::value && size_of_float_is4) {
		void(*Convlt1_hz_float32)(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size) = NULL;
		if (width > 3 && CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86((Convlt1_hz_float32 = Convlt1_hz_float32_minpack4_Intrin_SSE2, minpack = 4));
			// Accroding to Intel VTune the intrinsic version (VS2013) is faster: Check ASM generated by VS2013 and we have better one
			// This is not the case for AVX: Our ASM code is faster than what VS2013 genereate
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
			COMPV_EXEC_IFDEF_ASM_X86((Convlt1_hz_float32 = Convlt1_hz_float32_minpack4_Asm_X86_SSE2, minpack = 4));
			COMPV_EXEC_IFDEF_ASM_X64((Convlt1_hz_float32 = Convlt1_hz_float32_minpack4_Asm_X64_SSE2, minpack = 4));
		}
		if (width > 15 && CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
			COMPV_EXEC_IFDEF_INTRIN_X86((Convlt1_hz_float32 = Convlt1_hz_float32_minpack16_Intrin_AVX2, minpack = 16));
			COMPV_EXEC_IFDEF_ASM_X86((Convlt1_hz_float32 = Convlt1_hz_float32_minpack16_Asm_X86_AVX2, minpack = 16));
			COMPV_EXEC_IFDEF_ASM_X64((Convlt1_hz_float32 = Convlt1_hz_float32_minpack16_Asm_X64_AVX2, minpack = 16));
			if (CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
				 COMPV_EXEC_IFDEF_ASM_X86((Convlt1_hz_float32 = Convlt1_hz_float32_minpack16_Asm_X86_FMA3_AVX2, minpack = 16));
				 COMPV_EXEC_IFDEF_ASM_X64((Convlt1_hz_float32 = Convlt1_hz_float32_minpack16_Asm_X64_FMA3_AVX2, minpack = 16));
			}
		}
		if (Convlt1_hz_float32) {
			// TODO(dmi): execute AVX_minpack16 then, SSE_minpack4 for the remaining > 15 pixels
			Convlt1_hz_float32(in_ptr, out_ptr, width, height, pad, (const float*)hkern_ptr, kern_size);
		}
	}

	// Check missed pixels
	if (minpack > 0) {
		int missed = (width & (minpack - 1));
		if (missed == 0) {
			return;
		}
		in_ptr += (width - missed);
		out_ptr += (width - missed);
		pad += (width - missed);
		width = missed;
	}
	else {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	}
	
	{
		int i, j, col;
		T sum;

		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				sum = 0;
				for (col = 0; col < kern_size; ++col) {
					sum += in_ptr[col] * hkern_ptr[col];
				}
				*out_ptr = COMPV_MATH_ROUNDFU_2_INT(sum, uint8_t);
				++in_ptr;
				++out_ptr;
			}
			in_ptr += pad;
			out_ptr += pad;
		}
	}
}

template <typename T>
void CompVConvlt<T>::convlt1_vert(const uint8_t* in_ptr, uint8_t* out_ptr, int width, int height, int stride, int pad, const T* vkern_ptr, int kern_size)
{
	int i, j, row;
	T sum;
	const uint8_t *ptr_;

	for (j = 0; j < height; ++j) {
		for (i = 0; i < width; ++i) {
			sum = 0;
			ptr_ = in_ptr;
			for (row = 0; row < kern_size; ++row) {
				sum += *ptr_ * vkern_ptr[row];
				ptr_ += stride;
			}
			*out_ptr = (uint8_t)sum; // TODO(dmi): do not mul() but add()
			++in_ptr;
			++out_ptr;
		}
		in_ptr += pad;
		out_ptr += pad;
	}
}

template<class T>
COMPV_ERROR_CODE CompVConvlt<T>::newObj(CompVPtr<CompVConvlt* >* convlt)
{
    COMPV_CHECK_EXP_RETURN(!convlt, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVConvlt*> convlt_ = NULL;

    convlt_ = new CompVConvlt();
    COMPV_CHECK_EXP_RETURN(!convlt, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    *convlt = convlt_;

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
