/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_INTRIN_NEON_H_)
#define _COMPV_BASE_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0491h/BABJFCGC.html
#define COMPV_ARM_NEON_NEQ_ZERO(vec)	(vgetq_lane_u64(vec, 0) || vgetq_lane_u64(vec, 1))
#define COMPV_ARM_NEON_EQ_ZERO(vec)		!COMPV_ARM_NEON_NEQ_ZERO(vec)

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_INTRIN_NEON_H_ */
