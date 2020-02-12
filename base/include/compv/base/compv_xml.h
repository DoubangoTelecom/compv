/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_XML_H_)
#define _COMPV_BASE_XML_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

namespace pugi {
	class xml_node;
};

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVXML
{
public:
	static COMPV_ERROR_CODE readArrayStrings(const pugi::xml_node* root, std::vector<std::string>& values);
	static COMPV_ERROR_CODE readArrayDoubles(const pugi::xml_node* root, std::vector<double>& values);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_XML_H_ */
