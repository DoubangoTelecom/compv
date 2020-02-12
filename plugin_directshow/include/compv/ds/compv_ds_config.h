/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_DIRECTSHOW_CONFIG_H_
#define _COMPV_PLUGIN_DIRECTSHOW_CONFIG_H_

#include "compv/camera/compv_camera_config.h"
#include "compv/base/compv_mat.h"

#include <vector>

// Windows's symbols export
#if COMPV_OS_WINDOWS
#	if defined(COMPV_PLUGIN_DIRECTSHOW_EXPORTS)
# 		define COMPV_PLUGIN_DIRECTSHOW_API		__declspec(dllexport)
#	else
# 		define COMPV_PLUGIN_DIRECTSHOW_API		__declspec(dllimport)
#	endif
#else
#	define COMPV_PLUGIN_DIRECTSHOW_API			__attribute__((visibility("default")))
#endif

#define COMPV_DS_SAFE_RELEASE(x) if ((x)) { (x)->Release(); (x) = NULL; }
#define COMPV_DS_SAFE_DELETE_PTR(x) if ((x)) { delete (x); (x) = NULL; }

#define COMPV_DS_COCREATE(cls, iid, target) \
	CoCreateInstance(cls, NULL, CLSCTX_INPROC_SERVER, iid, reinterpret_cast<void**>(&target))
#define COMPV_DS_QUERY(source, iid, target) \
	source->QueryInterface(iid, reinterpret_cast<void**>(&target))

#define COMPV_DS_SEC_TO_100NS(SEC)  static_cast<REFERENCE_TIME>(10000000ui64) / static_cast<REFERENCE_TIME>((SEC))
#define COMPV_DS_100NS_TO_SEC(_100NS)  static_cast<REFERENCE_TIME>(10000000ui64) / static_cast<REFERENCE_TIME>((_100NS))

#define COMPV_CHECK_HRESULT_CODE_NOP(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); } } while(0)
#define COMPV_CHECK_HRESULT_CODE_BAIL(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); goto bail; } } while(0)
#define COMPV_CHECK_HRESULT_CODE_RETURN(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); return __hr__; } } while(0)
#define COMPV_CHECK_HRESULT_CODE_ASSERT(hr, ...) do { HRESULT __hr__ = (hr); if (FAILED(__hr__)) { COMPV_DEBUG_ERROR("Operation Failed (%08x) -> " ##__VA_ARGS__, __hr__); COMPV_ASSERT(false); } } while(0)
#define COMPV_CHECK_HRESULT_EXP_NOP(exp, hr, ...) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_NOP(hr,  ##__VA_ARGS__); } while(0)
#define COMPV_CHECK_HRESULT_EXP_RETURN(exp, hr, ...) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_RETURN(hr,  ##__VA_ARGS__); } while(0)
#define COMPV_CHECK_HRESULT_EXP_BAIL(exp, hr, ...) do { if ((exp)) COMPV_CHECK_HRESULT_CODE_BAIL(hr,  ##__VA_ARGS__); } while(0)

#include <streams.h>

#if defined(HAVE_QEDIT_H) /*|| (_WIN32_WINNT <= 0x0600)*/
#include <qedit.h>
#else
static const IID IID_ISampleGrabberCB = { 0x0579154A, 0x2B53, 0x4994,{ 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };

MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB :
public IUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE SampleCB(
        double SampleTime,
        IMediaSample *pSample) = 0;

    virtual HRESULT STDMETHODCALLTYPE BufferCB(
        double SampleTime,
        BYTE *pBuffer,
        long BufferLen) = 0;

};

static const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce,{ 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };

MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber :
public IUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(
        BOOL OneShot) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetMediaType(
        const AM_MEDIA_TYPE *pType) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(
        AM_MEDIA_TYPE *pType) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(
        BOOL BufferThem) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(
        /* [out][in] */ long *pBufferSize,
        /* [out] */ long *pBuffer) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(
        /* [retval][out] */ IMediaSample **ppSample) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetCallback(
        ISampleGrabberCB *pCallback,
        long WhichMethodToCallback) = 0;

};

struct __declspec(uuid("c1f400a0-3f08-11d3-9f0b-006008039e37"))
SampleGrabber;
// [ default ] interface ISampleGrabber
static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3,{ 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

static const CLSID CLSID_NullRenderer = { 0xC1F400A4, 0x3F08, 0x11d3,{ 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

#endif /* defined(HAVE_QEDIT) */

COMPV_NAMESPACE_BEGIN()

extern const char* CompVDSUtilsGuidName(const GUID& guid);

typedef HRESULT (STDMETHODCALLTYPE *CompVDSBufferCBFunc)(const CompVMatPtr image, const void *pcUserData);

struct CompVDSCameraDeviceInfo {
    std::string id;
    std::string name;
    std::string description;
    CompVDSCameraDeviceInfo(const std::string& id_, const std::string& name_, const std::string& description_) {
        id = id_;
        name = name_;
        description = description_;
    }
};

struct CompVDSCameraCaps {
	LONG width;
	LONG height;
	int fps;
	GUID subType;
	BOOL autofocus;
	// Important: update 'isEquals' and 'toString' functions if you add new field

	CompVDSCameraCaps(LONG width_ = 640, LONG height_ = 480, int fps_ = 25, GUID subType_ = MEDIASUBTYPE_YUY2, BOOL autofocus_ = TRUE) {
		width = width_;
		height = height_;
		fps = fps_;
		subType = subType_;
		autofocus = autofocus_;
	}

	COMPV_INLINE bool isEquals(const CompVDSCameraCaps& caps)const {
		return width == caps.width && height == caps.height && fps == caps.fps && InlineIsEqualGUID(subType, caps.subType) && autofocus == caps.autofocus;
	}

	COMPV_INLINE const std::string toString()const {
		return
			std::string("width=") + std::to_string(width) + std::string(", ")
			+ std::string("height=") + std::to_string(height) + std::string(", ")
			+ std::string("fps=") + std::to_string(fps) + std::string(", ")
			+ std::string("subType=") + std::string(CompVDSUtilsGuidName(subType))
			+ std::string("autofocus=") + std::to_string(autofocus);
	}
};

typedef std::vector<CompVDSCameraDeviceInfo > CompVDSCameraDeviceInfoList;

COMPV_NAMESPACE_END()


#endif /* _COMPV_PLUGIN_DIRECTSHOW_CONFIG_H_ */
