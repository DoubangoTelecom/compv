/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PLUGIN_DIRECTSHOW_UTILS_H_)
#define _COMPV_PLUGIN_DIRECTSHOW_UTILS_H_

#include "compv/ds/compv_ds_config.h"
#include "compv/base/compv_common.h"

#include <vector>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVDSUtils
{
public:
	static const char* guidName(__in const GUID& guid);

    static COMPV_ERROR_CODE enumerateCaptureDevices(__out CompVDSCameraDeviceInfoList& list);

    static COMPV_ERROR_CODE createSourceFilter(__out IBaseFilter **sourceFilter, __in const std::string& deviceId = "");

    static COMPV_ERROR_CODE connectFilters(__in IGraphBuilder* graphBuilder, __in IBaseFilter* source, __in IBaseFilter* destination, __in AM_MEDIA_TYPE* mediaType = NULL);
    static COMPV_ERROR_CODE disconnectFilters(__in IGraphBuilder* graphBuilder, __in IBaseFilter* source, __in IBaseFilter* destination);
    static COMPV_ERROR_CODE disconnectAllFilters(__in IGraphBuilder* graphBuilder);
    static COMPV_ERROR_CODE removeAllFilters(__in IGraphBuilder* graphBuilder);

	static COMPV_ERROR_CODE convertSubType(__in const COMPV_SUBTYPE& subTypeIn, __out GUID &subTypeOut);
	static COMPV_ERROR_CODE convertSubType(__in const GUID &subTypeIn, __out COMPV_SUBTYPE &subTypeOut);

	static COMPV_ERROR_CODE capsToMediaType(IAMStreamConfig* streamConfig, const CompVDSCameraCaps &caps, AM_MEDIA_TYPE*& mediaType);
	static COMPV_ERROR_CODE mediaTypeToCaps(const AM_MEDIA_TYPE* mediaType, CompVDSCameraCaps &caps);

	static HRESULT supportedCaps(__in IBaseFilter *sourceFilter, __out std::vector<CompVDSCameraCaps>& caps);
	static COMPV_ERROR_CODE bestCap(__in const std::vector<CompVDSCameraCaps>& supported, __in const CompVDSCameraCaps& requested, __out CompVDSCameraCaps& best, __in bool ignoreFPS = false);

private:
    static HRESULT pin(__in IBaseFilter *pFilter, __in PIN_DIRECTION dir, __out IPin** pin);
    static HRESULT bstrToString(__in BSTR* bstr, __out std::string& str);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_UTILS_H_ */
