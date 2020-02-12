/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_EXP_H_)
#define _COMPV_BASE_MATH_EXP_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVMathExp
{
public:
	static COMPV_ERROR_CODE exp(const CompVMatPtr &in, CompVMatPtrPtr out);
	static COMPV_ERROR_CODE hookExp_64f(
		void(**CompVMathExpExp_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f),
		int* minpack = nullptr
	);
	static bool isInitialized() {
		return s_bInitialized;
	}
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	static const uint64_t* lut64u() {
		return s_arrayLut64u;
	}
	static const uint32_t* lut32u() {
		return s_arrayLut32u;
	}
	static const uint64_t* vars64u() {
		return s_arrayVars64u;
	}
	static const compv_float64_t* vars64f() {
		return s_arrayVars64f;
	}
	static const compv_float32_t* vars32f() {
		return s_arrayVars32f;
	}

private:
	static bool s_bInitialized;
	static COMPV_ALIGN_DEFAULT() const uint64_t s_arrayVars64u[2];
	static COMPV_ALIGN_DEFAULT() const compv_float64_t s_arrayVars64f[8];
	static COMPV_ALIGN_DEFAULT() const compv_float32_t s_arrayVars32f[5];
	static COMPV_ALIGN_DEFAULT() const uint64_t s_arrayLut64u[2048 /* 1UL << sbit[11] */];
	static COMPV_ALIGN_DEFAULT() const uint32_t s_arrayLut32u[1024 /* tbl */];
	
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_EXP_H_ */
