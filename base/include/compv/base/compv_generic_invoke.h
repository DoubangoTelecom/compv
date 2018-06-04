/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_GENERIC_INVOKE_H_)
#define _COMPV_BASE_GENERIC_INVOKE_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

#define CompVGenericInvokeStaticVoidRawType(subType, staticFunc, ...) \
	switch (subType) { \
		case COMPV_SUBTYPE_RAW_INT8: staticFunc<int8_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_UINT8: staticFunc<uint8_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_INT16: staticFunc<int16_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_UINT16: staticFunc<uint16_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_INT32: staticFunc<int32_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_UINT32: staticFunc<uint32_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_FLOAT32: staticFunc<compv_float32_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_FLOAT64: staticFunc<compv_float64_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_USCALAR: staticFunc<compv_uscalar_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_SCALAR: staticFunc<compv_scalar_t>(__VA_ARGS__); break; \
		default: \
			COMPV_DEBUG_ERROR_EX("CompVGenericInvokeStaticVoidRawType", "Invalid generic type: %s", CompVGetSubtypeString(subType)); \
			return COMPV_ERROR_CODE_E_INVALID_SUBTYPE; \
	}

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_GENERIC_INVOKE_H_ */
