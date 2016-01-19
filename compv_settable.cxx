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
#include "compv/compv_settable.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVSettable::CompVSettable()
{

}

CompVSettable::~CompVSettable()
{

}

COMPV_ERROR_CODE CompVSettable::addFunction(int id, CompVSettableFunInt32 func)
{
	COMPV_CHECK_EXP_RETURN(!func, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_Functions[id] = compv_settable_func_t(id, COMPV_SETTABLE_DATA_TYPE_INT32, func);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSettable::addFunction(int id, CompVSettableFunObj func)
{
	COMPV_CHECK_EXP_RETURN(!func, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	m_Functions[id] = compv_settable_func_t(id, COMPV_SETTABLE_DATA_TYPE_OBJ, func);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVSettable::setInt32(int id, int32_t value)
{
	std::map<int, compv_settable_func_t>::iterator it = m_Functions.find(id);
	if (it == m_Functions.end()) {
		COMPV_DEBUG_ERROR("This object doesn't support this method");
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	if (it->second.id != COMPV_SETTABLE_DATA_TYPE_INT32) {
		COMPV_DEBUG_ERROR("Invalid data type: %d <> %d", it->second.id, COMPV_SETTABLE_DATA_TYPE_INT32);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	return ((CompVSettableFunInt32)it->second.fun)(value);
}

COMPV_ERROR_CODE CompVSettable::setObj(int id, CompVObj* value)
{
	std::map<int, compv_settable_func_t>::iterator it = m_Functions.find(id);
	if (it == m_Functions.end()) {
		COMPV_DEBUG_ERROR("This object doesn't support this method");
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	if (it->second.id != COMPV_SETTABLE_DATA_TYPE_OBJ) {
		COMPV_DEBUG_ERROR("Invalid data type: %d <> %d", it->second.id, COMPV_SETTABLE_DATA_TYPE_OBJ);
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	return ((CompVSettableFunObj)it->second.fun)(value);
}

COMPV_NAMESPACE_END()
