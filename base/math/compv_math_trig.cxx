/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_trig.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_trig_intrin_sse2.h"
#include "compv/base/math/intrin/x86/compv_math_trig_intrin_avx.h"
#include "compv/base/math/intrin/arm/compv_math_trig_intrin_neon.h"

#define COMPV_THIS_CLASSNAME	"CompVMathStats"

#define COMPV_MATH_TRIG_FASTATAN2_32F_SAMPLES_PER_THREAD			(16 * 16)
#define COMPV_MATH_TRIG_FASTATAN2_64F_SAMPLES_PER_THREAD			(8 * 8)
#define COMPV_MATH_TRIG_HYPOT_32F_SAMPLES_PER_THREAD			(32 * 32)
#define COMPV_MATH_TRIG_HYPOT_64F_SAMPLES_PER_THREAD			(16 * 16)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathTrigFastAtan2_32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* y, COMPV_ALIGNED(SSE) const compv_float32_t* x, COMPV_ALIGNED(SSE) compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigFastAtan2_32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* y, COMPV_ALIGNED(AVX) const compv_float32_t* x, COMPV_ALIGNED(AVX) compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigFastAtan2_32f_Asm_X64_FMA3_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* y, COMPV_ALIGNED(AVX) const compv_float32_t* x, COMPV_ALIGNED(AVX) compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigHypotNaive_32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* x, COMPV_ALIGNED(SSE) const compv_float32_t* y, COMPV_ALIGNED(SSE) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigHypotNaive_32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* x, COMPV_ALIGNED(AVX) const compv_float32_t* y, COMPV_ALIGNED(AVX) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigHypotNaive_32f_Asm_X64_FMA3_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* x, COMPV_ALIGNED(AVX) const compv_float32_t* y, COMPV_ALIGNED(AVX) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathTrigFastAtan2_32f_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* y, COMPV_ALIGNED(NEON) const compv_float32_t* x, COMPV_ALIGNED(NEON) compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigHypotNaive_32f_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* x, COMPV_ALIGNED(NEON) const compv_float32_t* y, COMPV_ALIGNED(NEON) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigHypotNaive_32f_Asm_FMA_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* x, COMPV_ALIGNED(NEON) const compv_float32_t* y, COMPV_ALIGNED(NEON) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathTrigHypotNaive_32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* x, COMPV_ALIGNED(NEON) const compv_float32_t* y, COMPV_ALIGNED(NEON) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
COMPV_EXTERNC void CompVMathTrigHypotNaive_32f_Asm_FMA_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* x, COMPV_ALIGNED(NEON) const compv_float32_t* y, COMPV_ALIGNED(NEON) compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

