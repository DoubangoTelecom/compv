/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_convlt.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/math/compv_math.h"
#include "compv/compv_cpu.h"
#include "compv/compv_debug.h"

#include "compv/intrinsics/x86/compv_convlt_intrin_sse2.h"
#include "compv/intrinsics/x86/compv_convlt_intrin_avx2.h"

#define COMPV_CONVOLUTION_MIN_SAMPLES_PER_THREAD (10*10)

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void Convlt1_verthz_float32_minpack4_Asm_X86_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const float* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_float32_minpack16_Asm_X86_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const float* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_float32_minpack16_Asm_X86_FMA3_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const float* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_fxpq16_minpack4_Asm_X86_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const uint16_t* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_fxpq16_minpack16_Asm_X86_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const uint16_t* vhkern_ptr, compv::compv_scalar_t kern_size);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void Convlt1_verthz_float32_minpack4_Asm_X64_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const float* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_float32_minpack16_Asm_X64_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const float* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_float32_minpack16_Asm_X64_FMA3_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const float* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_fxpq16_minpack4_Asm_X64_SSE2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const uint16_t* vhkern_ptr, compv::compv_scalar_t kern_size);
COMPV_EXTERNC void Convlt1_verthz_fxpq16_minpack16_Asm_X64_AVX2(const uint8_t* in_ptr, uint8_t* out_ptr, compv::compv_scalar_t width, compv::compv_scalar_t height, compv::compv_scalar_t stride, compv::compv_scalar_t pad, const uint16_t* vhkern_ptr, compv::compv_scalar_t kern_size);
#endif /* COMPV_ARCH_X64 && COMPV_ASM */

COMPV_NAMESPACE_BEGIN()

