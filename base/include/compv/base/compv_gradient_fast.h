/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_GRADIENT_FAST_H_)
#define _COMPV_BASE_GRADIENT_FAST_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_base.h"
#include "compv/base/compv_mat.h"
#include "compv/base/math/compv_math_cast.h"

COMPV_NAMESPACE_BEGIN()

// Gradient computation using [-1, 0, 1] kernel without convolution (no mul, add/sub only)
class COMPV_BASE_API CompVGradientFast
{
public:
	template <typename OutType> static COMPV_ERROR_CODE gradX(const CompVMatPtr& input, CompVMatPtrPtr outputX) {
		COMPV_CHECK_EXP_RETURN(!input || input->planeCount() != 1 || !outputX || (input->elmtInBytes() != sizeof(uint8_t) && !input->isRawTypeMatch<compv_float32_t>()), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (input->isRawTypeMatch<compv_float32_t>()) {
			COMPV_CHECK_CODE_RETURN(gradX_32f32f(input, outputX));
		}
		else if (input->elmtInBytes() == sizeof(uint8_t)) {
			if (std::is_same<OutType, compv_float32_t>::value) {
				COMPV_CHECK_CODE_RETURN(gradX_8u32f(input, outputX));
			}
			else {
				COMPV_CHECK_CODE_RETURN(gradX_8u16s(input, outputX));
				if (!std::is_same<OutType, int16_t>::value) {
					COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<int16_t, OutType>(*outputX, outputX)));
				}
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	template <typename OutType> static COMPV_ERROR_CODE gradY(const CompVMatPtr& input, CompVMatPtrPtr outputY) {
		COMPV_CHECK_EXP_RETURN(!input || input->planeCount() != 1 || !outputY || (input->elmtInBytes() != sizeof(uint8_t) && !input->isRawTypeMatch<compv_float32_t>()), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (input->isRawTypeMatch<compv_float32_t>()) {
			COMPV_CHECK_CODE_RETURN(gradY_32f32f(input, outputY));
		}
		else if (input->elmtInBytes() == sizeof(uint8_t)) {
			if (std::is_same<OutType, compv_float32_t>::value) {
				COMPV_CHECK_CODE_RETURN(gradY_8u32f(input, outputY));
			}
			else {
				COMPV_CHECK_CODE_RETURN(gradY_8u16s(input, outputY));
				if (!std::is_same<OutType, int16_t>::value) {
					COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<int16_t, OutType>(*outputY, outputY)));
				}
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE magnitude(const CompVMatPtr& input, CompVMatPtrPtr mag);
	static COMPV_ERROR_CODE magnitude(const CompVMatPtr& gradX, const CompVMatPtr& gradY, CompVMatPtrPtr mag);
	static COMPV_ERROR_CODE direction(const CompVMatPtr& input, CompVMatPtrPtr dir, const bool angleInDeg);
	static COMPV_ERROR_CODE direction(const CompVMatPtr& gradX, const CompVMatPtr& gradY, CompVMatPtrPtr dir, const bool angleInDeg);

private:
	static COMPV_ERROR_CODE gradX_8u16s(const CompVMatPtr& input, CompVMatPtrPtr outputX);
	static COMPV_ERROR_CODE gradX_8u32f(const CompVMatPtr& input, CompVMatPtrPtr outputX);
	static COMPV_ERROR_CODE gradX_32f32f(const CompVMatPtr& input, CompVMatPtrPtr outputX);
	static COMPV_ERROR_CODE gradY_8u16s(const CompVMatPtr& input, CompVMatPtrPtr outputY);
	static COMPV_ERROR_CODE gradY_8u32f(const CompVMatPtr& input, CompVMatPtrPtr outputY);
	static COMPV_ERROR_CODE gradY_32f32f(const CompVMatPtr& input, CompVMatPtrPtr outputY);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_GRADIENT_FAST_H_ */
