/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_grabber.h"
#include "compv/ds/compv_ds_utils.h"
#include "compv/base/image/compv_image.h"

#define COMPV_THIS_CLASSNAME "CompVDSGrabber"

COMPV_NAMESPACE_BEGIN()

//
//	CompVDSGrabber
//

CompVDSGrabber::CompVDSGrabber(CompVDSBufferCBFunc func COMPV_DEFAULT(NULL), const void* pcUserData COMPV_DEFAULT(NULL))
    : CUnknown(TEXT("CompVDSGrabber"), static_cast<LPUNKNOWN>(NULL))
	, m_eSubTypeNeg(COMPV_SUBTYPE_NONE)
	, m_guidSubTypeNeg(GUID_NULL)
{
	m_BufferCB.func = func;
	m_BufferCB.pcUserData = pcUserData;
}

CompVDSGrabber::~CompVDSGrabber()
{
    COMPV_CHECK_CODE_NOP(stop());
}

HRESULT STDMETHODCALLTYPE CompVDSGrabber::SampleCB(
    double SampleTime,
    IMediaSample *pSample) /* Overrides(ISampleGrabberCB)*/
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CompVDSGrabber::BufferCB(
    double SampleTime,
    BYTE *pBuffer,
    long BufferLen) /* Overrides(ISampleGrabberCB)*/
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!pBuffer || !BufferLen, E_POINTER);

	CompVDSBufferCBFunc BufferCB_func = m_BufferCB.func;
	const void* BufferCB_pcUserData = m_BufferCB.pcUserData;
	if (!BufferCB_func) {
		return S_OK;
	}

	HRESULT hr = S_OK;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	const CompVDSCameraCaps* capsNeg;

	// Get the negociated caps (camera connected MediaType)
	capsNeg = m_ptrGraphCapture->capsNeg();
	COMPV_CHECK_HRESULT_EXP_BAIL(!capsNeg, hr = E_POINTER);

	// Check if the foramt changed
	if (!InlineIsEqualGUID(m_guidSubTypeNeg, capsNeg->subType)) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Camera media type [%s] changed (%s -> %s), updating the format", capsNeg->toString().c_str(), CompVDSUtils::guidName(m_guidSubTypeNeg), CompVDSUtils::guidName(capsNeg->subType));
		COMPV_CHECK_CODE_BAIL(err = CompVDSUtils::convertSubType(capsNeg->subType, m_eSubTypeNeg));
		m_guidSubTypeNeg = capsNeg->subType;
	}
	
	// Forward the image data to the listeners
	COMPV_CHECK_CODE_BAIL(err = CompVImage::wrap(m_eSubTypeNeg, static_cast<const void*>(pBuffer), static_cast<size_t>(capsNeg->width), static_cast<size_t>(capsNeg->height), static_cast<size_t>(capsNeg->width), &m_ptrImageCB));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = BufferCB_func(m_ptrImageCB, BufferCB_pcUserData));

bail:
	if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
		return E_FAIL;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CompVDSGrabber::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) /*Overrides(IUnknown)*/
{
    COMPV_CHECK_HRESULT_CODE_RETURN(CUnknown::NonDelegatingQueryInterface(riid, ppvObject)); // base implementation
    return S_OK;
}

ULONG STDMETHODCALLTYPE CompVDSGrabber::AddRef(void) /*Overrides(IUnknown)*/
{
    return CUnknown::NonDelegatingAddRef(); // base implementation
}

ULONG STDMETHODCALLTYPE CompVDSGrabber::Release(void) /*Overrides(IUnknown)*/
{
    return CUnknown::NonDelegatingRelease(); // base implementation
}

COMPV_ERROR_CODE CompVDSGrabber::start(const std::string& deviceId, const CompVDSCameraCaps& caps)
{
    if (!m_ptrGraphCapture) {
        COMPV_CHECK_CODE_RETURN(CompVDSGraphCapture::newObj(&m_ptrGraphCapture, this));
    }
    COMPV_CHECK_CODE_RETURN(m_ptrGraphCapture->start(deviceId, caps));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSGrabber::stop()
{
    if (m_ptrGraphCapture) {
        COMPV_ERROR_CODE err;
        COMPV_CHECK_CODE_NOP(err = m_ptrGraphCapture->stop());
        return err;
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
