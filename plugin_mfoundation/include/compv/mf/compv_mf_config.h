/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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

#define COMPV_CHECK_HRESULT_CODE_NOP(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); } } while(0)
#define COMPV_CHECK_HRESULT_CODE_BAIL(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); goto bail; } } while(0)
#define COMPV_CHECK_HRESULT_CODE_RETURN(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); return __hr__; } } while(0)
#define COMPV_CHECK_HRESULT_CODE_ASSERT(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); COMPV_ASSERT(false); } } while(0)
#define COMPV_CHECK_HRESULT_EXP_NOP(exp, hr, ...) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_NOP(hr,  ##__VA_ARGS__); } while(0)
#define COMPV_CHECK_HRESULT_EXP_RETURN(exp, hr, ...) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_RETURN(hr,  ##__VA_ARGS__); } while(0)
#define COMPV_CHECK_HRESULT_EXP_BAIL(exp, hr, ...) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_BAIL(hr,  ##__VA_ARGS__); } while(0)

// Video processor (https://msdn.microsoft.com/en-us/library/windows/desktop/hh162913(v=vs.85).aspx) not whanted for several reasons
// - Not supportted on Win7 (but this is not a real issue as the code can detect it)
// - Resize the video to match the aspect ratio. Not wanted as we already use OpenGL for this.
// - Duplicate frames to match the requested framerate when requested fps > supported fps
// Because of all these reasons, we'll use separate resizer (https://msdn.microsoft.com/en-us/library/windows/desktop/ff819491(v=vs.85).aspx),
// frame rate converter (https://msdn.microsoft.com/en-us/library/windows/desktop/ff819100(v=vs.85).aspx) and color converter (https://msdn.microsoft.com/en-us/library/windows/desktop/ff819079(v=vs.85).aspx)
#if !defined COMPV_MF_USE_PROCESSOR_IN_TOPO
#	define COMPV_MF_USE_PROCESSOR_IN_TOPO 0
#endif

typedef GUID COMPV_MF_SUBTYPE;

extern const char* CompVMFUtilsGuidName(const GUID& guid);

typedef HRESULT(STDMETHODCALLTYPE *CompVMFBufferCBFunc)(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
	LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
	DWORD dwSampleSiz, const void *pcUserData);

struct CompVMFCameraCaps {
	UINT32 width;
	UINT32 height;
	UINT32 numFps;
	UINT32 denFps;
	COMPV_MF_SUBTYPE subType;
	// Important: update 'isEquals' and 'toString' functions if you add new field
	BOOL autofocus;

	CompVMFCameraCaps(
		UINT32 width_ = 640, 
		UINT32 height_ = 480, 
		UINT32 numFps_ = 25000, 
		UINT32 denFps_ = 1000, 
		COMPV_MF_SUBTYPE subType_ = MFVideoFormat_YUY2, 
		BOOL autofocus_ = TRUE) 
	{
		width = width_;
		height = height_;
		numFps = numFps_;
		denFps = denFps_;
		subType = subType_;
		autofocus = autofocus_;
	}
	virtual ~CompVMFCameraCaps() {
	}

	COMPV_INLINE bool isEquals(const CompVMFCameraCaps& caps)const {
		return width == caps.width && height == caps.height && numFps == caps.numFps && denFps == caps.denFps && InlineIsEqualGUID(subType, caps.subType) && autofocus == caps.autofocus;
	}

	COMPV_INLINE const std::string toString()const {
		return
			std::string("width=") + std::to_string(width) + std::string(", ")
			+ std::string("height=") + std::to_string(height) + std::string(", ")
			+ std::string("fps=") + std::to_string(numFps) + std::string("/") + std::to_string(denFps) + std::string(", ")
			+ std::string("subType=") + std::string(CompVMFUtilsGuidName(subType))
			+ std::string("autofocus=") + std::to_string(autofocus);
	}
};

#endif /* _COMPV_PLUGIN_MFOUNDATION_CONFIG_H_ */
