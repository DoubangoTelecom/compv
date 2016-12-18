/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_MFOUNDATION_CONFIG_H_
#define _COMPV_PLUGIN_MFOUNDATION_CONFIG_H_

#include "compv/camera/compv_camera_config.h"
#include "compv/base/compv_mat.h"

#include <mfapi.h>

// Windows's symbols export
#if COMPV_OS_WINDOWS
#	if defined(COMPV_PLUGIN_MFOUNDATION_EXPORTS)
# 		define COMPV_PLUGIN_MFOUNDATION_API		__declspec(dllexport)
#	else
# 		define COMPV_PLUGIN_MFOUNDATION_API		__declspec(dllimport)
#	endif
#else
#	define COMPV_PLUGIN_MFOUNDATION_API
#endif

#define COMPV_MF_SAFE_RELEASE(ppT) \
{ \
    if (*ppT) \
    { \
        (*ppT)->Release(); \
        *ppT = NULL; \
    } \
}

#define COMPV_CHECK_HRESULT_CODE_NOP(hr) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x)", __hr__); } } while(0)
#define COMPV_CHECK_HRESULT_CODE_BAIL(hr) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x)", __hr__); goto bail; } } while(0)
#define COMPV_CHECK_HRESULT_CODE_RETURN(hr) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x)", __hr__); return __hr__; } } while(0)
#define COMPV_CHECK_HRESULT_CODE_ASSERT(hr) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x)", __hr__); COMPV_ASSERT(false); } } while(0)
#define COMPV_CHECK_HRESULT_EXP_NOP(exp, hr) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_NOP(hr); } while(0)
#define COMPV_CHECK_HRESULT_EXP_RETURN(exp, hr) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_RETURN(hr); } while(0)
#define COMPV_CHECK_HRESULT_EXP_BAIL(exp, hr) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_BAIL(hr); } while(0)

extern const char* CompVMFUtilsGuidName(const GUID& guid);

typedef HRESULT(STDMETHODCALLTYPE *CompVMFBufferCBFunc)(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
	LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
	DWORD dwSampleSiz, const void *pcUserData);

struct CompVMFCameraCaps {
	LONG width;
	LONG height;
	int fps;
	GUID subType;
	// Important: update 'isEquals' and 'toString' functions if you add new field

	CompVMFCameraCaps(LONG width_ = 640, LONG height_ = 480, int fps_ = 25, GUID subType_ = MFVideoFormat_YUY2) {
		width = width_;
		height = height_;
		fps = fps_;
		subType = subType_;
	}

	COMPV_INLINE bool isEquals(const CompVMFCameraCaps& caps)const {
		return width == caps.width && height == caps.height && fps == caps.fps && InlineIsEqualGUID(subType, caps.subType);
	}

	COMPV_INLINE const std::string toString()const {
		return
			std::string("width=") + std::to_string(width) + std::string(", ")
			+ std::string("height=") + std::to_string(height) + std::string(", ")
			+ std::string("fps=") + std::to_string(fps) + std::string(", ")
			+ std::string("subType=") + std::string(CompVMFUtilsGuidName(subType));
	}
};

#endif /* _COMPV_PLUGIN_MFOUNDATION_CONFIG_H_ */
