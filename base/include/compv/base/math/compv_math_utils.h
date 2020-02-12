/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_UTILS_H_)
#define _COMPV_BASE_MATH_UTILS_H_

#include "compv/base/compv_config.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_cpu.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathUtils
{
public:
    static COMPV_ERROR_CODE init();
    static COMPV_INLINE int32_t clamp(int32_t min, int32_t val, int32_t max) { // do not use macro, otherwise 'val' which coulbe a function will be evaluated several times
        // Important: Do not use CLIP3_INT here: very slow on Windows (tested on Win8 core i7 using VS2013)
        return COMPV_MATH_CLIP3(min, max, val);
    }

	template <typename InputType = int16_t>
    static COMPV_INLINE uint8_t clampPixel8(InputType val) {
        // Important: Do not use CLIP3_INT here: very slow on Windows (tested on Win8 core i7 using VS2013)
        // Also, asm_cmov_clip2() is sloow -> To be checked
        return static_cast<uint8_t>(COMPV_MATH_CLIP3(0, 255, val));
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

    template <typename OutputType>
    static COMPV_INLINE OutputType roundFloatUnsigned(float f) {
        return static_cast<OutputType>(roundFloatUnsignedFunc(f));
    }
    template <typename OutputType>
    static COMPV_INLINE OutputType roundFloatSigned(float f) {
        return static_cast<OutputType>(roundFloatSignedFunc(f));
    }

    template <typename InputType>
    static COMPV_ERROR_CODE max(const InputType* data, size_t width, size_t height, size_t stride, InputType &max) {
        COMPV_CHECK_EXP_RETURN(!data || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		return CompVMathUtils::max_C<InputType>(data, width, height, stride, max);
    }

    template <typename InputType>
    static COMPV_ERROR_CODE mean(const InputType* data, size_t count, InputType &mean) {
        COMPV_CHECK_EXP_RETURN(!data || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        return CompVMathUtils::mean_C<InputType>(data, count, mean);
    }

    template <typename InputType>
    static COMPV_ERROR_CODE mean(const InputType* data, size_t width, size_t height, size_t stride, InputType &mean) {
        COMPV_CHECK_EXP_RETURN(!data || !width || !height || !stride, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
        InputType* means = reinterpret_cast<InputType*>(CompVMem::malloc(height * sizeof(InputType)));
        COMPV_CHECK_EXP_BAIL(!means, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
        for (size_t j = 0; j < height; ++j) {
            CompVMathUtils::mean<InputType>(data, width, means[j]);
            data += stride;
        }
        CompVMathUtils::mean<InputType>(means, height, mean);
	bail:
        CompVMem::free((void**)&means);
        return err;
    }

    // Function used to compute L1 distance
    // r = abs(a) + abs(b)
    template <typename InputType, typename OutputType>
    static COMPV_ERROR_CODE sumAbs(const InputType* a, const InputType* b, OutputType* r, size_t width, size_t height, size_t stride) {
        COMPV_CHECK_EXP_RETURN(!a || !b || !r || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_CHECK_CODE_RETURN((CompVMathUtils::sumAbs_C<InputType, OutputType>(a, b, r, width, height, stride)));
        return COMPV_ERROR_CODE_S_OK;
    }

	// sum<uint8_t, uint32_t> would work without overflow upto ~160Mo data. For other kind of sums you should use 'sum2'
    template <typename InputType, typename OutputType>
    static COMPV_ERROR_CODE sum(const InputType* a, size_t width, size_t height, size_t stride, OutputType &r) {
        COMPV_CHECK_EXP_RETURN(!a || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum_C<InputType, OutputType>(a, width, height, stride, r)));
        return COMPV_ERROR_CODE_S_OK;
    }

    template <typename InputType, typename OutputType>
    static COMPV_ERROR_CODE sum2(const InputType* a, const InputType* b, OutputType* s, size_t width, size_t height, size_t stride) {
        COMPV_CHECK_EXP_RETURN(!a || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum2_C<InputType, OutputType>(a, b, s, width, height, stride)));
        return COMPV_ERROR_CODE_S_OK;
    }

	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE atan2(const InputType* y, const InputType* x, OutputType* r, size_t width, size_t height, size_t stride) {
		COMPV_CHECK_EXP_RETURN(!y || !x || !r || !width || !height || stride < width, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::atan2_C<InputType, OutputType>(y, x, r, width, height, stride)));
		return COMPV_ERROR_CODE_S_OK;
	}

    // ret = clip(min, max, v*scale)
    template <typename InputType, typename ScaleType, typename OutputType>
    static COMPV_ERROR_CODE scaleAndClip(const InputType* in, const ScaleType scale, OutputType*& out, OutputType min, OutputType max, size_t width, size_t height, size_t stride) {
        COMPV_CHECK_EXP_RETURN(!in || !width || !height || stride < width || max < min, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        if (!out) {
            out = reinterpret_cast<OutputType*>(CompVMem::malloc(height * stride * sizeof(OutputType)));
            COMPV_CHECK_EXP_RETURN(!out, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        }
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip_C<InputType, ScaleType, OutputType>(in, scale, out, min, max, width, height, stride)));
        return COMPV_ERROR_CODE_S_OK;
    }

	// ret = clip(0, 255, v*scale)
	template <typename InputType, typename ScaleType>
	static COMPV_ERROR_CODE scaleAndClipPixel8(const InputType* in, const ScaleType scale, uint8_t*& out, size_t width, size_t height, size_t stride) {
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::scaleAndClip<InputType, ScaleType, uint8_t>(in, scale, out, 0, 255, width, height, stride)));
		return COMPV_ERROR_CODE_S_OK;
	}

    template <typename T>
    static COMPV_INLINE T hypot(T x, T y) {
#if 1
        return static_cast<T>(::hypot(x, y));
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
	template <typename InputType>
	static COMPV_ERROR_CODE max_C(const InputType* data, size_t width, size_t height, size_t stride, InputType &max) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
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

    template <typename InputType, typename OutputType>
    static COMPV_ERROR_CODE sumAbs_C(const InputType* a, const InputType* b, OutputType*& r, size_t width, size_t height, size_t stride) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
        OutputType* r_ = r;
        size_t i, j;
        for (j = 0; j < height; ++j) {
            for (i = 0; i < width; ++i) {
                r_[i] = static_cast<OutputType>(COMPV_MATH_ABS(a[i]) + COMPV_MATH_ABS(b[i]));
            }
            r_ += stride;
            a += stride;
            b += stride;
        }
        return COMPV_ERROR_CODE_S_OK;
    }

    template <typename InputType, typename OutputType>
    static COMPV_ERROR_CODE sum_C(const InputType* a, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, OutputType &r) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
        r = 0;
		for (size_t j = 0; j < height; ++j) {
			for (size_t i = 0; i < width; ++i) {
				r += a[i];
			}
			a += stride;
		}
        return COMPV_ERROR_CODE_S_OK;
    }

    template <typename InputType, typename OutputType>
    static COMPV_ERROR_CODE sum2_C(const InputType* a, const InputType* b, OutputType* s, size_t width, size_t height, size_t stride) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
        size_t i, j;
        for (j = 0; j < height; ++j) {
            for (i = 0; i < width; ++i) {
                s[i] = a[i] + b[i];
            }
            a += stride;
            b += stride;
            s += stride;
        }
        return COMPV_ERROR_CODE_S_OK;
    }

    template <typename InputType>
    static COMPV_ERROR_CODE mean_C(const InputType* data, size_t count, InputType &mean) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
        double sum;
        COMPV_CHECK_CODE_RETURN((CompVMathUtils::sum<InputType, double>(data, count, sum)));
        mean = (InputType)(sum / count);
        return COMPV_ERROR_CODE_S_OK;
    }

	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE atan2_C(const InputType* y, const InputType* x, OutputType* r, size_t width, size_t height, size_t stride) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		size_t i, j;
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				r[i] = static_cast<OutputType>(COMPV_MATH_ATAN2(static_cast<OutputType>(y[i]), static_cast<OutputType>(x[i])));
			}
			y += stride;
			x += stride;
			r += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// ret = clip(min, max, v*scale)
	template <typename InputType, typename ScaleType, typename OutputType>
	static COMPV_ERROR_CODE scaleAndClip_C(const InputType* in, const ScaleType scale, OutputType* out, OutputType min, OutputType max, size_t width, size_t height, size_t stride) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		OutputType inScaled;
		size_t i, j;
		for (j = 0; j < height; ++j) {
			for (i = 0; i < width; ++i) {
				inScaled = static_cast<OutputType>(in[i] * scale);
				out[i] = static_cast<OutputType>(COMPV_MATH_CLIP3(min, max, inScaled)); // SIMD: saturation
			}
			out += stride;
			in += stride;
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
    static int(*roundFloatUnsignedFunc)(float f);
    static int(*roundFloatSignedFunc)(float f);
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathUtils::max(const uint16_t* data, size_t width, size_t height, size_t stride, uint16_t &max);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathUtils::sumAbs(const int16_t* a, const int16_t* b, uint16_t* r, size_t width, size_t height, size_t stride);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathUtils::sum(const uint8_t* a, size_t width, size_t height, size_t stride, uint32_t &r);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathUtils::sum2(const int32_t* a, const int32_t* b, int32_t* s, size_t width, size_t height, size_t stride);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathUtils::sum2(const uint32_t* a, const uint32_t* b, uint32_t* s, size_t width, size_t height, size_t stride);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathUtils::mean(const uint8_t* data, size_t count, uint8_t &mean);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathUtils::scaleAndClip(const uint16_t* in, const compv_float32_t scale, uint8_t*& out, uint8_t min, uint8_t max, size_t width, size_t height, size_t stride);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_UTILS_H_ */
