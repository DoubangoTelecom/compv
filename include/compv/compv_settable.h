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
#if !defined(_COMPV_SETTABLE_H_)
#define _COMPV_SETTABLE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"

#include <map>

COMPV_NAMESPACE_BEGIN()

typedef COMPV_ERROR_CODE (*CompVSettableFunInt32)(int32_t value);
typedef COMPV_ERROR_CODE(*CompVSettableFunObj)(CompVObj* value);

enum {
	COMPV_SETTABLE_DATA_TYPE_INT32,
	COMPV_SETTABLE_DATA_TYPE_OBJ,
};

typedef struct compv_settable_fun_s {
	int id;
	int type;
	void* fun;
public:
	compv_settable_fun_s() : id(-1), type(-1), fun(NULL) { }
	compv_settable_fun_s(int id_, int type_, void* fun_) {
		id = id_;
		type = type_;
		fun = fun_;
	}
}
compv_settable_func_t;

class COMPV_API CompVSettable
{
protected:
	CompVSettable();
	COMPV_ERROR_CODE addFunction(int id, CompVSettableFunInt32 func);
	COMPV_ERROR_CODE addFunction(int id, CompVSettableFunObj func);

public:
	virtual ~CompVSettable();
	COMPV_ERROR_CODE setInt32(int id, int32_t value);
	COMPV_ERROR_CODE setObj(int id, CompVObj* value);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	std::map<int, compv_settable_func_t> m_Functions;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_SETTABLE_H_ */
