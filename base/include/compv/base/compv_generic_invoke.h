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

#define CompVGenericInvokeVoidRawType(subType, func, ...) \
	switch (subType) { \
		case COMPV_SUBTYPE_RAW_INT8: func<int8_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_UINT8: func<uint8_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_INT16: func<int16_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_UINT16: func<uint16_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_INT32: func<int32_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_UINT32: func<uint32_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_FLOAT32: func<compv_float32_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_FLOAT64: func<compv_float64_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_USCALAR: func<compv_uscalar_t>(__VA_ARGS__); break; \
		case COMPV_SUBTYPE_RAW_SCALAR: func<compv_scalar_t>(__VA_ARGS__); break; \
		default: \
			COMPV_DEBUG_ERROR_EX("CompVGenericInvokeStaticVoidRawType", "Invalid generic type: %s", CompVGetSubtypeString(subType)); \
			return COMPV_ERROR_CODE_E_INVALID_SUBTYPE; \
	}

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_GENERIC_INVOKE_H_ */