template class CompVConvlt<uint16_t >; // for fixed points only (to be sued when kernel values are positive)
template class CompVConvlt<int16_t >;
template class CompVConvlt<compv_float64_t >;
template class CompVConvlt<compv_float32_t >;


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
    CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
    if (!out_ptr && threadDisp && threadDisp->isMotherOfTheCurrentThread()) {
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

// if "out_ptr" is defined, its size must be at least (img_height * img_stride) + borders
template<class T>
COMPV_ERROR_CODE CompVConvlt<T>::convlt1(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const T* vkern_ptr, const T* hkern_ptr, int kern_size, uint8_t* out_ptr /*= NULL*/, int img_border /*= 0*/)
{
    return convlt1_private(img_ptr, img_width, img_stride, img_height, vkern_ptr, hkern_ptr, kern_size, out_ptr, img_border);
}

// if "out_ptr" is defined, its size must be at least (img_height * img_stride) + borders
template <class T>
COMPV_ERROR_CODE CompVConvlt<T>::convlt1_fxp(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const uint16_t* vkern_ptr, const uint16_t* hkern_ptr, int kern_size, uint8_t* out_ptr /*= NULL*/, int img_border /*= 0*/)
{
    return convlt1_private(img_ptr, img_width, img_stride, img_height, (const T*)vkern_ptr, (const T*)hkern_ptr, kern_size, out_ptr, img_border, true);
}

template <typename T>
COMPV_ERROR_CODE CompVConvlt<T>::convlt1_private(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const T* vkern_ptr, const T* hkern_ptr, int kern_size, uint8_t* out_ptr, int img_border, bool bFxp /*= false*/)
{
    // Check inputs
    COMPV_CHECK_EXP_RETURN(!img_ptr || (img_width < kern_size * 2) || (img_height < kern_size * 2) || (img_stride < img_width) || !vkern_ptr || !hkern_ptr || img_border < 0 || !(kern_size & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
    bool bMultiThreaded = threadDisp && threadDisp->isMotherOfTheCurrentThread();

    // Make sure we're not sharing the internal memory across threads
    COMPV_CHECK_EXP_RETURN(bMultiThreaded && !out_ptr, COMPV_ERROR_CODE_E_INVALID_CALL);

    // The realloc_aligned() implementation memcpy() old data which is slow. Prefer, free_aligned() followed by malloc_aligned()

    /* Alloc memory */
    size_t neededSize = (img_height + (img_border << 1)) * (img_stride + (img_border << 1));
    if (!out_ptr && m_nDataSize < neededSize) {
        CompVMem::free(&m_pDataPtr);
        m_pDataPtr = CompVMem::malloc(neededSize);
        if (!m_pDataPtr) {
            m_nDataSize = 0;
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        }
        m_nDataSize = neededSize;
    }
    // Allocate tmp memory if multithread to avoid sharing 'm_pDataPtr0'
    void* imgTmpMT = NULL;
    if (bMultiThreaded) {
        imgTmpMT = CompVMem::malloc(neededSize);
        COMPV_CHECK_EXP_RETURN(!imgTmpMT, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    if (!imgTmpMT && m_nDataSize0 < neededSize) {
        CompVMem::free(&m_pDataPtr0);
        m_pDataPtr0 = CompVMem::malloc(neededSize);
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

    imgTmp = (!imgTmpMT ? (uint8_t*)m_pDataPtr0 : (uint8_t*)imgTmpMT) + (img_border * img_stride) + img_border;
    imgOut = (!out_ptr ? ((uint8_t*)m_pDataPtr) : out_ptr) + (img_border * img_stride) + img_border;

    // Horizontal
    topleft = img_ptr + start_margin;
    imgpad = (img_stride - img_width) + start_center + start_center;
    imgPtr = imgTmp + start_center;
    if (bFxp) {
        CompVConvlt::convlt1_verthz_fxp(topleft, imgPtr, (img_width - start_center - start_center), img_height, /*stride*/1, imgpad, (const uint16_t*)hkern_ptr, kern_size);
    }
    else {
        CompVConvlt::convlt1_verthz(topleft, imgPtr, (img_width - start_center - start_center), img_height, /*stride*/1, imgpad, hkern_ptr, kern_size);
    }

    // Vertical
    topleft = imgTmp + (start_margin * img_stride); // output from hz filtering is now used as input
    imgpad = (img_stride - img_width);
    imgPtr = imgOut + (start_center * img_stride);
    if (bFxp) {
        CompVConvlt::convlt1_verthz_fxp(topleft, imgPtr, img_width, img_height - start_center - start_center, img_stride, imgpad, (const uint16_t*)vkern_ptr, kern_size);
    }
    else {
        CompVConvlt::convlt1_verthz(topleft, imgPtr, img_width, img_height - start_center - start_center, img_stride, imgpad, vkern_ptr, kern_size);
    }

    m_pResultPtr = (const void*)imgOut;
    m_nResultSize = neededSize;

    CompVMem::free(&imgTmpMT);

    return COMPV_ERROR_CODE_S_OK;
}

// Private function: do not check input parameters
// Use stride = 1 for "hz"
template <typename T>
void CompVConvlt<T>::convlt1_verthz(const uint8_t* in_ptr, uint8_t* out_ptr, int width, int height, int stride, int pad, const T* vhkern_ptr, int kern_size)
{
    int minpack = 0; // Minimum number of pixels the function can handle for each operation (must be pof 2)
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Not multithreaded (See FXP version)

    // Floating point implementation
    if (std::is_same<T, compv_float32_t>::value) {
        void(*Convlt1_verthz_float32)(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const float* hkern_ptr, compv_scalar_t kern_size) = NULL;
        if (width > 3 && CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
            COMPV_EXEC_IFDEF_INTRIN_X86((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack4_Intrin_SSE2, minpack = 4));
            // TODO(dmi): Accroding to Intel VTune the intrinsic version (VS2013) is faster: Check ASM generated by VS2013 and we have better one
            // This is not the case for AVX: Our ASM code is faster than what VS2013 genereate
            COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
            COMPV_EXEC_IFDEF_ASM_X86((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack4_Asm_X86_SSE2, minpack = 4));
            COMPV_EXEC_IFDEF_ASM_X64((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack4_Asm_X64_SSE2, minpack = 4));
        }
        if (width > 15 && CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
            COMPV_EXEC_IFDEF_INTRIN_X86((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack16_Intrin_AVX2, minpack = 16));
            COMPV_EXEC_IFDEF_ASM_X86((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack16_Asm_X86_AVX2, minpack = 16));
            COMPV_EXEC_IFDEF_ASM_X64((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack16_Asm_X64_AVX2, minpack = 16));
            if (CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
                COMPV_EXEC_IFDEF_ASM_X86((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack16_Asm_X86_FMA3_AVX2, minpack = 16));
                COMPV_EXEC_IFDEF_ASM_X64((Convlt1_verthz_float32 = Convlt1_verthz_float32_minpack16_Asm_X64_FMA3_AVX2, minpack = 16));
            }
        }
        if (Convlt1_verthz_float32) {
            // TODO(dmi): execute AVX_minpack16 then, SSE_minpack4 for the remaining > 15 pixels
            Convlt1_verthz_float32(in_ptr, out_ptr, width, height, stride, pad, (const float*)vhkern_ptr, kern_size);
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

    CompVConvlt<T>::convlt1_verthz_C(in_ptr, out_ptr, width, height, stride, pad, vhkern_ptr, kern_size);
}

// ARM: maxVal(vhkern_ptr) = 0x7fff, FXPQ(vhkern_ptr) = 15
// X86: maxVal(vhkern_ptr) = 0xffff, FXPQ(vhkern_ptr) = 16
template <typename T>
void CompVConvlt<T>::convlt1_verthz_fxp(const uint8_t* in_ptr, uint8_t* out_ptr, int width, int height, int stride, int pad, const uint16_t* vhkern_ptr, int kern_size)
{
    int minpack = 0; // Minimum number of pixels the function can handle for each operation (must be pof 2)

    // ARM-NEON: cast uint16_t to int16_t
    // Up to the caller to make sure that "vhkern_ptr" values are equal to (x * (1<<COMPV_FXPQ)), x within [0.f, 1.f]

    // Unlike the floating-point implementation our ASM code is by far faster than what VS2013 generate

    void(*Convlt1_verthz_fxp)(const uint8_t* in_ptr, uint8_t* out_ptr, compv_scalar_t width, compv_scalar_t height, compv_scalar_t stride, compv_scalar_t pad, const uint16_t* hkern_ptr, compv_scalar_t kern_size) = NULL;
    if (width > 3 && CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
        COMPV_EXEC_IFDEF_INTRIN_X86((Convlt1_verthz_fxp = Convlt1_verthz_fxpq16_minpack4_Intrin_SSE2, minpack = 4));
        COMPV_EXEC_IFDEF_ASM_X86((Convlt1_verthz_fxp = Convlt1_verthz_fxpq16_minpack4_Asm_X86_SSE2, minpack = 4));
        COMPV_EXEC_IFDEF_ASM_X64((Convlt1_verthz_fxp = Convlt1_verthz_fxpq16_minpack4_Asm_X64_SSE2, minpack = 4));
    }
    if (width > 15 && CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
        COMPV_EXEC_IFDEF_INTRIN_X86((Convlt1_verthz_fxp = Convlt1_verthz_fxpq16_minpack16_Intrin_AVX2, minpack = 16));
        COMPV_EXEC_IFDEF_ASM_X86((Convlt1_verthz_fxp = Convlt1_verthz_fxpq16_minpack16_Asm_X86_AVX2, minpack = 16));
        COMPV_EXEC_IFDEF_ASM_X64((Convlt1_verthz_fxp = Convlt1_verthz_fxpq16_minpack16_Asm_X64_AVX2, minpack = 16));
    }

	int threadsCount = 1;
	CompVAsyncTaskIds taskIds;
	CompVPtr<CompVThreadDispatcher11* >threadDisp = CompVEngine::getThreadDispatcher11();
	if (threadDisp && threadDisp->getThreadsCount() > 1 && !threadDisp->isMotherOfTheCurrentThread()) {
		threadsCount = COMPV_MATH_MIN((width * height) / COMPV_CONVOLUTION_MIN_SAMPLES_PER_THREAD, int(threadDisp->getThreadsCount()));
		threadsCount = COMPV_MATH_MIN(threadsCount, (int)height); // divide across rows
	}

    if (Convlt1_verthz_fxp) {
		if (threadsCount > 1) {
			taskIds.reserve(threadsCount);
			auto funcPtr = [&](const uint8_t* blockInPtr, uint8_t* blockOutPtr, compv_scalar_t blockWidth, compv_scalar_t blockHeight, compv_scalar_t blockStride, compv_scalar_t blockPad, const uint16_t* blockHkernPtr, compv_scalar_t blockKernSize) -> COMPV_ERROR_CODE {
				Convlt1_verthz_fxp(blockInPtr, blockOutPtr, blockWidth, blockHeight, blockStride, blockPad, blockHkernPtr, blockKernSize);
				return COMPV_ERROR_CODE_S_OK;
			};
			const int heights = (height / threadsCount);
			const int lastHeight = height - ((threadsCount - 1) * heights);
			const int blockCount = heights * (width + pad); // stride parameter doesn't mean what you may think. Real stride = "width + pad"
			for (int i = 0, blockStart = 0; i < threadsCount; ++i, blockStart += blockCount) {
				COMPV_CHECK_CODE_ASSERT(threadDisp->invoke(std::bind(funcPtr, in_ptr + blockStart, out_ptr + blockStart, width, (i == threadsCount - 1) ? lastHeight : heights, stride, pad, vhkern_ptr, kern_size), taskIds));
			}
			// wait for threads later
		}
		else {
			Convlt1_verthz_fxp(in_ptr, out_ptr, width, height, stride, pad, vhkern_ptr, kern_size);
		}
        int missed = (width & (minpack - 1));
        if (missed == 0) {
			if (!taskIds.empty()) {
				COMPV_CHECK_CODE_ASSERT(threadDisp->wait(taskIds));
			}
            return;
        }
        in_ptr += (width - missed);
        out_ptr += (width - missed);
        pad += (width - missed);
        width = missed;
    }
    else {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // No SIMD and Single threaded
    }

    CompVConvlt<T>::convlt1_verthz_fxp_C(in_ptr, out_ptr, width, height, stride, pad, vhkern_ptr, kern_size);

	if (!taskIds.empty()) {
		COMPV_CHECK_CODE_ASSERT(threadDisp->wait(taskIds));
	}
}

template <typename T>
void CompVConvlt<T>::convlt1_verthz_C(const uint8_t* in_ptr, uint8_t* out_ptr, int width, int height, int stride, int pad, const T* vhkern_ptr, int kern_size)
{
    int i, j, row;
    T sum;
    const uint8_t *ptr_;

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            sum = 0;
            ptr_ = in_ptr;
            for (row = 0; row < kern_size; ++row) {
                sum += *ptr_ * vhkern_ptr[row];
                ptr_ += stride;
            }
            *out_ptr = COMPV_MATH_ROUNDFU_2_INT(sum, uint8_t);
            ++in_ptr;
            ++out_ptr;
        }
        in_ptr += pad;
        out_ptr += pad;
    }
}

template <typename T>
void CompVConvlt<T>::convlt1_verthz_fxp_C(const uint8_t* in_ptr, uint8_t* out_ptr, int width, int height, int stride, int pad, const uint16_t* vhkern_ptr, int kern_size)
{
    int i, j, row, sum;
    const uint8_t *ptr_;

    for (j = 0; j < height; ++j) {
        for (i = 0; i < width; ++i) {
            sum = 0;
            ptr_ = in_ptr;
            for (row = 0; row < kern_size; ++row) {
                sum += (*ptr_ * (int)vhkern_ptr[row]) >> COMPV_FXPQ;
                ptr_ += stride;
            }
            *out_ptr = (uint8_t)(sum);
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
