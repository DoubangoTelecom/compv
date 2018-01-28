/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_bits.h"
#include "compv/base/parallel/compv_parallel.h"

#define COMPV_BITS_AND_SAMPLES_PER_THREAD		(100 * 100)
#define COMPV_BITS_NOT_AND_SAMPLES_PER_THREAD	(100 * 100)
#define COMPV_BITS_NOT_SAMPLES_PER_THREAD		(100 * 100)

COMPV_BASE_API compv::compv_uscalar_t kPopcnt256[] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3,
	4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4
	, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4,
	5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3
	, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5,
	6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5
	, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

COMPV_NAMESPACE_BEGIN()

static void CompVBitsAnd_8u_C(const uint8_t* Aptr, const uint8_t* BPtr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride);
static void CompVBitsNotAnd_8u_C(const uint8_t* Aptr, const uint8_t* BPtr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride);
static void CompVBitsNot_8u_C(const uint8_t* Aptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride);

// R = (A & B)
// Supports and type (float, double, uint8, uint16....)
COMPV_ERROR_CODE CompVBits::and(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->cols() != B->cols() || A->rows() != B->rows() || A->planeCount() != B->planeCount(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (!R_ || (R_ != A && R_ != B)) { // This function allows R to be equal to A or B
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}

	void(*CompVBitsAnd_8u)(const uint8_t* Aptr, const uint8_t* BPtr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
		= CompVBitsAnd_8u_C;

	int planeId = 0;
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		CompVBitsAnd_8u(
			A->ptr<const uint8_t>(ystart, 0, planeId), B->ptr<const uint8_t>(ystart, 0, planeId), R_->ptr<uint8_t>(ystart, 0, planeId),
			static_cast<compv_uscalar_t>(A->cols(planeId)), static_cast<compv_uscalar_t>(yend - ystart),
			static_cast<compv_uscalar_t>(A->strideInBytes(planeId)), static_cast<compv_uscalar_t>(B->strideInBytes(planeId)), static_cast<compv_uscalar_t>(R_->strideInBytes(planeId))
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	const int planesCount = static_cast<int>(A->planeCount());
	for (planeId = 0; planeId < planesCount; ++planeId) {
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			A->cols(planeId),
			A->rows(planeId),
			COMPV_BITS_AND_SAMPLES_PER_THREAD
		));
	}

	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

// R = (~A & B)
// Supports and type (float, double, uint8, uint16....)
COMPV_ERROR_CODE CompVBits::not_and(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->cols() != B->cols() || A->rows() != B->rows() || A->planeCount() != B->planeCount(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (!R_ || (R_ != A && R_ != B)) { // This function allows R to be equal to A or B
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}

	// TODO(dmi): SSE -> _mm_andnot_si128
	// TODO(dmi): NEON -> vbic_u8

	void(*CompVBitsNotAnd_8u)(const uint8_t* Aptr, const uint8_t* BPtr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
		= CompVBitsNotAnd_8u_C;

	int planeId = 0;
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		CompVBitsNotAnd_8u(
			A->ptr<const uint8_t>(ystart, 0, planeId), B->ptr<const uint8_t>(ystart, 0, planeId), R_->ptr<uint8_t>(ystart, 0, planeId),
			static_cast<compv_uscalar_t>(A->cols(planeId)), static_cast<compv_uscalar_t>(yend - ystart),
			static_cast<compv_uscalar_t>(A->strideInBytes(planeId)), static_cast<compv_uscalar_t>(B->strideInBytes(planeId)), static_cast<compv_uscalar_t>(R_->strideInBytes(planeId))
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	const int planesCount = static_cast<int>(A->planeCount());
	for (planeId = 0; planeId < planesCount; ++planeId) {
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			A->cols(planeId),
			A->rows(planeId),
			COMPV_BITS_AND_SAMPLES_PER_THREAD
		));
	}

	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBits::not(const CompVMatPtr& A, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (!R_ || R_ != A) { // This function allows R to be equal to A
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}

	void(*CompVBitsNot_8u)(const uint8_t* Aptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride)
		= CompVBitsNot_8u_C;

	int planeId = 0;
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		CompVBitsNot_8u(
			A->ptr<const uint8_t>(ystart, 0, planeId), R_->ptr<uint8_t>(ystart, 0, planeId),
			static_cast<compv_uscalar_t>(A->cols(planeId)), static_cast<compv_uscalar_t>(yend - ystart),
			static_cast<compv_uscalar_t>(A->strideInBytes(planeId)), static_cast<compv_uscalar_t>(R_->strideInBytes(planeId))
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	const int planesCount = static_cast<int>(A->planeCount());
	for (planeId = 0; planeId < planesCount; ++planeId) {
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			A->cols(planeId),
			A->rows(planeId),
			COMPV_BITS_AND_SAMPLES_PER_THREAD
		));
	}

	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

static void CompVBitsAnd_8u_C(const uint8_t* Aptr, const uint8_t* BPtr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = (Aptr[i] & BPtr[i]);
		}
		Rptr += Rstride;
		Aptr += Astride;
		BPtr += Bstride;
	}
}

static void CompVBitsNotAnd_8u_C(const uint8_t* Aptr, const uint8_t* BPtr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = (~Aptr[i] & BPtr[i]);
		}
		Rptr += Rstride;
		Aptr += Astride;
		BPtr += Bstride;
	}
}

static void CompVBitsNot_8u_C(const uint8_t* Aptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = ~Aptr[i];
		}
		Rptr += Rstride;
		Aptr += Astride;
	}
}

COMPV_NAMESPACE_END()
