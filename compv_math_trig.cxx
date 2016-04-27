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
/*
Most of trig approx. are implemented using document at "documentation/trig_approximations.pdf"
*/
#include "compv/compv_math_trig.h"
#include "compv/compv_cpu.h"

#include <algorithm>

COMPV_NAMESPACE_BEGIN()

static void sincos_Zero_PiOver2_P32_C(const float* inRad, float* outSin, float* outCos, compv_scalar_t count);
static void sincos_Zero_PiTime2_P32_C(const float* inRad, float* outSin, float* outCos, compv_scalar_t count);
static void sincos_P32_C(const float* inRad, float* outSin, float* outCos, compv_scalar_t count);

/*
Computes cos(x) with 3.2 decimal accuracy. x must be within [0, PI/2]
*/
void CompVMathTrig::sincos_Zero_PiOver2_P32(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
{
	void(*fn)(const float* inRad, float* outSin, float* outCos, compv_scalar_t count) = sincos_Zero_PiOver2_P32_C;

	fn(inRad, outSin, outCos, count);
}

/*
Computes cos(x) with 3.2 decimal accuracy. x must be within [0, 2*PI]
If the angle is within [0, Pi/2] then, use "sincos_Zero_PiOver2_P32" which is faster.
*/
void CompVMathTrig::sincos_Zero_PiTime2_P32(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
{
	void(*fn)(const float* inRad, float* outSin, float* outCos, compv_scalar_t count) = sincos_Zero_PiTime2_P32_C;

	fn(inRad, outSin, outCos, count);
}

/*
Computes cos(x) with 3.2 decimal accuracy. x could be any value within IR.
If the angle is within [0, Pi/2] then, use "sincos_Zero_PiOver2_P32" which is faster.
*/
void CompVMathTrig::sincos_P32(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
{
	void(*fn)(const float* inRad, float* outSin, float* outCos, compv_scalar_t count) = sincos_P32_C;

	fn(inRad, outSin, outCos, count);
}

static void sincos_Zero_PiOver2_P32_C(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	COMPV_DEBUG_INFO_CODE_NOT_TESTED();
    float in2;
    for (compv_scalar_t i = 0; i < count; ++i) {
		// cos(x)
		in2 = inRad[i];
		in2 *= in2;
		outCos[i] = (kMathTrigC1 + in2 * (kMathTrigC2 + kMathTrigC3 * in2));
		// sin(x) = cos((pi/2)-x)
		in2 = kfMathTrigPiOver2 - inRad[i];
		in2 *= in2;
		outSin[i] = (kMathTrigC1 + in2 * (kMathTrigC2 + kMathTrigC3 * in2));
    }
}

static void sincos_Zero_PiTime2_P32_C(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
	int q_;
	float in_, in2_, out_;
	bool signed_;

#define SINCOS_GET_IN2_AND_SIGN(angle) { \
	q_ = int(angle / kfMathTrigPiOver2); /* Get quadrant # (0 to 3) */ \
	switch (q_) { \
		case 0: /* sincos_Zero_PiOver2_P32(x) */ \
			in_ = angle; signed_ = false; break; \
		case 1:  /* -sincos_Zero_PiOver2_P32(pi - x) */ \
			in_ = kfMathTrigPi - angle; signed_ = true; break; \
		case 2:  /* -sincos_Zero_PiOver2_P32(x - pi) */ \
			in_ = angle - kfMathTrigPi; signed_ = true; break; \
		case 3: /* sincos_Zero_PiOver2_P32(twopi - x) */ \
			in_ = kfMathTrigPiTimes2 - angle; signed_ = false; break; \
		default: \
			COMPV_DEBUG_ERROR("%d not valid quadrant", q_); continue; \
			} /* switch */ \
		in2_ = in_ * in_; \
		}

	for (compv_scalar_t i = 0; i < count; ++i) {
		// cos(x)
		in_ = inRad[i];
		//in_ = fmodf(in_, kfMathTrigPiTimes2);
		if (in_ < 0) {
			in_ = -in_;
		}
		SINCOS_GET_IN2_AND_SIGN(in_);
		out_ = (kMathTrigC1 + in2_ * (kMathTrigC2 + kMathTrigC3 * in2_));
		outCos[i] = signed_ ? -out_ : out_;
		// sin(x) = cos((pi/2)-x)
		in_ = (kfMathTrigPiOver2 - inRad[i]);
		//in_ = fmodf(in_, kfMathTrigPiTimes2);
		if (in_ < 0) {
			in_ = -in_;
		}
		SINCOS_GET_IN2_AND_SIGN(in_);
		out_ = (kMathTrigC1 + in2_ * (kMathTrigC2 + kMathTrigC3 * in2_));
		outSin[i] = signed_ ? -out_ : out_;
	}
}

static void sincos_P32_C(const float* inRad, float* outSin, float* outCos, compv_scalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Not faster than cosf/sinf, because of fmodf
	int q_;
	float in_, in2_, out_;
	bool signed_;

#define SINCOS_GET_IN2_AND_SIGN(angle) { \
	q_ = int(angle / kfMathTrigPiOver2); /* Get quadrant # (0 to 3) */ \
	switch (q_) { \
		case 0: /* sincos_Zero_PiOver2_P32(x) */ \
			in_ = angle; signed_ = false; break; \
		case 1:  /* -sincos_Zero_PiOver2_P32(pi - x) */ \
			in_ = kfMathTrigPi - angle; signed_ = true; break; \
		case 2:  /* -sincos_Zero_PiOver2_P32(x - pi) */ \
			in_ = angle - kfMathTrigPi; signed_ = true; break; \
		case 3: /* sincos_Zero_PiOver2_P32(twopi - x) */ \
			in_ = kfMathTrigPiTimes2 - angle; signed_ = false; break; \
		default: \
			COMPV_DEBUG_ERROR("%d not valid quadrant", q_); continue; \
		} /* switch */ \
		in2_ = in_ * in_; \
	}

	for (compv_scalar_t i = 0; i < count; ++i) {
		// cos(x)
		in_ = inRad[i];
		in_ = fmodf(in_, kfMathTrigPiTimes2);
		if (in_ < 0) {
			in_ = -in_;
		}
		SINCOS_GET_IN2_AND_SIGN(in_);
		out_ = (kMathTrigC1 + in2_ * (kMathTrigC2 + kMathTrigC3 * in2_));
		outCos[i] = signed_ ? -out_ : out_;
		// sin(x) = cos((pi/2)-x)
		in_ = (kfMathTrigPiOver2 - inRad[i]);
		in_ = fmodf(in_, kfMathTrigPiTimes2);
		if (in_ < 0) {
			in_ = -in_;
		}
		SINCOS_GET_IN2_AND_SIGN(in_);
		out_ = (kMathTrigC1 + in2_ * (kMathTrigC2 + kMathTrigC3 * in2_));
		outSin[i] = signed_ ? -out_ : out_;
	}
}

COMPV_NAMESPACE_END()
