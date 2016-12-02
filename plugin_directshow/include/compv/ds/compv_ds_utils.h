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

private:
	static HRESULT getPin(IBaseFilter *pFilter, PIN_DIRECTION dir, IPin** pin);
	static HRESULT bstrToString(__in BSTR* bstr, __out std::string& str);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_UTILS_H_ */
