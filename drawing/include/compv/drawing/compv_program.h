/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_PROGRAM_H_)
#define _COMPV_DRAWING_PROGRAM_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class CompVProgram;
typedef CompVPtr<CompVProgram* > CompVProgramPtr;
typedef CompVProgramPtr* CompVProgramPtrPtr;

class COMPV_DRAWING_API CompVProgram : public CompVObj
{
protected:
	CompVProgram();
public:
	virtual ~CompVProgram();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVProgram";
	};
	
	virtual COMPV_ERROR_CODE shadAttachVertexFile(const char* pcFilePath) = 0;
	virtual COMPV_ERROR_CODE shadAttachFragmentFile(const char* pcFilePath) = 0;
	virtual COMPV_ERROR_CODE shadAttachVertexData(const char* dataPtr, size_t dataLength) = 0;
	virtual COMPV_ERROR_CODE shadAttachFragmentData(const char* dataPtr, size_t dataLength) = 0;
	virtual COMPV_ERROR_CODE link() = 0;
	virtual COMPV_ERROR_CODE useBegin() = 0;
	virtual COMPV_ERROR_CODE useEnd() = 0;

	static COMPV_ERROR_CODE newObj(CompVProgramPtrPtr program);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_PROGRAM_H_ */
