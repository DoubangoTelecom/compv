/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/x86/compv_bits_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVBitsLogicalAnd_8u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(SSE) compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_and_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 16]), _mm_and_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 16])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i + 16]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 32]), _mm_and_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 32])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i + 32]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 48]), _mm_and_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 48])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i + 48]))
			));
		}
		for (; i < width; i += 16) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_and_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i]))
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
		Bptr += Bstride;
	}
}

void CompVBitsLogicalNotAnd_8u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(SSE) compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_andnot_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 16]), _mm_andnot_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 16])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i + 16]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 32]), _mm_andnot_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 32])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i + 32]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 48]), _mm_andnot_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 48])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i + 48]))
			));
		}
		for (; i < width; i += 16) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_andnot_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Bptr[i]))
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
		Bptr += Bstride;
	}
}

void CompVBitsLogicalNot_8u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;
	const __m128i vecFF = _mm_set1_epi8((char)0xff);

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				vecFF
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 16]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 16])),
				vecFF
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 32]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 32])),
				vecFF
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 48]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 48])),
				vecFF
			));
		}
		for (; i < width; i += 16) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_andnot_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				vecFF
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
	}
}

void CompVBitsLogicalXorVt_8u_Intrin_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* A_Minus1_ptr, COMPV_ALIGNED(SSE) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&A_Minus1_ptr[i]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 16]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 16])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&A_Minus1_ptr[i + 16]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 32]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 32])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&A_Minus1_ptr[i + 32]))
			));
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i + 48]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i + 48])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&A_Minus1_ptr[i + 48]))
			));
		}
		for (; i < width; i += 16) {
			_mm_store_si128(reinterpret_cast<__m128i*>(&Rptr[i]), _mm_xor_si128(
				_mm_load_si128(reinterpret_cast<const __m128i*>(&Aptr[i])),
				_mm_load_si128(reinterpret_cast<const __m128i*>(&A_Minus1_ptr[i]))
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
		A_Minus1_ptr += Astride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