static void CompVMathTrigFastAtan2_32f_C(const compv_float32_t* y, const compv_float32_t* x, compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMathTrigFastAtan2_64f_C(const compv_float64_t* y, const compv_float64_t* x, compv_float64_t* r, const compv_float64_t* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMathTrigHypot_32f_C(const compv_float32_t* x, const compv_float32_t* y, compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMathTrigHypot_64f_C(const compv_float64_t* x, const compv_float64_t* y, compv_float64_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMathTrigHypotNaive_32f_C(const compv_float32_t* x, const compv_float32_t* y, compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);
static void CompVMathTrigHypotNaive_64f_C(const compv_float64_t* x, const compv_float64_t* y, compv_float64_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride);


//
//	CompVMathTrigGeneric
//

template<class T>
class CompVMathTrigGeneric
{
public:
	// (1x3) -> (3x3)
	static COMPV_ERROR_CODE rodriguesVectorToMatrix(const T(&vector)[3], CompVMatPtrPtr matrix)
	{
		// http://mathworld.wolfram.com/RodriguesRotationFormula.html
		// https://www.cs.duke.edu/courses/compsci527/fall13/notes/rodrigues.pdf

		// Zhang's Camera Calibration Algorithm: In-Depth Tutorial and Implementation: https://github.com/DoubangoTelecom/compv/blob/master/documentation/Camera%20calibration/Burger-CameraCalibration-20160516.pdf
		// A.2.4 Converting rotations
		// Rodrigues vector to Rotation matrix

		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(matrix, 3, 3));
		T* R0 = (*matrix)->ptr<T>(0);
		T* R1 = (*matrix)->ptr<T>(1);
		T* R2 = (*matrix)->ptr<T>(2);

		const T theta = std::sqrt((vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]));

		// TODO(dmi): maybe use closeToZero instead of comparing with 0.0
		if (theta == 0) {
			R0[0] = 1, R0[1] = 0, R0[2] = 0;
			R1[0] = 0, R1[1] = 1, R1[2] = 0;
			R2[0] = 0, R2[1] = 0, R2[2] = 1;
		}
		else {
			const T scale = (1 / theta);
			const T wx = vector[0] * scale;
			const T wy = vector[1] * scale;
			const T wz = vector[2] * scale;
			const T cos_theta = std::cos(theta);
			const T sin_theta = std::sin(theta);
			const T one_minus_cos_theta = (1 - cos_theta);
			const T wxwy = (wx * wy);
			const T wxwz = (wx * wz);
			const T wywz = (wy * wz);

			R0[0] = cos_theta + ((wx * wx) * one_minus_cos_theta);
			R0[1] = (wxwy * one_minus_cos_theta) - (wz * sin_theta);
			R0[2] = (wy * sin_theta) + (wxwz * one_minus_cos_theta);

			R1[0] = (wz * sin_theta) + (wxwy * one_minus_cos_theta);
			R1[1] = cos_theta + ((wy * wy) * one_minus_cos_theta);
			R1[2] = (wywz * one_minus_cos_theta) - (wx * sin_theta);

			R2[0] = (wxwz * one_minus_cos_theta) - (wy * sin_theta);
			R2[1] = (wx * sin_theta) + (wywz * one_minus_cos_theta);
			R2[2] = cos_theta + ((wz * wz) * one_minus_cos_theta);
		}

		return COMPV_ERROR_CODE_S_OK;
	}

	// (3x3) -> (1x3)
	// "rot3x3" must be a rotation matrix: a square matrix R is a rotation matrix if and only if Rt = R* and det(R) = 1 -> Rt.R = R.Rt = I
	static COMPV_ERROR_CODE rodriguesMatrixToVector(const CompVMatPtr& matrix, T(&vector)[3])
	{
		COMPV_CHECK_EXP_RETURN(!matrix || matrix->cols() != 3 || matrix->rows() != 3, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		// https://www.cs.duke.edu/courses/compsci527/fall13/notes/rodrigues.pdf

		// Zhang's Camera Calibration Algorithm: In-Depth Tutorial and Implementation: https://github.com/DoubangoTelecom/compv/blob/master/documentation/Camera%20calibration/Burger-CameraCalibration-20160516.pdf
		// A.2.4 Converting rotations
		// Rotation matrix to Rodrigues vector

		const T* R = matrix->ptr<const T>(0);
		const T R11 = R[0];
		const T R12 = R[1];
		const T R13 = R[2];
		R += matrix->stride();
		const T R21 = R[0];
		const T R22 = R[1];
		const T R23 = R[2];
		R += matrix->stride();
		const T R31 = R[0];
		const T R32 = R[1];
		const T R33 = R[2];

		const T a32 = (R32 - R23) * T(0.5);
		const T a13 = (R13 - R31) * T(0.5);
		const T a21 = (R21 - R12) * T(0.5);

		const T s = std::sqrt((a32 * a32) + (a13 * a13) + (a21 * a21));
		const T c = (R11 + R22 + R33 - 1) * T(0.5);

		// TODO(dmi): maybe use closeToZero instead of comparing with 0.0

		if (s == 0 && c == 1) {
			vector[0] = vector[1] = vector[2] = 0;
		}
		else {
			T u0, u1, u2;
			if (s == 0 && c == -1) {
				if ((R11 + 1) != 0 || R21 != 0 || R31 != 0) {
					u0 = (R11 + 1), u1 = R21, u2 = R31;
				}
				else if (R12 != 0 || (R22 + 1) != 0 || R32 != 0) {
					u0 = R12, u1 = (R22 + 1), u2 = R32;
				}
				else {
					u0 = R13, u1 = R23, u2 = (R33 + 1);
				}
				const T scale = (T(COMPV_MATH_PI) / std::sqrt((u0 * u0) + (u1 * u1) + (u2 * u2)));
				u0 *= scale, u1 *= scale, u2 *= scale;

				const T normr = std::sqrt((u0 * u0) + (u1 * u1) + (u2 * u2));
				if ((normr == T(COMPV_MATH_PI)) && ((u0 == 0 && u1 == 0 && u2 < 0) || (u0 == 0 && u1 == 0) || (u0 < 0))) {
					u0 = -u0, u1 = -u1, u2 = -u2;
				}
			}
			else {
				const T theta = std::atan2(s, c);
				const T scale = (theta / s);
				u0 = a32 * scale, u1 = a13 * scale, u2 = a21 * scale;
			}

			vector[0] = u0, vector[1] = u1, vector[2] = u2;
		}

		return COMPV_ERROR_CODE_S_OK;
	}
};

// 
//	CompVMathTrig
//

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesVectorToMatrix(const compv_float64x3_t& vector, CompVMatPtrPtr matrix) {
	COMPV_CHECK_CODE_RETURN((CompVMathTrigGeneric<compv_float64_t>::rodriguesVectorToMatrix(vector, matrix)));
	return COMPV_ERROR_CODE_S_OK;
}
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesVectorToMatrix(const compv_float32x3_t& vector, CompVMatPtrPtr matrix) {
	COMPV_CHECK_CODE_RETURN((CompVMathTrigGeneric<compv_float32_t>::rodriguesVectorToMatrix(vector, matrix)));
	return COMPV_ERROR_CODE_S_OK;
}

template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesMatrixToVector(const CompVMatPtr& matrix, compv_float64x3_t &vector) {
	COMPV_CHECK_CODE_RETURN((CompVMathTrigGeneric<compv_float64_t>::rodriguesMatrixToVector(matrix, vector)));
	return COMPV_ERROR_CODE_S_OK;
}
template<> COMPV_BASE_API COMPV_ERROR_CODE CompVMathTrig::rodriguesMatrixToVector(const CompVMatPtr& matrix, compv_float32x3_t &vector) {
	COMPV_CHECK_CODE_RETURN((CompVMathTrigGeneric<compv_float32_t>::rodriguesMatrixToVector(matrix, vector)));
	return COMPV_ERROR_CODE_S_OK;
}


template<typename FloatType>
static COMPV_ERROR_CODE CompVMathTrigFastAtan2_X(const FloatType* y, const FloatType* x, FloatType* r, const FloatType& scale, 
	const size_t width, const size_t height, const size_t stride,
	void(*fptr)(const FloatType* y, const FloatType* x, FloatType* r, const FloatType* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride))
{
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		fptr(&y[offset], &x[offset], &r[offset], &scale,
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		std::is_same<FloatType, compv_float64_t>::value ? COMPV_MATH_TRIG_FASTATAN2_64F_SAMPLES_PER_THREAD : COMPV_MATH_TRIG_FASTATAN2_32F_SAMPLES_PER_THREAD
	));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathTrig::fastAtan2(const CompVMatPtr& y, const CompVMatPtr& x, CompVMatPtrPtr r, const bool angleInDeg)
{
	COMPV_CHECK_EXP_RETURN(!y || !x || !r || y->cols() != x->cols() || y->rows() != x->rows() || y->stride() != x->stride() || y->subType() != x->subType() ||
		(y->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && y->subType() != COMPV_SUBTYPE_RAW_FLOAT64)
		,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const size_t width = y->cols();
	const size_t height = y->rows();
	const size_t stride = y->stride();
	const COMPV_SUBTYPE subType = y->subType();
	CompVMatPtr r_ = (*r == y || *r == x) ? nullptr : *r;
	if (subType == COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&r_, height, width, stride));
		void(*CompVMathTrigFastAtan2_64f)(const compv_float64_t* y, const compv_float64_t* x, compv_float64_t* r, const compv_float64_t* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
			= CompVMathTrigFastAtan2_64f_C;
#if COMPV_ARCH_X86
#elif COMPV_ARCH_ARM
#endif
		COMPV_CHECK_CODE_RETURN((CompVMathTrigFastAtan2_X<compv_float64_t>(y->ptr<const compv_float64_t>(), x->ptr<const compv_float64_t>(), r_->ptr<compv_float64_t>(), angleInDeg ? 1.0 : M_PI / 180.0,
			width, height, stride, CompVMathTrigFastAtan2_64f)));
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&r_, height, width, stride));
		void(*CompVMathTrigFastAtan2_32f)(const compv_float32_t* y, const compv_float32_t* x, compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
			= CompVMathTrigFastAtan2_32f_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && x->isAlignedSSE() && y->isAlignedSSE() && r_->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTrigFastAtan2_32f = CompVMathTrigFastAtan2_32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigFastAtan2_32f = CompVMathTrigFastAtan2_32f_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && x->isAlignedAVX() && y->isAlignedAVX() && r_->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTrigFastAtan2_32f = CompVMathTrigFastAtan2_32f_Intrin_AVX);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigFastAtan2_32f = CompVMathTrigFastAtan2_32f_Asm_X64_AVX);
			if (CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigFastAtan2_32f = CompVMathTrigFastAtan2_32f_Asm_X64_FMA3_AVX);
			}
		}
