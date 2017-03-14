/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_DISTANCE_H_)
#define _COMPV_BASE_MATH_DISTANCE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"
#include "compv/base/math/compv_math_utils.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathDistance
{
public:
	static COMPV_ERROR_CODE hamming(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr);

	// r = abs(a) + abs(b)
	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE l1(const InputType* a, const InputType* b, OutputType* r, size_t width, size_t height, size_t stride) {
		COMPV_CHECK_CODE_RETURN((CompVMathUtils::sumAbs<InputType, OutputType>(a, b, r, width, height, stride)));
		return COMPV_ERROR_CODE_S_OK;
	}

	// r = sqrt((a^2) + (b^2)) = hypot(a, b)
	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE l2(const InputType* a, const InputType* b, OutputType* r, size_t width, size_t height, size_t stride) {
		COMPV_CHECK_CODE_RETURN((CompVMathDistance::l2_C<InputType, OutputType>(a, b, r, width, height, stride)));
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	template <typename InputType, typename OutputType>
	static COMPV_ERROR_CODE l2_C(const InputType* a, const InputType* b, OutputType* r, size_t width, size_t height, size_t stride) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");
		for (size_t j = 0; j < height; ++j) {
			for (size_t i = 0; i < width; ++i) {
				r[i] = static_cast<OutputType>(CompVMathUtils::hypot(a[i], b[i]));
			}
			a += stride;
			b += stride;
			r += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_DISTANCE_H_ */

