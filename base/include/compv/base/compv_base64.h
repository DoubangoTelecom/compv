/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BASE64_H_)
#define _COMPV_BASE_BASE64_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_buffer.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVBase64
{
public:
	static COMPV_ERROR_CODE encode(const uint8_t* bufPtr, const size_t buffSize, std::string& base64);
	static COMPV_ERROR_CODE decode(const std::string& base64, CompVBufferPtrPtr data);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BASE64_H_ */
