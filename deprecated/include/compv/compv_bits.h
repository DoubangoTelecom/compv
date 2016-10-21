/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BITS_H_)
#define _COMPV_BITS_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

COMPV_EXTERNC_BEGIN()

extern COMPV_API compv::compv_scalar_t kPopcnt256[];

COMPV_EXTERNC_END()

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVBits
{
public:
};

#if defined(_MSC_VER)
#	define compv_popcnt16(val)		__popcnt16((val))
#	define compv_popcnt32(val)		__popcnt((val))
#	define compv_popcnt64(val)		__popcnt64((val))
#else
#	define compv_popcnt16(val)		__builtin_popcount((val))
#	define compv_popcnt32(val)		__builtin_popcount((val))
#	define compv_popcnt64(val)		__builtin_popcountll((val))
#endif
#define compv_popcnt16_soft(val)		(kPopcnt256[val & 0xFF] + kPopcnt256[(val >> 8) & 0xFF])

// https://github.com/DoubangoTelecom/compv/issues/27
// #define compv_popcnt16(hard, val)		(val ? compv_popcnt16_hard((val)) : compv_popcnt16_soft((val)))

COMPV_NAMESPACE_END()

#endif /* _COMPV_BITS_H_ */
