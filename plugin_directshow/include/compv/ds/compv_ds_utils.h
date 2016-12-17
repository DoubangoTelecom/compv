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
    static COMPV_ERROR_CODE enumerateCaptureDevices(CompVDSCameraDeviceInfoList& list);

    static COMPV_ERROR_CODE createSourceFilter(__out IBaseFilter **sourceFilter, __in const std::string& deviceId = "");

    static COMPV_ERROR_CODE connectFilters(IGraphBuilder* graphBuilder, IBaseFilter* source, IBaseFilter* destination, AM_MEDIA_TYPE* mediaType = NULL);
    static COMPV_ERROR_CODE disconnectFilters(IGraphBuilder* graphBuilder, IBaseFilter* source, IBaseFilter* destination);
    static COMPV_ERROR_CODE disconnectAllFilters(IGraphBuilder* graphBuilder);
    static COMPV_ERROR_CODE removeAllFilters(IGraphBuilder* graphBuilder);

	static COMPV_ERROR_CODE convertSubType(const COMPV_SUBTYPE& subTypeIn, GUID &subTypeOut);
	static COMPV_ERROR_CODE convertSubType(const GUID &subTypeIn, COMPV_SUBTYPE &subTypeOut);

	static COMPV_ERROR_CODE capsToMediaType(IAMStreamConfig* streamConfig, const CompVDSCameraCaps &caps, AM_MEDIA_TYPE*& mediaType);
	static COMPV_ERROR_CODE mediaTypeToCaps(const AM_MEDIA_TYPE* mediaType, CompVDSCameraCaps &caps);

	static HRESULT supportedCaps(IBaseFilter *sourceFilter, std::vector<CompVDSCameraCaps>& caps);
	static COMPV_ERROR_CODE bestCap(const std::vector<CompVDSCameraCaps>& supported, const CompVDSCameraCaps& requested, CompVDSCameraCaps& best, bool ignoreFPS = false);

private:
    static HRESULT pin(IBaseFilter *pFilter, PIN_DIRECTION dir, IPin** pin);
    static HRESULT bstrToString(__in BSTR* bstr, __out std::string& str);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_UTILS_H_ */
