/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_CAST_H_)
#define _COMPV_BASE_MATH_CAST_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathCast
{
public:
	template<typename srcType, typename dstType>
	static COMPV_ERROR_CODE process_static(const CompVMatPtr& src, CompVMatPtrPtr dst)
	{
		COMPV_CHECK_EXP_RETURN(!src || !src->isRawTypeMatch<srcType>() || !dst, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		if (std::is_same<srcType, dstType>::value) {
			COMPV_DEBUG_INFO_EX("CompVMathCast", "This is just a useless call. Are you drunk?")
			COMPV_CHECK_CODE_RETURN(src->clone(dst));
			return COMPV_ERROR_CODE_S_OK;
		}
		// At this step we're sure the mat contains a raw type (int, float ...)
		const size_t width = src->cols();
		const size_t height = src->rows();
		const size_t stride = src->stride();
		CompVMatPtr dst_ = (*dst == src)? nullptr : *dst; // "src" must not be equal to "dst" because sizeof(srcType) most likely != sizeof(dstType)
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj<dstType>(&dst_, height, width, src->alignment(), stride));
		
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<srcType, dstType>(
			src->ptr<const srcType>(), dst_->ptr<dstType>(), width, height, stride
			)));
		*dst = *dst_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	template<typename srcType, typename dstType>
	static COMPV_ERROR_CODE process_static(const srcType* src, dstType* dst, const size_t width, const size_t height, const size_t stride)	{
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static_C<srcType, dstType>(src, dst, width, height, stride)));
		return COMPV_ERROR_CODE_S_OK;
	}

	template<typename srcType, typename dstType>
	static COMPV_ERROR_CODE process_static_C(const srcType* src, dstType* dst, const size_t width, const size_t height, const size_t stride)
	{
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
		for (size_t j = 0; j < height; ++j) {
			for (size_t i = 0; i < width; ++i) {
				dst[i] = static_cast<dstType>(src[i]);
			}
			dst += stride;
			src += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
};

COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathCast::process_static(const compv_float32_t* src, compv_float64_t* dst, const size_t width, const size_t height, const size_t stride);
COMPV_TEMPLATE_EXTERN COMPV_BASE_API COMPV_ERROR_CODE CompVMathCast::process_static(const compv_float64_t* src, compv_float32_t* dst, const size_t width, const size_t height, const size_t stride);

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_CAST_H_ */
