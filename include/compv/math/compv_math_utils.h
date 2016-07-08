/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATHUTILS_H_)
#define _COMPV_MATHUTILS_H_

#include "compv/compv_config.h"
#include "compv/math/compv_math.h"
#include "compv/compv_common.h"
#include "compv/compv_mem.h"
#include "compv/compv_cpu.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVMathUtils
{
public:
    static COMPV_ERROR_CODE init();
    static COMPV_INLINE int32_t clamp(int32_t min, int32_t val, int32_t max) { // do not use macro, otherwise 'val' which coulbe a function will be evaluated several times
        // Important: Do not use CLIP3_INT here: very slow on Windows (tested on Win8 core i7 using VS2013)
        return COMPV_MATH_CLIP3(min, max, val);
    }
    static COMPV_INLINE uint8_t clampPixel8(int16_t val) {
        // Important: Do not use CLIP3_INT here: very slow on Windows (tested on Win8 core i7 using VS2013)
        // Also, asm_cmov_clip2() is sloow -> To be checked
        return (uint8_t)COMPV_MATH_CLIP3(0, 255, val);
    }
    static COMPV_INLINE compv_scalar_t maxVal(compv_scalar_t x, compv_scalar_t y) {
        return maxValFunc(x, y);
    }
    static COMPV_INLINE compv_scalar_t minVal(compv_scalar_t x, compv_scalar_t y) {
        return minValFunc(x, y);
    }
    static COMPV_INLINE int32_t minArrayI32(const int32_t* array, compv_scalar_t count) {
        return minArrayI32Func(array, count);
    }
    static COMPV_INLINE compv_scalar_t clip3(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val) {
        return clip3Func(min, max, val);
    }
    static COMPV_INLINE compv_scalar_t clip2(compv_scalar_t max, compv_scalar_t val) {
        return clip2Func(max, val);
    }
	static COMPV_INLINE void rand(uint32_t *r, compv_scalar_t count) {
		randFunc(r, count);
	}

	template <typename InputType>
	static COMPV_ERROR_CODE max(const InputType* data, size_t width, size_t height, size_t stride, InputType &max)
	{
		COMPV_CHECK_EXP_RETURN(!data || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
		max = data[0];
		for (size_t j = 0; j < height; ++j) {
			for (size_t i = 0; i < width; ++i) {
				if (data[i] > max) {
					max = data[i];
				}
			}
			data += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename InputType, typename AccumulatorType>
	static COMPV_ERROR_CODE mean(const InputType* data, size_t count, InputType &mean)
	{
		COMPV_CHECK_EXP_RETURN(!data || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
		AccumulatorType sum = 0;
		for (size_t i = 0; i < count; ++i) {
			sum += data[i]; // UpTo the caller to choose correct "InputType" and "AccumulatorType" to avoid overflow
		}
		mean = (InputType)(sum / count);
		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename InputType, typename AccumulatorType>
	static COMPV_ERROR_CODE mean(const InputType* data, size_t width, size_t height, size_t stride, InputType &mean)
	{
		COMPV_CHECK_EXP_RETURN(!data || !width || !height || !stride, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		InputType* means = (InputType*)CompVMem::malloc(height * sizeof(InputType));
		COMPV_CHECK_EXP_RETURN(!means, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		for (size_t j = 0; j < height; ++j) {
			CompVMathUtils::mean<InputType, AccumulatorType>(data, width, means[j]);
			data += stride;
		}
		CompVMathUtils::mean<InputType, AccumulatorType>(means, height, mean);
		CompVMem::free((void**)&means);
		return COMPV_ERROR_CODE_S_OK;
	}

	// Function used to compute L1 distance
	// r = abs(a) + abs(b)
	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE addAbs(const InputType* a, const InputType* b, OutputType*& r, size_t width, size_t height, size_t stride)
	{
		COMPV_CHECK_EXP_RETURN(!a || !b || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (!r) {
			r = (OutputType*)CompVMem::malloc(height * stride * sizeof(OutputType));
			COMPV_CHECK_EXP_RETURN(!r, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::addAbs_C<InputType, OutputType>(a, b, r, width, height, stride)));
		return COMPV_ERROR_CODE_S_OK;
	}

	// compute gradient using L1 distance (g = abs(gx) + abs(gy)) and the maximum value
	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE gradientL1(const InputType* gx, const InputType* gy, OutputType*& g, size_t width, size_t height, size_t stride, OutputType* max = NULL)
	{
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradient<InputType, OutputType>(gx, gy, g, width, height, stride, max, true)));
		return COMPV_ERROR_CODE_S_OK;
	}

	// compute gradient using L2 distance (g = hypot(gx, gy) and the maximum value
	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE gradientL2(const InputType* gx, const InputType* gy, OutputType*& g, size_t width, size_t height, size_t stride, OutputType* max = NULL)
	{
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::gradient<InputType, OutputType>(gx, gy, g, width, height, stride, max, true)));
		return COMPV_ERROR_CODE_S_OK;
	}

	// ret = clip(min, max, v*scale)
	template <typename InputType, typename ScaleType, typename OutputType>
	static COMPV_ERROR_CODE scaleAndClip(const InputType* in, const ScaleType scale, OutputType*& out, OutputType min, OutputType max, size_t width, size_t height, size_t stride)
	{
		COMPV_CHECK_EXP_RETURN(!in || !width || !height || stride < width || max < min, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (!out) {
			out = (OutputType*)CompVMem::malloc(height * stride * sizeof(OutputType));
			COMPV_CHECK_EXP_RETURN(!out, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}

		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
		OutputType* out_ = out;
		size_t i, j;
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				out_[i] = (OutputType)COMPV_MATH_CLIP3(min, max, (in[i] * scale));
			}
			out_ += stride;
			in += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename T>
	static COMPV_INLINE T hypot(T x, T y) {
#if 1
		return (T)::hypot(x, y);
#elif 0
		// https://en.wikipedia.org/wiki/Hypot
		// Without overflow / underflow
		T t;
		x = COMPV_MATH_ABS(x);
		y = COMPV_MATH_ABS(y);
		t = COMPV_MATH_MIN(x, y);
		x = COMPV_MATH_MAX(x, y);
		t = t / x;
		return x * COMPV_MATH_SQRT(1 + t*t);
#endif
	}

private:
		template <typename InputType, typename OutputType>
		static COMPV_ERROR_CODE addAbs_C(const InputType* a, const InputType* b, OutputType*& r, size_t width, size_t height, size_t stride)
		{
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
			OutputType* r_ = r;
			size_t i, j;
			for (j = 0; j < height; ++j) {
				for (i = 0; i < width; ++i) {
					r_[i] = (OutputType)(COMPV_MATH_ABS(a[i]) + COMPV_MATH_ABS(b[i]));
				}
				r_ += stride;
				a += stride;
				b += stride;
			}
			return COMPV_ERROR_CODE_S_OK;
		}

		template <typename InputType, typename OutputType>
		static COMPV_ERROR_CODE gradient(const InputType* gx, const InputType* gy, OutputType*& g, size_t width, size_t height, size_t stride, OutputType* max = NULL, bool L1 = true)
		{
			COMPV_CHECK_EXP_RETURN(!gx || !gy || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			if (!g) {
				g = (OutputType*)CompVMem::malloc(height * stride * sizeof(OutputType));
				COMPV_CHECK_EXP_RETURN(!g, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			}
			
			size_t i, j;
			OutputType* g_ = g;
			if (L1) {
				COMPV_CHECK_CODE_RETURN((CompVMathUtils::addAbs<InputType, OutputType>(gx, gy, g, width, height, stride)));
			}
			else {
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
				for (j = 0; j < height; ++j) {
					for (i = 0; i < width; ++i) {
						g_[i] = (OutputType)hypot(gx[i], gy[i]);
					}
					g_ += stride;
					gx += stride;
					gy += stride;
				}
			}
			if (max) {
				COMPV_CHECK_CODE_RETURN(CompVMathUtils::max<OutputType>(g, width, height, stride, *max));
			}
			return COMPV_ERROR_CODE_S_OK;
		}

private:
    static bool s_Initialized;
    static compv_scalar_t(*maxValFunc)(compv_scalar_t a, compv_scalar_t b);
    static compv_scalar_t(*minValFunc)(compv_scalar_t a, compv_scalar_t b);
    static int32_t(*minArrayI32Func)(const int32_t* array, compv_scalar_t count);
    static compv_scalar_t(*clip3Func)(compv_scalar_t min, compv_scalar_t max, compv_scalar_t val);
    static compv_scalar_t(*clip2Func)(compv_scalar_t max, compv_scalar_t val);
	static void(*randFunc)(uint32_t *r, compv_scalar_t count);
};

extern template COMPV_ERROR_CODE CompVMathUtils::addAbs(const int16_t* a, const int16_t* b, uint16_t*& r, size_t width, size_t height, size_t stride);

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATHUTILS_H_ */
