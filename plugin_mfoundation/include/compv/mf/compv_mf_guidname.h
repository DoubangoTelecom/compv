/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#ifndef _COMPV_PLUGIN_MFOUNDATION_GUIDNAME_H_
#define _COMPV_PLUGIN_MFOUNDATION_GUIDNAME_H_

#include "compv/mf/compv_mf_config.h"

//  Outputting GUID names.  If you want to include the name
//  associated with a GUID (eg CLSID_...) then
//
//      GuidNames[yourGUID]
//
//  Returns the name defined in uuids.h as a string

typedef struct {
	CHAR   *szName;
	GUID    guid;
} GUID_STRING_ENTRY;

class CGuidNameList {
public:
	CHAR *operator [] (const GUID& guid);
};

extern CGuidNameList GuidNames;


#endif /* _COMPV_PLUGIN_MFOUNDATION_GUIDNAME_H_ */
