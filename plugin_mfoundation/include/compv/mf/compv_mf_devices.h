/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_MFOUNDATION_DEVICES_H_
#define _COMPV_PLUGIN_MFOUNDATION_DEVICES_H_

#include "compv/mf/compv_mf_config.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include <mfapi.h>

COMPV_NAMESPACE_BEGIN()

class CompVMFDeviceList
{
protected:
	CompVMFDeviceList(bool bVideo);
public:
	virtual  ~CompVMFDeviceList();

	UINT32  count()const;
	void clear();
	HRESULT deviceAtIndex(__in UINT32 index, __out IMFActivate **ppActivate);
	HRESULT deviceWithId(__in const WCHAR *pszId, __out IMFActivate **ppActivate);
	HRESULT deviceName(__in UINT32 index, __out WCHAR **ppszName);
	HRESULT deviceId(__in UINT32 index, __out WCHAR **ppszId);

protected:
	HRESULT enumerateDevices(const GUID& sourceType);

private:
	UINT32 m_cDevices;
	IMFActivate **m_ppDevices;
	bool m_bVideo;
};

class CompVMFDeviceListAudio : public CompVMFDeviceList
{
public:
	CompVMFDeviceListAudio();
	virtual  ~CompVMFDeviceListAudio();
	HRESULT enumerateDevices();
};

class CompVMFDeviceListVideo : public CompVMFDeviceList
{
public:
	CompVMFDeviceListVideo();
	virtual  ~CompVMFDeviceListVideo();
	HRESULT enumerateDevices();
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_MFOUNDATION_DEVICES_H_ */
