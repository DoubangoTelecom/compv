/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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

#define CompVGenericCheckVoid(code)	((code)) 
#define CompVGenericCheckCode(code) COMPV_CHECK_CODE_RETURN((code))

#define CompVGenericInvokeRawType(subType, func, check, ...) \
	COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wunused-value") \
	switch (subType) { \
		case COMPV_SUBTYPE_RAW_INT8: CompVGenericCheck##check(func<int8_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_UINT8: case COMPV_SUBTYPE_PIXELS_Y: CompVGenericCheck##check(func<uint8_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_INT16: CompVGenericCheck##check(func<int16_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_UINT16: CompVGenericCheck##check(func<uint16_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_INT32: CompVGenericCheck##check(func<int32_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_UINT32: CompVGenericCheck##check(func<uint32_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_FLOAT32: CompVGenericCheck##check(func<compv_float32_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_FLOAT64: CompVGenericCheck##check(func<compv_float64_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_USCALAR: CompVGenericCheck##check(func<compv_uscalar_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_SCALAR: CompVGenericCheck##check(func<compv_scalar_t>(__VA_ARGS__)); break; \
		default: \
			COMPV_DEBUG_ERROR_EX("CompVGenericInvokeStaticVoidRawType", "Invalid generic type: %s", CompVGetSubtypeString(subType)); \
			CompVGenericCheck##check(COMPV_ERROR_CODE_E_INVALID_SUBTYPE); \
			COMPV_ASSERT(false); /* Called for void functions only */ \
			break; \
	} \
	COMPV_GCC_DISABLE_WARNINGS_END()

#define CompVGenericFloatInvokeRawType(subType, func, check, ...) \
	switch (subType) { \
		case COMPV_SUBTYPE_RAW_FLOAT32: CompVGenericCheck##check(func<compv_float32_t>(__VA_ARGS__)); break; \
		case COMPV_SUBTYPE_RAW_FLOAT64: CompVGenericCheck##check(func<compv_float64_t>(__VA_ARGS__)); break; \
		default: \
			COMPV_DEBUG_ERROR_EX("CompVGenericInvokeStaticVoidRawType", "Invalid generic type: %s", CompVGetSubtypeString(subType)); \
			CompVGenericCheck##check(COMPV_ERROR_CODE_E_INVALID_SUBTYPE); \
			COMPV_ASSERT(false); /* Called for void functions only */ \
			break; \
	}

#define CompVGenericInvokeVoidRawType(subType, func, ...) CompVGenericInvokeRawType(subType, func, Void, __VA_ARGS__)
#define CompVGenericInvokeCodeRawType(subType, func, ...) CompVGenericInvokeRawType(subType, func, Code, __VA_ARGS__)
#define CompVGenericFloatInvokeVoidRawType(subType, func, ...) CompVGenericFloatInvokeRawType(subType, func, Void, __VA_ARGS__)
#define CompVGenericFloatInvokeCodeRawType(subType, func, ...) CompVGenericFloatInvokeRawType(subType, func, Code, __VA_ARGS__)

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_GENERIC_INVOKE_H_ */
