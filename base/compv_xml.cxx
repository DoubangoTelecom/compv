/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_xml.h"
#include "compv/base/pugixml-1.9/pugixml.h"

COMPV_NAMESPACE_BEGIN()

// Expecting something like this:
// <percentages>
//		<value>0.2</value>
//		<value>0.6</value>
//		<value>1.4</value>
// </percentages>
COMPV_ERROR_CODE CompVXML::readArrayStrings(const pugi::xml_node* root, std::vector<std::string>& values)
{
	COMPV_CHECK_EXP_RETURN(!root, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	values.clear();
	const pugi::xml_node& root_ = *root;
	for (pugi::xml_node nn = root_.first_child(); nn; nn = nn.next_sibling()) {
		values.push_back(nn.child_value());
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVXML::readArrayDoubles(const pugi::xml_node* root, std::vector<double>& values)
{
	std::vector<std::string> values_;
	COMPV_CHECK_CODE_RETURN(CompVXML::readArrayStrings(root, values_));
	const size_t count = values_.size();
	values.resize(count);
	for (size_t i = 0; i < count; ++i) {
		values[i] = atof(values_[i].c_str());
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
