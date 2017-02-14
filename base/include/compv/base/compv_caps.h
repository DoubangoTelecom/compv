/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_CAPS_H_)
#define _COMPV_BASE_CAPS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVCaps
{
protected:
	CompVCaps();
public:
	virtual ~CompVCaps();

	// Next two functions should be overrided
	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
	virtual COMPV_ERROR_CODE get(int id, const void** valuePtrPtr, size_t valueSize);

	// Next functions are convenient implementation for the above two
	COMPV_ERROR_CODE setBool(int id, bool value) { return set(id, &value, sizeof(bool)); }
	COMPV_ERROR_CODE setInt(int id, int value) { return set(id, &value, sizeof(int)); }
	COMPV_ERROR_CODE setFloat32(int id, compv_float32_t value) { return set(id, &value, sizeof(compv_float32_t)); }
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_CAPS_H_ */
