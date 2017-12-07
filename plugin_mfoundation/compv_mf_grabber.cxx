/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/mf/compv_mf_grabber.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_mem.h"

#include <new>
#include <shlwapi.h>

#define COMPV_THIS_CLASSNAME "CompVMFGrabberCB"

COMPV_NAMESPACE_BEGIN()

// Create a new instance of the object.
HRESULT CompVMFGrabberCB::CreateInstance(CompVMFBufferCBFunc cbFun, const void* cbUserData, CompVMFGrabberCB **ppCB)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppCB, E_POINTER);
	*ppCB = new (std::nothrow) CompVMFGrabberCB(cbFun, cbUserData);
	COMPV_CHECK_HRESULT_EXP_RETURN(!*ppCB, E_OUTOFMEMORY);
	return S_OK;
}

CompVMFGrabberCB::CompVMFGrabberCB(CompVMFBufferCBFunc cbFun, const void* cbUserData)
	: m_cRef(1)
	, m_bMuted(false)
{
	m_BufferCB.func = cbFun;
	m_BufferCB.pcUserData = cbUserData;
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%p, %p)", __FUNCTION__, cbFun, cbUserData);
}

CompVMFGrabberCB::~CompVMFGrabberCB()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
}

STDMETHODIMP CompVMFGrabberCB::QueryInterface(REFIID riid, void** ppv)
{
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4838)
	static const QITAB qit[] = {
		QITABENT(CompVMFGrabberCB, IMFSampleGrabberSinkCallback),
		QITABENT(CompVMFGrabberCB, IMFClockStateSink),
		{ 0 }
	};
	COMPV_VS_DISABLE_WARNINGS_END()
	return QISearch(this, qit, riid, ppv); // Do not check the return, expected to fail when interface doesn't exist
}

STDMETHODIMP_(ULONG) CompVMFGrabberCB::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CompVMFGrabberCB::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (cRef == 0) {
		delete this;
	}
	return cRef;

}

// IMFClockStateSink methods.

// In these example, the IMFClockStateSink methods do not perform any actions.
// You can use these methods to track the state of the sample grabber sink.

STDMETHODIMP CompVMFGrabberCB::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%lld, %lld)", __FUNCTION__, hnsSystemTime, llClockStartOffset);
	return S_OK;
}

STDMETHODIMP CompVMFGrabberCB::OnClockStop(MFTIME hnsSystemTime)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%lld)", __FUNCTION__, hnsSystemTime);
	return S_OK;
}

STDMETHODIMP CompVMFGrabberCB::OnClockPause(MFTIME hnsSystemTime)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%lld)", __FUNCTION__, hnsSystemTime);
	return S_OK;
}

STDMETHODIMP CompVMFGrabberCB::OnClockRestart(MFTIME hnsSystemTime)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%lld)", __FUNCTION__, hnsSystemTime);
	return S_OK;
}

STDMETHODIMP CompVMFGrabberCB::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%lld)", __FUNCTION__, hnsSystemTime);
	return S_OK;
}

// IMFSampleGrabberSink methods.

STDMETHODIMP CompVMFGrabberCB::OnSetPresentationClock(IMFPresentationClock* pClock)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%p)", __FUNCTION__, pClock);
	return S_OK;
}

STDMETHODIMP CompVMFGrabberCB::OnProcessSample(
	REFGUID guidMajorMediaType, DWORD dwSampleFlags,
	LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
	DWORD dwSampleSize)
{
	if (m_BufferCB.func) {
		if (m_bMuted) {
			// Send zeros. Do not skip sending data to avoid NAT issues and session deconnection.
			// Some TelePresence systems disconnect the session when the remote peer stops sending video data.
			CompVMem::set((void*)pSampleBuffer, 0, static_cast<size_t>(dwSampleSize));
		}
		COMPV_CHECK_HRESULT_CODE_RETURN(m_BufferCB.func(guidMajorMediaType, dwSampleFlags,
			llSampleTime, llSampleDuration, pSampleBuffer,
			dwSampleSize, m_BufferCB.pcUserData));
	}

	return S_OK;
}

STDMETHODIMP CompVMFGrabberCB::OnShutdown()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	return S_OK;
}

COMPV_NAMESPACE_END()
