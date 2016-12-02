/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_grabber.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVDSGrabber
//

CompVDSGrabber::CompVDSGrabber()
	: CUnknown(TEXT("CompVDSGrabber"), static_cast<LPUNKNOWN>(NULL))
{

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
#if 0
	const AM_MEDIA_TYPE& mediaType = m_ptrGraphCapture->connectedMediaType();
	VIDEOINFOHEADER *pVih = NULL;
	BITMAPINFOHEADER* bih = NULL;

	// Examine the format block.
	if ((mediaType.formattype == FORMAT_VideoInfo) && (mediaType.cbFormat >= sizeof(VIDEOINFOHEADER)) && (mediaType.pbFormat != NULL)) {
		pVih = reinterpret_cast<VIDEOINFOHEADER *>(mediaType.pbFormat);
		bih = &pVih->bmiHeader;
	}

	if (mediaType.subtype == MEDIASUBTYPE_YUY2) {
		COMPV_DEBUG_INFO("MEDIASUBTYPE_YUY2");
	}
#endif

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

COMPV_ERROR_CODE CompVDSGrabber::start(const std::string& deviceId COMPV_DEFAULT(""))
{
	if (!m_ptrGraphCapture) {
		COMPV_CHECK_CODE_RETURN(CompVDSGraphCapture::newObj(&m_ptrGraphCapture, this));
	}
	COMPV_CHECK_CODE_RETURN(m_ptrGraphCapture->start(deviceId));
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