#elif COMPV_ARCH_ARM
        if (CompVCpu::isEnabled(compv::kCpuFlagARM_NEON) && x->isAlignedNEON() && y->isAlignedNEON() && r_->isAlignedNEON()) {
            COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathTrigFastAtan2_32f = CompVMathTrigFastAtan2_32f_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathTrigFastAtan2_32f = CompVMathTrigFastAtan2_32f_Asm_NEON32);
        }
#endif
		COMPV_CHECK_CODE_RETURN((CompVMathTrigFastAtan2_X<compv_float32_t>(y->ptr<const compv_float32_t>(), x->ptr<const compv_float32_t>(), r_->ptr<compv_float32_t>(), static_cast<compv_float32_t>(angleInDeg ? 1.0 : M_PI / 180.0),
			width, height, stride, CompVMathTrigFastAtan2_32f)));
	}

	*r = r_;
	return COMPV_ERROR_CODE_S_OK;
}

template<typename FloatType>
static COMPV_ERROR_CODE CompVMathTrigHypot_X(const FloatType* x, const FloatType* y, FloatType* r,
	const size_t width, const size_t height, const size_t stride,
	void(*fptr)(const FloatType* x, const FloatType* y, FloatType* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride))
{
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const size_t offset = (ystart * stride);
		fptr(&x[offset], &y[offset], &r[offset],
			static_cast<compv_uscalar_t>(width), static_cast<compv_uscalar_t>(yend - ystart), static_cast<compv_uscalar_t>(stride));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		width,
		height,
		std::is_same<FloatType, compv_float64_t>::value ? COMPV_MATH_TRIG_HYPOT_64F_SAMPLES_PER_THREAD : COMPV_MATH_TRIG_HYPOT_32F_SAMPLES_PER_THREAD
	));
	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Hypot
