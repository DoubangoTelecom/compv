/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/mf/compv_mf_devices.h"
#include "compv/mf/compv_mf_utils.h"
#include "compv/base/compv_debug.h"

#include <new>
#include <mfidl.h>
#include <Mferror.h>
#include <shlwapi.h>

#define COMPV_THIS_CLASSNAME "CompVMFDeviceList"

COMPV_NAMESPACE_BEGIN()

CompVMFDeviceList::CompVMFDeviceList(bool bVideo)
	: m_bVideo(bVideo)
	, m_ppDevices(NULL)
	, m_cDevices(0)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
}

CompVMFDeviceList::~CompVMFDeviceList()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	clear();
}

UINT32 CompVMFDeviceList::count()const
{
	return m_cDevices;
}

void CompVMFDeviceList::clear()
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s", __FUNCTION__);
	for (UINT32 i = 0; i < m_cDevices; i++) {
		COMPV_MF_SAFE_RELEASE(&m_ppDevices[i]);
	}
	CoTaskMemFree(m_ppDevices);
	m_ppDevices = NULL;
	m_cDevices = 0;
}

HRESULT CompVMFDeviceList::enumerateDevices(const GUID& sourceType)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%s)", __FUNCTION__, CompVMFUtils::guidName(sourceType));
	HRESULT hr = S_OK;
	IMFAttributes *pAttributes = NULL;

	clear();

	// Initialize an attribute store. We will use this to
	// specify the enumeration parameters.
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFCreateAttributes(&pAttributes, 1));

	// Ask for source type = video capture devices
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pAttributes->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		sourceType
	));

	// Enumerate devices
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = MFEnumDeviceSources(pAttributes, &m_ppDevices, &m_cDevices));

bail:
	COMPV_MF_SAFE_RELEASE(&pAttributes);
	return hr;
}

HRESULT CompVMFDeviceList::deviceAtIndex(__in UINT32 index, __out IMFActivate **ppActivate)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%u)", __FUNCTION__, index);
	COMPV_CHECK_HRESULT_EXP_RETURN(!ppActivate || index >= count(), E_INVALIDARG);

	*ppActivate = m_ppDevices[index];
	(*ppActivate)->AddRef();

	return S_OK;
}

HRESULT CompVMFDeviceList::deviceWithId(__in const WCHAR *pszId, __out IMFActivate **ppActivate)
{
	// The only way for the end user to have valid device Ids is to call enumerateDevices first (using CompVMFUtils::devices()).
	// This means when (pszId != NULL && !empty(pszId)) then, count is > 0
	HRESULT hr = S_OK;
	if (!pszId || wcslen(pszId) == 0) { // No Id or empty string -> use default device
		if (count() == 0) { // count could be equal to zero if the user never called 'CompVMFUtils::devices()' and is trying to retrieve a default device
			COMPV_CHECK_HRESULT_CODE_RETURN(enumerateDevices(m_bVideo ? MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID : MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID));
		}
		COMPV_CHECK_HRESULT_CODE_RETURN(hr = deviceAtIndex(0, ppActivate));
		return hr;
	}
	WCHAR *_pszId = NULL;
	BOOL bFound = FALSE;
	UINT32 index = 0;
	for (UINT32 i = 0; i < count() && !bFound; ++i) {
		if ((SUCCEEDED(deviceId(i, &_pszId)))) {
			if (wcscmp(pszId, _pszId) == 0) {
				index = i;
				bFound = TRUE;
				// do not break the loop because we need to free(_pszId)
			}
		}
		if (_pszId) {
			CoTaskMemFree(_pszId), _pszId = NULL;
		}
	}
	COMPV_CHECK_HRESULT_CODE_RETURN(hr = deviceAtIndex(index, ppActivate));
	return hr;
}

// The caller must free the memory for the string by calling CoTaskMemFree
HRESULT CompVMFDeviceList::deviceName(__in UINT32 index, __out WCHAR **ppszName)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%u)", __FUNCTION__, index);
	COMPV_CHECK_HRESULT_EXP_RETURN(index >= count(), E_INVALIDARG);

	HRESULT hr;

	COMPV_CHECK_HRESULT_CODE_RETURN(hr = m_ppDevices[index]->GetAllocatedString(
		MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
		ppszName,
		NULL
	));

	return hr;
}

// The caller must free the memory for the string by calling CoTaskMemFree
HRESULT CompVMFDeviceList::deviceId(__in UINT32 index, __out WCHAR **ppszId)
{
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "%s(%u)", __FUNCTION__, index);
	COMPV_CHECK_HRESULT_EXP_RETURN(index >= count(), E_INVALIDARG);

	HRESULT hr;

	COMPV_CHECK_HRESULT_CODE_RETURN(hr = m_ppDevices[index]->GetAllocatedString(
		m_bVideo ? MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK : MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID,
		ppszId,
		NULL
	));

	return hr;
}

CompVMFDeviceListAudio::CompVMFDeviceListAudio() : CompVMFDeviceList(false){ }
CompVMFDeviceListAudio::~CompVMFDeviceListAudio() { }
HRESULT CompVMFDeviceListAudio::enumerateDevices()
{
	// call base class function
	return CompVMFDeviceList::enumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
}
COMPV_ERROR_CODE CompVMFDeviceListAudio::newObj(CompVMFDeviceListAudioPtrPtr list) {
	COMPV_CHECK_EXP_RETURN(!list, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMFDeviceListAudioPtr list_ = new CompVMFDeviceListAudio();
	COMPV_CHECK_EXP_RETURN(!list_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*list = list_;
	return COMPV_ERROR_CODE_S_OK;
}

CompVMFDeviceListVideo::CompVMFDeviceListVideo() : CompVMFDeviceList(true) { }
CompVMFDeviceListVideo::~CompVMFDeviceListVideo() {}
HRESULT CompVMFDeviceListVideo::enumerateDevices()
{
	// call base class function
	return CompVMFDeviceList::enumerateDevices(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
}
COMPV_ERROR_CODE CompVMFDeviceListVideo::newObj(CompVMFDeviceListVideoPtrPtr list) {
	COMPV_CHECK_EXP_RETURN(!list, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMFDeviceListVideoPtr list_ = new CompVMFDeviceListVideo();
	COMPV_CHECK_EXP_RETURN(!list_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*list = list_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()