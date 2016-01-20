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

COMPV_ERROR_CODE CompVSettable::set(int id, const void* valuePtr, size_t valueSize)
{
    switch (id) {
    case -1:
    default:
        COMPV_DEBUG_ERROR("id=%d not supported", id);
        return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
    }
}

COMPV_NAMESPACE_END()
