/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CL_IMAGE_HYSTERESIS_H_)
#define _COMPV_CL_IMAGE_HYSTERESIS_H_

#include "compv/cl/compv_cl_config.h"
#include "compv/base/compv_mat.h"

#include <CL/opencl.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVCLHysteresis
{
public:
	static COMPV_ERROR_CODE process_8u8u(const CompVMatPtr& in, CompVMatPtrPtr out, const double& tLow, const double& tHigh);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CL_IMAGE_HYSTERESIS_H_ */
