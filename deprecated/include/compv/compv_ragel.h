/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_RAGEL_H_)
#define _COMPV_RAGEL_H_

#include "compv/compv_config.h"

COMPV_NAMESPACE_BEGIN()

#if defined(_MSC_VER)
#	define COMPV_RAGEL_DISABLE_WARNINGS_BEGIN() \
		__pragma(warning( push )) \
		__pragma(warning( disable : 4267 4244 ))
#	define COMPV_RAGEL_DISABLE_WARNINGS_END() \
		__pragma(warning( pop ))
#else
#	define COMPV_RAGEL_DISABLE_WARNINGS_BEGIN()
#	define COMPV_RAGEL_DISABLE_WARNINGS_END()
#endif /* _MSC_VER */

COMPV_NAMESPACE_END()


#endif /* _COMPV_RAGEL_H_ */