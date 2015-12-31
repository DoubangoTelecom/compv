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
#if !defined(_COMPV_BUFFER_H_)
#define _COMPV_BUFFER_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVBuffer : public CompVObj
{
protected:
	CompVBuffer(const void* pcPtr = NULL, size_t size = 0);
public:
	virtual ~CompVBuffer();
	virtual COMPV_INLINE const char* getObjectId() { return "CompVBuffer"; };

	COMPV_ERROR_CODE copyData(const void* pcPtr, size_t size);
	COMPV_ERROR_CODE takeData(void** ppPtr, size_t size);
	COMPV_INLINE const void* getPtr() { return m_pPtr; }
	COMPV_INLINE size_t getSize(){ return m_nSize; }
	COMPV_INLINE bool isEmpty() { return !(getSize() && getPtr()); }
	static COMPV_ERROR_CODE newObj(const void* pcPtr, size_t size, CompVObjWrapper<CompVBuffer*>* buffer);
	static COMPV_ERROR_CODE newObjAndNullData(CompVObjWrapper<CompVBuffer*>* buffer);
	static COMPV_ERROR_CODE newObjAndTakeData(void** ppPtr, size_t size, CompVObjWrapper<CompVBuffer*>* buffer);
	static COMPV_ERROR_CODE newObjAndCopyData(const void* pcPtr, size_t size, CompVObjWrapper<CompVBuffer*>* buffer);

private:
	void* m_pPtr;
	size_t m_nSize;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BUFFER_H_ */
