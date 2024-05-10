/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GPU_H_)
#define _COMPV_GPU_H_

#include "compv/gpu/compv_gpu_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_sharedlib.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_GPU_API CompVGpu
{
public:
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE deInit();
	static COMPV_ERROR_CODE setEnabled(bool enabled);
	static COMPV_INLINE bool isInitialized() {
		return s_bInitialized;
	}
	static COMPV_INLINE bool isActive() {
		return s_bActive;
	}
	static COMPV_INLINE bool isEnabled() {
		return s_bEnabled;
	}
	static COMPV_INLINE bool isActiveAndEnabled() {
		return isActive() && isEnabled();
	}
	static COMPV_INLINE bool isCudaSupported() {
		return s_bCudaSupported;
	}

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static bool s_bInitialized;
	static bool s_bActive;
	static bool s_bEnabled;
	static bool s_bCudaSupported;
	static CompVSharedLibPtr s_ptrImpl;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_H_ */
