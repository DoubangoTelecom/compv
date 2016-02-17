/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
* Copyright (C) 2016 Mamadou DIOP
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
#if !defined(_COMPV_INTERESTPOINT_H_)
#define _COMPV_INTERESTPOINT_H_

#include "compv/compv_config.h"
#include "compv/compv_box.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVBoxInterestPoint : public CompVBox<CompVInterestPoint > {
protected:
	CompVBoxInterestPoint(size_t nCapacity = 0, bool bLockable = false);
public:
	virtual ~CompVBoxInterestPoint();
	COMPV_ERROR_CODE sort(bool(*CompVBoxPredicateCompare)(const CompVInterestPoint*, const CompVInterestPoint*)); // multithreaded sorting
	static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVBoxInterestPoint* >* box, size_t nCapacity = 0, bool bLockable = false);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_INTERESTPOINT_H_ */
