/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_utils.h"
#include "compv/base/compv_debug.h"

#include <comutil.h>
#include <initguid.h>

#define COMPV_THIS_CLASSNAME "CompVDSUtils"

COMPV_NAMESPACE_BEGIN()

// MEDIASUBTYPE_I420 is defined in 'wmcodecdsp.h' (MediaFoundation)
DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
struct {
	COMPV_SUBTYPE subType;
	const GUID& guid;
} static const kCompVSubTypeGuidPairs[] = {
	// Most computer vision features require grayscale image as input.
	// YUV formats first because it's easier to convert to grayscal compared to RGB.
	// FIXME(dmi)
	{ COMPV_SUBTYPE_PIXELS_YUV420P, MEDIASUBTYPE_I420 }, // YUV420P, IYUV, I420: used by MPEG codecs
	{ COMPV_SUBTYPE_PIXELS_NV12, MEDIASUBTYPE_NV12 }, // Used by NVIDIA CUDA
	{ COMPV_SUBTYPE_PIXELS_UYVY, MEDIASUBTYPE_UYVY },
	{ COMPV_SUBTYPE_PIXELS_YUY2, MEDIASUBTYPE_YUY2 },
	{ COMPV_SUBTYPE_PIXELS_YV12, MEDIASUBTYPE_YV12 },

	// RGB formats as fallback
	{ COMPV_SUBTYPE_PIXELS_BGR24, MEDIASUBTYPE_RGB24 },
	{ COMPV_SUBTYPE_PIXELS_BGRA32, MEDIASUBTYPE_RGB32 },
	{ COMPV_SUBTYPE_PIXELS_ABGR32, MEDIASUBTYPE_ARGB32 },
	{ COMPV_SUBTYPE_PIXELS_RGB565LE, MEDIASUBTYPE_RGB565 },
};
static const size_t kCompVSubTypeGuidPairsCount = sizeof(kCompVSubTypeGuidPairs) / sizeof(kCompVSubTypeGuidPairs[0]);

#define CompVFindSubTypePairByGuid(_guid, _index) { \
	*_index = -1; \
	for (int _i = 0; _i < kCompVSubTypeGuidPairsCount; ++_i) { \
		if (InlineIsEqualGUID(kCompVSubTypeGuidPairs[_i].guid, _guid)) { \
			*_index = _i; break; \
		} \
	} \
}


const char* CompVDSUtils::guidName(const GUID& guid)
{
	if (InlineIsEqualGUID(guid, MEDIASUBTYPE_I420)) {
		return "MEDIASUBTYPE_I420";
	}
	return GuidNames[guid];
}
const char* CompVDSUtilsGuidName(const GUID& guid) { return CompVDSUtils::guidName(guid); } // Used in 'ds_config.h' to avoid including 'ds_utils.h'(recurssive include)

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

HRESULT CompVDSUtils::pin(IBaseFilter *pFilter, PIN_DIRECTION dir, IPin** pin)
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

    COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::pin(source, PINDIR_OUTPUT, &outPin));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::pin(destination, PINDIR_INPUT, &inPin));

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

    COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::pin(source, PINDIR_OUTPUT, &outPin));
    COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::pin(destination, PINDIR_INPUT, &inPin));

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

