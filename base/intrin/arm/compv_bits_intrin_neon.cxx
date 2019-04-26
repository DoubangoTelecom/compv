/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/arm/compv_bits_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVBitsLogicalAnd_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			vst1q_u8(&Rptr[i], vandq_u8(
				vld1q_u8(&Aptr[i]),
				vld1q_u8(&Bptr[i])
			));
			vst1q_u8(&Rptr[i + 16], vandq_u8(
				vld1q_u8(&Aptr[i + 16]),
				vld1q_u8(&Bptr[i + 16])
			));
			vst1q_u8(&Rptr[i + 32], vandq_u8(
				vld1q_u8(&Aptr[i + 32]),
				vld1q_u8(&Bptr[i + 32])
			));
			vst1q_u8(&Rptr[i + 48], vandq_u8(
				vld1q_u8(&Aptr[i + 48]),
				vld1q_u8(&Bptr[i + 48])
			));
		}
		for (; i < width; i += 16) {
			vst1q_u8(&Rptr[i], vandq_u8(
				vld1q_u8(&Aptr[i]),
				vld1q_u8(&Bptr[i])
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
		Bptr += Bstride;
	}
}

void CompVBitsLogicalNotAnd_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			vst1q_u8(&Rptr[i], vbicq_u8(
				vld1q_u8(&Bptr[i]),
				vld1q_u8(&Aptr[i])
			));
			vst1q_u8(&Rptr[i + 16], vbicq_u8(
				vld1q_u8(&Bptr[i + 16]),
				vld1q_u8(&Aptr[i + 16])
			));
			vst1q_u8(&Rptr[i + 32], vbicq_u8(
				vld1q_u8(&Bptr[i + 32]),
				vld1q_u8(&Aptr[i + 32])
			));
			vst1q_u8(&Rptr[i + 48], vbicq_u8(
				vld1q_u8(&Bptr[i + 48]),
				vld1q_u8(&Aptr[i + 48])
			));
		}
		for (; i < width; i += 16) {
			vst1q_u8(&Rptr[i], vbicq_u8(
				vld1q_u8(&Bptr[i]),
				vld1q_u8(&Aptr[i])
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
		Bptr += Bstride;
	}
}

void CompVBitsLogicalNot_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			vst1q_u8(&Rptr[i], vmvnq_u8(
				vld1q_u8(&Aptr[i])
			));
			vst1q_u8(&Rptr[i + 16], vmvnq_u8(
				vld1q_u8(&Aptr[i + 16])
			));
			vst1q_u8(&Rptr[i + 32], vmvnq_u8(
				vld1q_u8(&Aptr[i + 32])
			));
			vst1q_u8(&Rptr[i + 48], vmvnq_u8(
				vld1q_u8(&Aptr[i + 48])
			));
		}
		for (; i < width; i += 16) {
			vst1q_u8(&Rptr[i], vmvnq_u8(
				vld1q_u8(&Aptr[i])
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
	}
}

void CompVBitsLogicalXorVt_8u_Intrin_NEON(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* A_Minus1_ptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
	compv_uscalar_t width64 = (width & -64);
	compv_uscalar_t i;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (i = 0; i < width64; i += 64) {
			vst1q_u8(&Rptr[i], veorq_u8(
				vld1q_u8(&A_Minus1_ptr[i]),
				vld1q_u8(&Aptr[i])
			));
			vst1q_u8(&Rptr[i + 16], veorq_u8(
				vld1q_u8(&A_Minus1_ptr[i + 16]),
				vld1q_u8(&Aptr[i + 16])
			));
			vst1q_u8(&Rptr[i + 32], veorq_u8(
				vld1q_u8(&A_Minus1_ptr[i + 32]),
				vld1q_u8(&Aptr[i + 32])
			));
			vst1q_u8(&Rptr[i + 48], veorq_u8(
				vld1q_u8(&A_Minus1_ptr[i + 48]),
				vld1q_u8(&Aptr[i + 48])
			));
		}
		for (; i < width; i += 16) {
			vst1q_u8(&Rptr[i], veorq_u8(
				vld1q_u8(&A_Minus1_ptr[i]),
				vld1q_u8(&Aptr[i])
			));
		}

		Rptr += Rstride;
		Aptr += Astride;
		A_Minus1_ptr += Astride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
