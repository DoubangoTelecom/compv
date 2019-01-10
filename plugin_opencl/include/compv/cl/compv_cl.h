/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CL_H_)
#define _COMPV_CL_H_

#include "compv/cl/compv_cl_config.h"
#include "compv/base/compv_common.h"

#include <CL/opencl.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class COMPV_CL_API CompVCL
{
public:
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	static COMPV_INLINE bool isInitialized() {
		return s_bInitialized;
	}
	static COMPV_INLINE bool isFloat64Supported() {
		return s_cl_khr_fp64_supported;
	}
	static COMPV_INLINE cl_platform_id clPlatformId() {
		return s_clPlatformId;
	}
	static COMPV_INLINE cl_device_id clDeviceId() {
		return s_clDeviceId;
	}

private:


private:
	static bool s_bInitialized;
	static bool s_cl_khr_fp64_supported;
	static cl_platform_id s_clPlatformId;
	static cl_device_id s_clDeviceId;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CL_H_ */
