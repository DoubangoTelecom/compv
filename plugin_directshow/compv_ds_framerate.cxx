/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_framerate.h"
#include "compv/base/compv_debug.h"
#include <initguid.h>

// {5CC3C068-49A2-4F1F-98EA-AB2E674DF548}
DEFINE_GUID(CLSID_CompVDSFilterFramerate,
	0x5cc3c068, 0x49a2, 0x4f1f, 0x98, 0xea, 0xab, 0x2e, 0x67, 0x4d, 0xf5, 0x48);

COMPV_NAMESPACE_BEGIN()

CompVDSFilterFramerate::CompVDSFilterFramerate()
	: CTransInPlaceFilter(TEXT("CompVDSFilterFramerate"), static_cast<LPUNKNOWN>(NULL), CLSID_CompVDSFilterFramerate, NULL)
{
	
}

CompVDSFilterFramerate::~CompVDSFilterFramerate()
{

}

HRESULT CompVDSFilterFramerate::CheckInputType(const CMediaType* mtIn) /*Overrides(CTransInPlaceFilter)*/
{
	COMPV_CHECK_HRESULT_CODE_RETURN(E_NOTIMPL);
	return S_OK;
}

HRESULT CompVDSFilterFramerate::Transform(IMediaSample *pSample) /*Overrides(CTransInPlaceFilter)*/
{
	COMPV_CHECK_HRESULT_CODE_RETURN(E_NOTIMPL);
	return S_OK;
}

COMPV_NAMESPACE_END()