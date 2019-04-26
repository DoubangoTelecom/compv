/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_clip.h"
#include "compv/base/compv_generic_invoke.h"

COMPV_NAMESPACE_BEGIN()

template<typename T>
static COMPV_ERROR_CODE CompVMathClipClip3Generic(const CompVMatPtr& in, const double minn, const double maxx, CompVMatPtrPtr out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	COMPV_CHECK_EXP_RETURN(!in || !in->isRawTypeMatch<T>() || !out || minn > maxx, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t height = in->rows();
	const size_t width = in->cols();
	const size_t stride = in->stride();

	const T minn_ = static_cast<T>(minn);
	const T maxx_ = static_cast<T>(maxx);

	if (*out != in) { // This function allows having input == output
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(out, height, width, stride));
	}
	const T* inPtr = in->ptr<const T>();
	T* outPtr = (*out)->ptr<T>();


	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			outPtr[i] = COMPV_MATH_CLIP3(minn_, maxx_, inPtr[i]); // TODO(dmi): Branchless SIMD -> max(minn, min(in[i], maxx))
		}
		inPtr += stride;
		outPtr += stride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathClip::clip3(const CompVMatPtr& in, const double minn, const double maxx, CompVMatPtrPtr out)
{
	COMPV_CHECK_EXP_RETURN(!in || in->planeCount() != 1 || !out || minn > maxx, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGenericInvokeCodeRawType(in->subType(), CompVMathClipClip3Generic, in, minn, maxx, out);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathClip::clip2(const CompVMatPtr& in, const double maxx, CompVMatPtrPtr out)
{
	COMPV_CHECK_CODE_RETURN(CompVMathClip::clip3(in, 0, maxx, out));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
