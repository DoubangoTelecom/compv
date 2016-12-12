/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_grabber.h"
#include "compv/base/image/compv_image.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVDSGrabber
//

CompVDSGrabber::CompVDSGrabber(CompVDSBufferCBFunc func COMPV_DEFAULT(NULL), const void* pcUserData COMPV_DEFAULT(NULL))
    : CUnknown(TEXT("CompVDSGrabber"), static_cast<LPUNKNOWN>(NULL))
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
	
	// FIXME: nothing is correct here
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();

    const AM_MEDIA_TYPE& mediaType = m_ptrGraphCapture->connectedMediaType();
    VIDEOINFOHEADER *pVih = NULL;
    BITMAPINFOHEADER* bih = NULL;
	HRESULT hr = S_OK;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    // Examine the format block.
    if ((mediaType.formattype == FORMAT_VideoInfo) && (mediaType.cbFormat >= sizeof(VIDEOINFOHEADER)) && (mediaType.pbFormat != NULL)) {
        pVih = reinterpret_cast<VIDEOINFOHEADER *>(mediaType.pbFormat);
        bih = &pVih->bmiHeader;
    }

	//if (mediaType.subtype == MEDIASUBTYPE_YUY2) {
	//    COMPV_DEBUG_INFO("MEDIASUBTYPE_YUY2");
	//}

	// FIXME: chroma fixed
	COMPV_CHECK_CODE_BAIL(err = CompVImage::wrap(COMPV_SUBTYPE_PIXELS_YUY2, static_cast<const void*>(pBuffer), static_cast<size_t>(bih->biWidth), static_cast<size_t>(bih->biHeight), static_cast<size_t>(bih->biWidth), &m_ptrImageCB));
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
