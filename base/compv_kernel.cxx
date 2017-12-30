/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_kernel.h"
#include "compv/base/math/compv_math_convlt.h"

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVKernel::mean(const size_t& blockSize, CompVMatPtrPtr kernel)
{
	COMPV_CHECK_EXP_RETURN(!kernel || !(blockSize & 1), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr ptr32fNormalizedKernl;
	const compv_float32_t vvv = 1.f / static_cast<compv_float32_t>(blockSize);
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&ptr32fNormalizedKernl, 1, blockSize));
	compv_float32_t* ptr32fNormalizedKernlPtr = ptr32fNormalizedKernl->ptr<compv_float32_t>();
	for (size_t i = 0; i < blockSize; ++i) {
		ptr32fNormalizedKernlPtr[i] = vvv;
	}
	// Create fixed point mean kernel. No need for floating point version because mean processing fxp version is largely enough.
	COMPV_CHECK_CODE_RETURN(CompVMathConvlt::fixedPointKernel(ptr32fNormalizedKernl, kernel));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
