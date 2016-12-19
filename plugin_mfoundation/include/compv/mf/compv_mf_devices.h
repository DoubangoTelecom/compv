/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_MFOUNDATION_DEVICES_H_
#define _COMPV_PLUGIN_MFOUNDATION_DEVICES_H_

#include "compv/mf/compv_mf_config.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#include <mfapi.h>

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(MFDeviceList)
COMPV_OBJECT_DECLARE_PTRS(MFDeviceListAudio)
COMPV_OBJECT_DECLARE_PTRS(MFDeviceListVideo)

class CompVMFDeviceList : public CompVObj
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
protected:
	CompVMFDeviceListAudio();
public:
	virtual  ~CompVMFDeviceListAudio();
	COMPV_OBJECT_GET_ID(CompVMFDeviceListAudio);
	HRESULT enumerateDevices();
	static COMPV_ERROR_CODE newObj(CompVMFDeviceListAudioPtrPtr list);
};

class CompVMFDeviceListVideo : public CompVMFDeviceList
{
protected:
	CompVMFDeviceListVideo();
public:
	virtual  ~CompVMFDeviceListVideo();
	COMPV_OBJECT_GET_ID(CompVMFDeviceListAudio);
	HRESULT enumerateDevices();
	static COMPV_ERROR_CODE newObj(CompVMFDeviceListVideoPtrPtr list);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_MFOUNDATION_DEVICES_H_ */