// Use hypot_naive which is optimised if you're sure there is no overflow/underflow risk
COMPV_ERROR_CODE CompVMathTrig::hypot(const CompVMatPtr& x, const CompVMatPtr& y, CompVMatPtrPtr r)
{
	COMPV_CHECK_EXP_RETURN(!y || !x || !r || y->cols() != x->cols() || y->rows() != x->rows() || y->stride() != x->stride() || y->subType() != x->subType() ||
		(y->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && y->subType() != COMPV_SUBTYPE_RAW_FLOAT64)
		,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const size_t width = y->cols();
	const size_t height = y->rows();
	const size_t stride = y->stride();
	const COMPV_SUBTYPE subType = y->subType();
	CompVMatPtr r_ = (*r == y || *r == x) ? nullptr : *r;
	if (subType == COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&r_, height, width, stride));
		void(*CompVMathTrigHypot_64f)(const compv_float64_t* x, const compv_float64_t* y, compv_float64_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
			= CompVMathTrigHypot_64f_C;
#if COMPV_ARCH_X86
#elif COMPV_ARCH_ARM
#endif
		COMPV_CHECK_CODE_RETURN((CompVMathTrigHypot_X<compv_float64_t>(x->ptr<const compv_float64_t>(), y->ptr<const compv_float64_t>(), r_->ptr<compv_float64_t>(),
			width, height, stride, CompVMathTrigHypot_64f)));
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&r_, height, width, stride));
		void(*CompVMathTrigHypot_32f)(const compv_float32_t* x, const compv_float32_t* y, compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
			= CompVMathTrigHypot_32f_C;
#if COMPV_ARCH_X86 && 0
		if (width >= 4 && CompVCpu::isEnabled(compv::kCpuFlagSSE2) && x->isAlignedSSE() && y->isAlignedSSE() && r_->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTrigHypot_32f = CompVMathTrigHypot_32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigHypot_32f = CompVMathTrigHypot_32f_Asm_X64_SSE2);
		}
		if (width >= 8 && CompVCpu::isEnabled(compv::kCpuFlagAVX) && x->isAlignedAVX() && y->isAlignedAVX() && r_->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTrigHypot_32f = CompVMathTrigHypot_32f_Intrin_AVX);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigHypot_32f = CompVMathTrigHypot_32f_Asm_X64_AVX);
			if (CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigHypot_32f = CompVMathTrigHypot_32f_Asm_X64_FMA3_AVX);
			}
		}
