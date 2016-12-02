/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_utils.h"
#include "compv/base/compv_debug.h"

#include <comutil.h>

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVDSUtils::enumerateCaptureDevices(CompVDSCameraDeviceInfoList& list)
{
	list.clear();
	HRESULT hr = S_OK;
	ICreateDevEnum* devEnum = NULL;
	IEnumMoniker* enumerator = NULL;
	IMoniker* moniker = NULL;
	VARIANT varId;
	VARIANT varName;
	VARIANT varDescription;
	std::string id;
	std::string name;
	std::string description;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_COCREATE(CLSID_SystemDeviceEnum, IID_ICreateDevEnum, devEnum));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumerator, CDEF_DEVMON_FILTER | CDEF_DEVMON_PNP_DEVICE));
	if (!enumerator) {
		goto bail;
	}

	while (enumerator->Next(1, &moniker, NULL) == S_OK) {
		IPropertyBag *propBag = NULL;
		if (FAILED(hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&propBag)))) {
			COMPV_DS_SAFE_RELEASE(moniker);
			continue;
		}

		id = "", name = "", description = "";

		// Init variants
		VariantInit(&varId);
		VariantInit(&varName);
		VariantInit(&varDescription);
		
		// DevicePath
		if (SUCCEEDED(propBag->Read(TEXT("DevicePath"), &varId, 0))) {
			COMPV_CHECK_HRESULT_CODE_NOP(CompVDSUtils::bstrToString(&varId.bstrVal, id));
		}
		// FriendlyName
		if (SUCCEEDED(propBag->Read(TEXT("FriendlyName"), &varName, 0))) {
			COMPV_CHECK_HRESULT_CODE_NOP(CompVDSUtils::bstrToString(&varName.bstrVal, name));
		}
		// Description
		if (SUCCEEDED(propBag->Read(L"Description", &varName, 0))) {
			COMPV_CHECK_HRESULT_CODE_NOP(CompVDSUtils::bstrToString(&varDescription.bstrVal, description));
		}
		// 
		COMPV_CHECK_HRESULT_CODE_NOP(VariantClear(&varId));
		COMPV_CHECK_HRESULT_CODE_NOP(VariantClear(&varName));
		COMPV_CHECK_HRESULT_CODE_NOP(VariantClear(&varDescription));

		COMPV_DS_SAFE_RELEASE(propBag);
		COMPV_DS_SAFE_RELEASE(moniker);
		
		if (!id.empty()) {
			list.push_back(CompVDSCameraDeviceInfo(id, name, description));
		}
	}

bail:
	COMPV_DS_SAFE_RELEASE(moniker);
	COMPV_DS_SAFE_RELEASE(enumerator);
	COMPV_DS_SAFE_RELEASE(devEnum);
	if (FAILED(hr)) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSUtils::createSourceFilter(__out IBaseFilter **sourceFilter, __in const std::string& deviceId COMPV_DEFAULT(""))
{
	COMPV_CHECK_EXP_RETURN(!sourceFilter, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*sourceFilter = NULL;
	HRESULT hr = S_OK;
	ICreateDevEnum* devEnum = NULL;
	IEnumMoniker* enumerator = NULL;
	IMoniker* moniker = NULL;
	VARIANT varId;
	std::string id;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_COCREATE(CLSID_SystemDeviceEnum, IID_ICreateDevEnum, devEnum));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumerator, CDEF_DEVMON_FILTER | CDEF_DEVMON_PNP_DEVICE));
	if (!enumerator) {
		goto bail;
	}

	while (enumerator->Next(1, &moniker, NULL) == S_OK && !*sourceFilter) {
		IPropertyBag *propBag = NULL;
		if (FAILED(hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&propBag)))) {
			COMPV_DS_SAFE_RELEASE(moniker);
			continue;
		}
		
		if (!deviceId.empty()) {
			id = "";
			VariantInit(&varId);
			if (SUCCEEDED(propBag->Read(TEXT("DevicePath"), &varId, 0))) {
				COMPV_CHECK_HRESULT_CODE_NOP(CompVDSUtils::bstrToString(&varId.bstrVal, id));
			}
			COMPV_CHECK_HRESULT_CODE_NOP(VariantClear(&varId));
			if (id == deviceId) {
				COMPV_CHECK_HRESULT_CODE_NOP(moniker->BindToObject(0, 0, IID_IBaseFilter, reinterpret_cast<void**>(sourceFilter)));
			}
		}
		else {
			COMPV_CHECK_HRESULT_CODE_NOP(moniker->BindToObject(0, 0, IID_IBaseFilter, reinterpret_cast<void**>(sourceFilter)));
		}
		
		COMPV_DS_SAFE_RELEASE(propBag);
		COMPV_DS_SAFE_RELEASE(moniker);
	}

bail:
	COMPV_DS_SAFE_RELEASE(moniker);
	COMPV_DS_SAFE_RELEASE(enumerator);
	COMPV_DS_SAFE_RELEASE(devEnum);
	if (FAILED(hr) || !*sourceFilter) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

