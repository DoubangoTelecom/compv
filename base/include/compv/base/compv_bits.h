/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BITS_H_)
#define _COMPV_BASE_BITS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_EXTERNC_BEGIN()

extern COMPV_BASE_API compv::compv_uscalar_t kPopcnt256[];

COMPV_EXTERNC_END()

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVBits
{
public:
	static COMPV_ERROR_CODE logical_and(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE logical_not_and(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE logical_not(const CompVMatPtr& A, CompVMatPtrPtr R);
	static COMPV_ERROR_CODE logical_xorhz(const CompVMatPtr& A, CompVMatPtrPtr R);
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

// "bsf" is the same as "ctz" on x86
#if defined(_MSC_VER)
#	define compv_bsf(val, ret)			_BitScanForward(ret, (val))
#	define compv_bsf64(val, ret)		_BitScanForward64(ret, (val))
typedef DWORD compv_bsf_t;
typedef DWORD64 compv_bsf64_t;
#else
#	if COMPV_ARCH_ARM
#		define compv_bsf(val, ret)		*(ret) = __builtin_ctz((val))
#		define compv_bsf64(val, ret)	*(ret) = __builtin_ctzll((val))
#	else
#		define compv_bsf(val, ret)		*(ret) = (__builtin_ffs((val))-1) /* do not use '__builtin_ctz' which could lead to 'tzcnt' (requires 'BMI1' CPU flags, faster) instead of 'bsf' */
#		define compv_bsf64(val, ret)	*(ret) = (__builtin_ffsll((val))-1)
#	endif /* COMPV_ARCH_ARM */
typedef int compv_bsf_t;
typedef uint64_t compv_bsf64_t;
#endif /* !defined(_MSC_VER) */

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BITS_H_ */
