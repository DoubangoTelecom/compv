/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_VIDEO_READER_H_)
#define _COMPV_BASE_VIDEO_READER_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(VideoReader)

class COMPV_BASE_API CompVVideoReader : public CompVObj
{
protected:
	CompVVideoReader();
public:
	virtual ~CompVVideoReader();

	virtual COMPV_ERROR_CODE open(const char* path) = 0;
	virtual bool isOpen()const = 0;
	virtual COMPV_ERROR_CODE close() = 0;
	virtual COMPV_ERROR_CODE read(CompVMatPtrPtr frame) = 0;
	virtual int frameRate()const { return -1; }

	static COMPV_ERROR_CODE newObj(CompVVideoReaderPtrPtr reader);
};

//
//	CompVVideoReaderFactory
//
struct COMPV_BASE_API CompVVideoReaderFactory {
public:
	const char* name;
	COMPV_ERROR_CODE(*newObjFuncPtr)(CompVVideoReaderPtrPtr reader);
	static COMPV_ERROR_CODE set(const CompVVideoReaderFactory* inst) {
		COMPV_CHECK_EXP_RETURN(!inst, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_DEBUG_INFO("Setting video reader factory: %s", inst->name);
		instance = inst;
		return COMPV_ERROR_CODE_S_OK;
	}
	static COMPV_ERROR_CODE newObj(CompVVideoReaderPtrPtr reader) {
		COMPV_CHECK_EXP_RETURN(!instance, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
		COMPV_CHECK_CODE_RETURN(instance->newObjFuncPtr(reader));
		return COMPV_ERROR_CODE_S_OK;
	}
private:
	static const CompVVideoReaderFactory* instance;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_VIDEO_READER_H_ */
