/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_JSON_H_)
#define _COMPV_BASE_JSON_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

namespace Json {
	class Value;
};

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVJSON
{
public:
	static COMPV_SUBTYPE subtype(const char* name);
	static const char* subtype(COMPV_SUBTYPE st);
	static COMPV_ERROR_CODE write(Json::Value* root, const char* name, const CompVMatPtr& mat);
	static COMPV_ERROR_CODE read(const Json::Value* root, const char* name, CompVMatPtrPtr mat);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_JSON_H_ */