HRESULT CompVDSUtils::getPin(IBaseFilter *pFilter, PIN_DIRECTION dir, IPin** pin)
{
	COMPV_CHECK_EXP_RETURN(!pin, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*pin = NULL;
	IEnumPins* enumPins = NULL;
	IPin* pin_ = NULL;
	HRESULT hr = S_OK;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = pFilter->EnumPins(&enumPins));
	COMPV_CHECK_HRESULT_EXP_BAIL(!enumPins, hr = E_POINTER);
	
	for (;;) {
		ULONG fetched = 0;
		PIN_DIRECTION pinDir = PIN_DIRECTION(-1);
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = enumPins->Next(1, &pin_, &fetched));
		if (fetched == 1 && pin_) {
			COMPV_CHECK_HRESULT_CODE_NOP(pin_->QueryDirection(&pinDir));
			if (pinDir == dir) {
				*pin = pin_, pin_ = NULL;
				break;
			}
		}
		COMPV_DS_SAFE_RELEASE(pin_);
	}

	COMPV_CHECK_HRESULT_EXP_BAIL(!*pin, hr = E_POINTER);

bail:
	COMPV_DS_SAFE_RELEASE(pin_);
	COMPV_DS_SAFE_RELEASE(enumPins);
	return hr;
}

COMPV_ERROR_CODE CompVDSUtils::connectFilters(IGraphBuilder* graphBuilder, IBaseFilter* source, IBaseFilter* destination, AM_MEDIA_TYPE* mediaType COMPV_DEFAULT(NULL))
{
	COMPV_CHECK_EXP_RETURN(!graphBuilder || !source || !destination, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	HRESULT hr = S_OK;
	IPin *outPin = NULL, *inPin = NULL;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::getPin(source, PINDIR_OUTPUT, &outPin));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::getPin(destination, PINDIR_INPUT, &inPin));

	if (mediaType) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = graphBuilder->ConnectDirect(outPin, inPin, mediaType));
	}
	else {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = graphBuilder->Connect(outPin, inPin));
	}

bail:
	COMPV_DS_SAFE_RELEASE(outPin);
	COMPV_DS_SAFE_RELEASE(inPin);
	if (FAILED(hr)) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSUtils::disconnectFilters(IGraphBuilder* graphBuilder, IBaseFilter* source, IBaseFilter* destination)
{
	COMPV_CHECK_EXP_RETURN(!graphBuilder || !source || !destination, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	HRESULT hr = S_OK;
	IPin *outPin = NULL, *inPin = NULL;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::getPin(source, PINDIR_OUTPUT, &outPin));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::getPin(destination, PINDIR_INPUT, &inPin));
	
	COMPV_CHECK_HRESULT_CODE_NOP(hr = graphBuilder->Disconnect(inPin));
	COMPV_CHECK_HRESULT_CODE_NOP(hr = graphBuilder->Disconnect(outPin));
	
bail:
	COMPV_DS_SAFE_RELEASE(outPin);
	COMPV_DS_SAFE_RELEASE(inPin);
	if (FAILED(hr)) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSUtils::disconnectAllFilters(IGraphBuilder* graphBuilder)
{
	COMPV_CHECK_EXP_RETURN(!graphBuilder, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	IEnumFilters* filterEnum = NULL;
	IBaseFilter* currentFilter = NULL;
	ULONG fetched;
	HRESULT hr = S_OK;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = graphBuilder->EnumFilters(&filterEnum));
	COMPV_CHECK_HRESULT_EXP_BAIL(!filterEnum, hr = E_POINTER);

	while (filterEnum->Next(1, &currentFilter, &fetched) == S_OK) {
		COMPV_CHECK_CODE_NOP(CompVDSUtils::disconnectFilters(graphBuilder, currentFilter, currentFilter));
		COMPV_DS_SAFE_RELEASE(currentFilter);
	}

bail:
	COMPV_DS_SAFE_RELEASE(filterEnum);
	COMPV_DS_SAFE_RELEASE(currentFilter);
	if (FAILED(hr)) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSUtils::removeAllFilters(IGraphBuilder* graphBuilder)
{
	COMPV_CHECK_EXP_RETURN(!graphBuilder, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	IEnumFilters* filterEnum = NULL;
	IBaseFilter* currentFilter = NULL;
	ULONG fetched;
	HRESULT hr = S_OK;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = graphBuilder->EnumFilters(&filterEnum));
	COMPV_CHECK_HRESULT_EXP_BAIL(!filterEnum, hr = E_POINTER);

	while (filterEnum->Next(1, &currentFilter, &fetched) == S_OK) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = graphBuilder->RemoveFilter(currentFilter));
		COMPV_DS_SAFE_RELEASE(currentFilter);
		COMPV_CHECK_HRESULT_CODE_NOP(filterEnum->Reset());
	}

bail:
	COMPV_DS_SAFE_RELEASE(filterEnum);
	COMPV_DS_SAFE_RELEASE(currentFilter);
	if (FAILED(hr)) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

HRESULT CompVDSUtils::bstrToString(__in BSTR* bstr, __out std::string& str)
{
	if (!bstr || !*bstr) {
		COMPV_CHECK_HRESULT_CODE_RETURN(E_INVALIDARG);
	}
	char* lpszStr = _com_util::ConvertBSTRToString(*bstr);
	if (!lpszStr) {
		COMPV_CHECK_HRESULT_CODE_RETURN(E_OUTOFMEMORY);
	}
	str = std::string(lpszStr);

	delete[]lpszStr;
	return S_OK;
}


COMPV_NAMESPACE_END()
