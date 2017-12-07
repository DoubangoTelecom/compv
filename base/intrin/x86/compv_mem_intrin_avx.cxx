/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/x86/compv_mem_intrin_avx.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_avx.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): add NTA version
// TODO(dmi): add ASM version
// size must be > 32 and it's up to the caller to check it
// size should be multiple of 32, if not the remaining will be ignored
#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void MemCopy_Intrin_Aligned_AVX(COMPV_ALIGNED(AVX) void* dataDstPtr, COMPV_ALIGNED(AVX) const void* dataSrcPtr, compv_uscalar_t size)
{
    COMPV_DEBUG_INFO_CHECK_AVX();
    _mm256_zeroupper();
    const __m256i* ymmSrc = (const __m256i*)dataSrcPtr;
    __m256i* ymmDst = (__m256i*)dataDstPtr;
    compv_uscalar_t count32x16 = (size >> 9);
    compv_uscalar_t count32x1 = (size >> 5);

    if (count32x16 > 0) {
        count32x1 = ((size - (count32x16 << 9))) >> 5;
        for (compv_uscalar_t i = 0; i < count32x16; ++i) {
            _mm256_store_si256(&ymmDst[0], _mm256_load_si256(&ymmSrc[0]));
            _mm256_store_si256(&ymmDst[1], _mm256_load_si256(&ymmSrc[1]));
            _mm256_store_si256(&ymmDst[2], _mm256_load_si256(&ymmSrc[2]));
            _mm256_store_si256(&ymmDst[3], _mm256_load_si256(&ymmSrc[3]));
            _mm256_store_si256(&ymmDst[4], _mm256_load_si256(&ymmSrc[4]));
            _mm256_store_si256(&ymmDst[5], _mm256_load_si256(&ymmSrc[5]));
            _mm256_store_si256(&ymmDst[6], _mm256_load_si256(&ymmSrc[6]));
            _mm256_store_si256(&ymmDst[7], _mm256_load_si256(&ymmSrc[7]));
            _mm256_store_si256(&ymmDst[8], _mm256_load_si256(&ymmSrc[8]));
            _mm256_store_si256(&ymmDst[9], _mm256_load_si256(&ymmSrc[9]));
            _mm256_store_si256(&ymmDst[10], _mm256_load_si256(&ymmSrc[10]));
            _mm256_store_si256(&ymmDst[11], _mm256_load_si256(&ymmSrc[11]));
            _mm256_store_si256(&ymmDst[12], _mm256_load_si256(&ymmSrc[12]));
            _mm256_store_si256(&ymmDst[13], _mm256_load_si256(&ymmSrc[13]));
            _mm256_store_si256(&ymmDst[14], _mm256_load_si256(&ymmSrc[14]));
            _mm256_store_si256(&ymmDst[15], _mm256_load_si256(&ymmSrc[15]));
            ymmDst += 16;
            ymmSrc += 16;
        }
    }

    for (compv_uscalar_t i = 0; i < count32x1; ++i) {
        _mm256_store_si256(ymmDst, _mm256_load_si256(ymmSrc));
        ++ymmSrc;
        ++ymmDst;
    }
    _mm256_zeroupper();
}

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
void MemCopyNTA_Intrin_Aligned_AVX(COMPV_ALIGNED(AVX) void* dataDstPtr, COMPV_ALIGNED(AVX) const void* dataSrcPtr, compv_uscalar_t size)
{
    _mm256_zeroupper();
    const __m256i* ymmSrc = (const __m256i*)dataSrcPtr;
    __m256i* ymmDst = (__m256i*)dataDstPtr;
    compv_uscalar_t count32x16 = (size >> 9);
    compv_uscalar_t count32x1 = (size >> 5);

    if (count32x16 > 0) {
        count32x1 = ((size - (count32x16 << 9))) >> 5;
        for (compv_uscalar_t i = 0; i < count32x16; ++i) {
            _mm256_stream_si256(&ymmDst[0], _mm256_load_si256(&ymmSrc[0]));
            _mm256_stream_si256(&ymmDst[1], _mm256_load_si256(&ymmSrc[1]));
            _mm256_stream_si256(&ymmDst[2], _mm256_load_si256(&ymmSrc[2]));
            _mm256_stream_si256(&ymmDst[3], _mm256_load_si256(&ymmSrc[3]));
            _mm256_stream_si256(&ymmDst[4], _mm256_load_si256(&ymmSrc[4]));
            _mm256_stream_si256(&ymmDst[5], _mm256_load_si256(&ymmSrc[5]));
            _mm256_stream_si256(&ymmDst[6], _mm256_load_si256(&ymmSrc[6]));
            _mm256_stream_si256(&ymmDst[7], _mm256_load_si256(&ymmSrc[7]));
            _mm256_stream_si256(&ymmDst[8], _mm256_load_si256(&ymmSrc[8]));
            _mm256_stream_si256(&ymmDst[9], _mm256_load_si256(&ymmSrc[9]));
            _mm256_stream_si256(&ymmDst[10], _mm256_load_si256(&ymmSrc[10]));
            _mm256_stream_si256(&ymmDst[11], _mm256_load_si256(&ymmSrc[11]));
            _mm256_stream_si256(&ymmDst[12], _mm256_load_si256(&ymmSrc[12]));
            _mm256_stream_si256(&ymmDst[13], _mm256_load_si256(&ymmSrc[13]));
            _mm256_stream_si256(&ymmDst[14], _mm256_load_si256(&ymmSrc[14]));
            _mm256_stream_si256(&ymmDst[15], _mm256_load_si256(&ymmSrc[15]));
            ymmDst += 16;
            ymmSrc += 16;
        }
    }

    for (compv_uscalar_t i = 0; i < count32x1; ++i) {
        _mm256_stream_si256(ymmDst, _mm256_load_si256(ymmSrc));
        ++ymmSrc;
        ++ymmDst;
    }
    _mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