#elif COMPV_ARCH_ARM
#endif
		COMPV_CHECK_CODE_RETURN((CompVMathTrigHypot_X<compv_float32_t>(x->ptr<const compv_float32_t>(), y->ptr<const compv_float32_t>(), r_->ptr<compv_float32_t>(),
			width, height, stride, CompVMathTrigHypot_32f)));
	}

	*r = r_;
	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Hypot
COMPV_ERROR_CODE CompVMathTrig::hypot_naive(const CompVMatPtr& x, const CompVMatPtr& y, CompVMatPtrPtr r)
{
	COMPV_CHECK_EXP_RETURN(!y || !x || !r || y->cols() != x->cols() || y->rows() != x->rows() || y->stride() != x->stride() || y->subType() != x->subType() ||
		(y->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && y->subType() != COMPV_SUBTYPE_RAW_FLOAT64)
		,
		COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const size_t width = y->cols();
	const size_t height = y->rows();
	const size_t stride = y->stride();
	const COMPV_SUBTYPE subType = y->subType();
	CompVMatPtr r_ = (*r == y || *r == x) ? nullptr : *r;
	if (subType == COMPV_SUBTYPE_RAW_FLOAT64) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&r_, height, width, stride));
		void(*CompVMathTrigHypotNaive_64f)(const compv_float64_t* x, const compv_float64_t* y, compv_float64_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
			= CompVMathTrigHypotNaive_64f_C;
#if COMPV_ARCH_X86
#elif COMPV_ARCH_ARM
#endif
		COMPV_CHECK_CODE_RETURN((CompVMathTrigHypot_X<compv_float64_t>(x->ptr<const compv_float64_t>(), y->ptr<const compv_float64_t>(), r_->ptr<compv_float64_t>(),
			width, height, stride, CompVMathTrigHypotNaive_64f)));
	}
	else {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&r_, height, width, stride));
		void(*CompVMathTrigHypotNaive_32f)(const compv_float32_t* x, const compv_float32_t* y, compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
			= CompVMathTrigHypotNaive_32f_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(compv::kCpuFlagSSE2) && x->isAlignedSSE() && y->isAlignedSSE() && r_->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(compv::kCpuFlagAVX) && x->isAlignedAVX() && y->isAlignedAVX() && r_->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Intrin_AVX);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Asm_X64_AVX);
			if (CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Asm_X64_FMA3_AVX);
			}
		}
#elif COMPV_ARCH_ARM
        if (CompVCpu::isEnabled(compv::kCpuFlagARM_NEON) && x->isAlignedNEON() && y->isAlignedNEON() && r_->isAlignedNEON()) {
            COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Asm_NEON32);
            COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Asm_NEON64);
            if (CompVCpu::isEnabled(compv::kCpuFlagARM_NEON_FMA)) {
                COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Asm_FMA_NEON64);
                COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathTrigHypotNaive_32f = CompVMathTrigHypotNaive_32f_Asm_FMA_NEON32);
            }
        }
#endif
		COMPV_CHECK_CODE_RETURN((CompVMathTrigHypot_X<compv_float32_t>(x->ptr<const compv_float32_t>(), y->ptr<const compv_float32_t>(), r_->ptr<compv_float32_t>(),
			width, height, stride, CompVMathTrigHypotNaive_32f)));
	}

	*r = r_;
	return COMPV_ERROR_CODE_S_OK;
}

