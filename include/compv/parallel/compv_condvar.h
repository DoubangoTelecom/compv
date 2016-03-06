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
#if !defined(_COMPV_PRALLEL_CONDVAR_H_)
#define _COMPV_PRALLEL_CONDVAR_H_

#include "compv/compv_config.h"
#include "compv/parallel/compv_mutex.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVCondvar : public CompVObj
{
protected:
	CompVCondvar();
public:
	virtual ~CompVCondvar();
	virtual COMPV_INLINE const char* getObjectId() { return "CompVCondvar"; };

	COMPV_ERROR_CODE wait(uint64_t millis = 0);
	COMPV_ERROR_CODE signal();
	COMPV_ERROR_CODE broadcast();

	static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVCondvar*>* condvar);

private:
	COMPV_ERROR_CODE waitWithoutTimeout();
	COMPV_ERROR_CODE waitWithTimeout(uint64_t millis);

private:
	void* m_pHandle;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PRALLEL_CONDVAR_H_ */

