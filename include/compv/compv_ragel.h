/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
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