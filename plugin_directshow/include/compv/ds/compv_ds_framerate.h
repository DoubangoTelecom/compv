/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PLUGIN_DIRECTSHOW_FRAMERATE_H_)
#define _COMPV_PLUGIN_DIRECTSHOW_FRAMERATE_H_

#include "compv/ds/compv_ds_config.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVDSFilterFramerate : public CTransInPlaceFilter
{
public:
    CompVDSFilterFramerate();
    virtual ~CompVDSFilterFramerate();

    virtual HRESULT CheckInputType(const CMediaType* mtIn)  override /*Overrides(CTransInPlaceFilter)*/;
    virtual HRESULT Transform(IMediaSample *pSample) override /*Overrides(CTransInPlaceFilter)*/;

private:

};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_FRAMERATE_H_ */
