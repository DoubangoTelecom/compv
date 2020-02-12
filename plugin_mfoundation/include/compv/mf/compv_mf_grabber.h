/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_MFOUNDATION_GRABBER_H_
#define _COMPV_PLUGIN_MFOUNDATION_GRABBER_H_

#include "compv/mf/compv_mf_config.h"

#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//      Sample Grabber callback [Declaration]
//      http://msdn.microsoft.com/en-us/library/windows/desktop/hh184779(v=vs.85).aspx
//
class CompVMFGrabberCB : public IMFSampleGrabberSinkCallback
{
	bool m_bMuted;
	long m_cRef;
	struct {
		CompVMFBufferCBFunc func;
		const void* pcUserData;
	} m_BufferCB;

	CompVMFGrabberCB(CompVMFBufferCBFunc cbFun, const void* cbUserData);

public:
	virtual ~CompVMFGrabberCB();
	static HRESULT CreateInstance(CompVMFBufferCBFunc cbFun, const void* cbUserData, CompVMFGrabberCB **ppCB);

	void SetMute(bool bMuted) {
		m_bMuted = bMuted;
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFClockStateSink methods
	STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
	STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
	STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
	STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
	STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

	// IMFSampleGrabberSinkCallback methods
	STDMETHODIMP OnSetPresentationClock(IMFPresentationClock* pClock);
	STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
		LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
		DWORD dwSampleSize);
	STDMETHODIMP OnShutdown();
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_MFOUNDATION_GRABBER_H_ */