COMPV_ERROR_CODE CompVDSUtils::convertSubType(const COMPV_SUBTYPE& subTypeIn, GUID &subTypeOut)
{
	for (size_t i = 0; i < kCompVSubTypeGuidPairsCount; ++i) {
		if (kCompVSubTypeGuidPairs[i].subType == subTypeIn) {
			subTypeOut = kCompVSubTypeGuidPairs[i].guid;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	COMPV_DEBUG_ERROR("Directshow camera implementation doesn't support subType %d", subTypeIn);
	return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
}

COMPV_ERROR_CODE CompVDSUtils::convertSubType(const GUID &subTypeIn, COMPV_SUBTYPE &subTypeOut)
{
	for (size_t i = 0; i < kCompVSubTypeGuidPairsCount; ++i) {
		if (InlineIsEqualGUID(kCompVSubTypeGuidPairs[i].guid, subTypeIn)) {
			subTypeOut = kCompVSubTypeGuidPairs[i].subType;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	COMPV_DEBUG_ERROR("CompV doesn't support subType %s", CompVDSUtils::guidName(subTypeIn));
	return COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT;
}

// Returned 'mediaType' must be freed using 'DeleteMediaType'
COMPV_ERROR_CODE CompVDSUtils::capsToMediaType(IAMStreamConfig* streamConfig, const CompVDSCameraCaps &caps, AM_MEDIA_TYPE*& mediaType)
{
	COMPV_CHECK_EXP_RETURN(!streamConfig, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (mediaType) {
		DeleteMediaType(mediaType);
		mediaType = NULL;
	}
	HRESULT hr = S_OK;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	VIDEOINFOHEADER* vih;
	BITMAPINFOHEADER* bih;
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = streamConfig->GetFormat(&mediaType));

	vih = reinterpret_cast<VIDEOINFOHEADER*>(mediaType->pbFormat);
	COMPV_CHECK_HRESULT_EXP_BAIL(!vih, hr = E_POINTER);
	vih->AvgTimePerFrame = static_cast<REFERENCE_TIME>(COMPV_DS_SEC_TO_100NS(caps.fps));

	bih = &vih->bmiHeader;
	COMPV_CHECK_HRESULT_EXP_BAIL(!bih, hr = E_POINTER);
	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = caps.width;
	bih->biHeight = caps.height;
	bih->biSizeImage = GetBitmapSize(bih);

	mediaType->cbFormat = sizeof(VIDEOINFOHEADER);
	mediaType->subtype = caps.subType;

bail:
	if (FAILED(hr) || COMPV_ERROR_CODE_IS_NOK(err)) {
		if (mediaType) {
			DeleteMediaType(mediaType);
			mediaType = NULL;
		}
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_DIRECTSHOW);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVDSUtils::mediaTypeToCaps(const AM_MEDIA_TYPE* mediaType, CompVDSCameraCaps &caps)
{
	COMPV_CHECK_EXP_RETURN(!mediaType, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	VIDEOINFOHEADER* vih;
	BITMAPINFOHEADER* bih;

	vih = reinterpret_cast<VIDEOINFOHEADER*>(mediaType->pbFormat);
	COMPV_CHECK_EXP_BAIL(!vih, err = COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT);
	caps.fps = static_cast<int>(COMPV_DS_100NS_TO_SEC(vih->AvgTimePerFrame));

	bih = &vih->bmiHeader;
	COMPV_CHECK_EXP_BAIL(!bih, err = COMPV_ERROR_CODE_E_INVALID_IMAGE_FORMAT);
	caps.width = bih->biWidth;
	caps.height = bih->biHeight;
	
	caps.subType = mediaType->subtype;
bail:
	return err;
}

HRESULT CompVDSUtils::supportedCaps(IBaseFilter *sourceFilter, std::vector<CompVDSCameraCaps>& caps)
{
	COMPV_CHECK_HRESULT_EXP_RETURN(!sourceFilter, E_INVALIDARG);
	HRESULT hr = S_OK;
	IPin *pinOut = NULL;
	IAMStreamConfig *streamConfig = NULL;
	AM_MEDIA_TYPE *mediaType = NULL;
	int count, size;
	VIDEO_STREAM_CONFIG_CAPS streamConfigCaps;
	VIDEOINFOHEADER* vih;
	BITMAPINFOHEADER* bih;
	std::vector<CompVDSCameraCaps> caps_;

	COMPV_CHECK_HRESULT_CODE_BAIL(hr = CompVDSUtils::pin(sourceFilter, PINDIR_OUTPUT, &pinOut));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = COMPV_DS_QUERY(pinOut, IID_IAMStreamConfig, streamConfig));
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = streamConfig->GetNumberOfCapabilities(&count, &size));
	COMPV_CHECK_HRESULT_EXP_BAIL(sizeof(streamConfigCaps) < size, hr = E_INVALIDARG); // streamConfigCaps will be used by 'GetStreamCaps' which expect as least 'size' BYTES
	COMPV_CHECK_HRESULT_CODE_BAIL(hr = streamConfig->GetFormat(&mediaType));
	
	for (int i = 0; i < count; ++i) {
		COMPV_CHECK_HRESULT_CODE_BAIL(hr = streamConfig->GetStreamCaps(i, &mediaType, reinterpret_cast<BYTE*>(&streamConfigCaps)));
		if (InlineIsEqualGUID(streamConfigCaps.guid, FORMAT_VideoInfo)) {
			vih = reinterpret_cast<VIDEOINFOHEADER*>(mediaType->pbFormat);
			COMPV_CHECK_HRESULT_EXP_BAIL(!vih, hr = E_POINTER);
			bih = &vih->bmiHeader;
			COMPV_CHECK_HRESULT_EXP_BAIL(!vih, hr = E_POINTER);
			caps_.push_back(CompVDSCameraCaps(
					bih->biWidth,
					bih->biHeight,
					static_cast<int>(COMPV_DS_100NS_TO_SEC(vih->AvgTimePerFrame)),
					mediaType->subtype)
			);
		}
		DeleteMediaType(mediaType);
		mediaType = NULL;
	}

	caps = caps_;

bail:
	if (mediaType) {
		DeleteMediaType(mediaType);
	}
	COMPV_DS_SAFE_RELEASE(streamConfig);
	COMPV_DS_SAFE_RELEASE(pinOut);
	return hr;
}

COMPV_ERROR_CODE CompVDSUtils::bestCap(const std::vector<CompVDSCameraCaps>& supported, const CompVDSCameraCaps& requested, CompVDSCameraCaps& best, bool ignoreFPS COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(supported.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	static const UINT32 kSubTypeMismatchPad = _UI32_MAX >> 4;
	static const UINT32 kFpsMismatchPad = _UI32_MAX >> 2;
	UINT32 score, bestScore = _UI32_MAX;
	int index, besti = 0, i = 0;

	for (std::vector<CompVDSCameraCaps>::const_iterator it = supported.begin(); it != supported.end(); ++it, ++i) {
		if (InlineIsEqualGUID(it->subType, requested.subType)) {
			score = 0;
		}
		else {
			CompVFindSubTypePairByGuid(it->subType, &index);
			score = (index == -1)
				? kSubTypeMismatchPad // Not a must but important: If(!VideoProcess) then CLSID_CColorConvertDMO
				: (kSubTypeMismatchPad >> (kCompVSubTypeGuidPairsCount - index));
		}
		score += static_cast<UINT32>(abs(it->width - requested.width)); // Not a must: If (!VideoProcess) then CLSID_CResizerDMO
		score += static_cast<UINT32>(abs(it->height - requested.height)); // Not a must: If (!VideoProcess) then CLSID_CResizerDMO
		if (!ignoreFPS) {
			score += (it->fps == requested.fps) ? 0 : kFpsMismatchPad; // Fps is a must because without video processor no alternative exist (CLSID_CFrameRateConvertDmo doesn't support I420)
		}
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "score(%s, [%s], [%s]) = %u", ignoreFPS ? "true" : "false", it->toString().c_str(), requested.toString().c_str(), score);

		if (score <= bestScore) {
			besti = i;
			bestScore = score;
		}
	}

	best = supported[besti];
	if (ignoreFPS) {
		best.fps = requested.fps;
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "bestCap(%s, %s) = %s", ignoreFPS ? "true" : "false", requested.toString().c_str(), best.toString().c_str());

	return COMPV_ERROR_CODE_S_OK;
}

HRESULT CompVDSUtils::bstrToString(__in BSTR* bstr, __out std::string& str)
{
    COMPV_CHECK_HRESULT_EXP_RETURN(!bstr || !*bstr, E_INVALIDARG);
    char* lpszStr = _com_util::ConvertBSTRToString(*bstr);
    if (!lpszStr) {
        COMPV_CHECK_HRESULT_CODE_RETURN(E_OUTOFMEMORY);
    }
    str = std::string(lpszStr);

    delete[]lpszStr;
    return S_OK;
}


COMPV_NAMESPACE_END()