// Copyright: Un-optimised code based on OpenCV - Not used at all unless SIMD and GPU are disabled (must never happen)
template<typename FloatType>
static void CompVMathTrigFastAtan2_X_C(const FloatType* y, const FloatType* x, FloatType* r, const FloatType* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	static const FloatType atan2_eps = static_cast<FloatType>(kMathTrigAtan2Eps);
	static const FloatType atan2_p1 = static_cast<FloatType>(kMathTrigAtan2P1);
	static const FloatType atan2_p3 = static_cast<FloatType>(kMathTrigAtan2P3);
	static const FloatType atan2_p5 = static_cast<FloatType>(kMathTrigAtan2P5);
	static const FloatType atan2_p7 = static_cast<FloatType>(kMathTrigAtan2P7);
	const FloatType& scale = *scale1;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const FloatType ax = std::abs(x[i]), ay = std::abs(y[i]);
			FloatType a, c, c2;
			if (ax >= ay) {
				c = ay / (ax + atan2_eps);
				c2 = c*c;
				a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c;
			}
			else {
				c = ax / (ay + atan2_eps);
				c2 = c*c;
				a = 90.f - (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c;
			}
			if (x[i] < 0) {
				a = 180.f - a;
			}
			if (y[i] < 0) {
				a = 360.f - a;
			}
			r[i] = a * scale;
		}
		y += stride;
		x += stride;
		r += stride;
	}
}
static void CompVMathTrigFastAtan2_32f_C(const compv_float32_t* y, const compv_float32_t* x, compv_float32_t* r, const compv_float32_t* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVMathTrigFastAtan2_X_C<compv_float32_t>(y, x, r, scale1, width, height, stride);
}
static void CompVMathTrigFastAtan2_64f_C(const compv_float64_t* y, const compv_float64_t* x, compv_float64_t* r, const compv_float64_t* scale1, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVMathTrigFastAtan2_X_C<compv_float64_t>(y, x, r, scale1, width, height, stride);
}


template<typename FloatType>
static void CompVMathTrigHypot_X_C(const FloatType* x, const FloatType* y, FloatType* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			// https://en.wikipedia.org/wiki/Hypot
			FloatType a = COMPV_MATH_ABS(x[i]);
			const FloatType b = COMPV_MATH_ABS(y[i]);
#if 0
			if (b > a) {
				// swap(a, b)
				t = a;
				a = b;
				b = t;
			}
#else // Branchless (SIMD-friendly)
			FloatType t = COMPV_MATH_MIN(a, b);
			a = COMPV_MATH_MAX(a, b);
#endif
			if (!a) {
				r[i] = t;
			}
			else {
				t = t / a;
				r[i] = a * COMPV_MATH_SQRT(1 + t*t);
			}
		}
		y += stride;
		x += stride;
		r += stride;
	}
}
static void CompVMathTrigHypot_32f_C(const compv_float32_t* x, const compv_float32_t* y, compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVMathTrigHypot_X_C<compv_float32_t>(x, y, r, width, height, stride);
}
static void CompVMathTrigHypot_64f_C(const compv_float64_t* x, const compv_float64_t* y, compv_float64_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVMathTrigHypot_X_C<compv_float64_t>(x, y, r, width, height, stride);
}

template<typename FloatType>
static void CompVMathTrigHypotNaive_X_C(const FloatType* x, const FloatType* y, FloatType* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			// https://en.wikipedia.org/wiki/Hypot
			r[i] = std::sqrt(x[i]* x[i] + y[i]* y[i]);
		}
		y += stride;
		x += stride;
		r += stride;
	}
}
static void CompVMathTrigHypotNaive_32f_C(const compv_float32_t* x, const compv_float32_t* y, compv_float32_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVMathTrigHypotNaive_X_C<compv_float32_t>(x, y, r, width, height, stride);
}
static void CompVMathTrigHypotNaive_64f_C(const compv_float64_t* x, const compv_float64_t* y, compv_float64_t* r, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride) {
	CompVMathTrigHypotNaive_X_C<compv_float64_t>(x, y, r, width, height, stride);
}

COMPV_NAMESPACE_END()
