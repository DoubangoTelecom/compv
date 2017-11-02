/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_trig.h"
#include "compv/base/math/compv_math_utils.h"

#define COMPV_THIS_CLASSNAME	"CompVMathStats"

COMPV_NAMESPACE_BEGIN()

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

COMPV_NAMESPACE_END()