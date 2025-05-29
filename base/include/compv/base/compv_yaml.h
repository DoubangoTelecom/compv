/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_YAML_H_)
#define _COMPV_BASE_YAML_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_mat.h"

namespace c4 {
	namespace yml {
		class Tree;
	}
}

COMPV_NAMESPACE_BEGIN()

// 'struct' instead of 'class' because cannot be an CompVObj as it'll most likely instantiated before CompVBase::init
struct COMPV_BASE_API CompVYAML
{	
protected:
	CompVYAML() = delete;
	CompVYAML(CompVYAML const& that);
	CompVYAML(const std::string& yaml_data, const std::string& default_section = "default");
public:
	virtual ~CompVYAML();

	bool isValid() const {
		return m_Tree != nullptr;
	}

	COMPV_ERROR_CODE setInt(const std::string& section, const std::string& name, const int& val) const;
	COMPV_ERROR_CODE setString(const std::string& section, const std::string& name, const std::string& val) const;
	COMPV_ERROR_CODE setFloat(const std::string& section, const std::string& name, const float& val) const;
	COMPV_ERROR_CODE setBool(const std::string& section, const std::string& name, const bool& val) const;

	int getInt(const std::string& section, const std::string& name, const int& default_val = 0) const;
	std::string getString(const std::string& section, const std::string& name, const std::string& default_val = "") const;
	float getFloat(const std::string& section, const std::string& name, const float& default_val = 0.f) const;
	bool getBool(const std::string& section, const std::string& name, const bool& default_val = false) const;

	static CompVYAML buildFromFile(const std::string& file_path, const std::string& default_section = "default") {
		return CompVYAML(CompVYAML::readData(file_path), default_section);
	}
	static CompVYAML buildFromString(const std::string& yaml_data, const std::string& default_section = "default") {
		return CompVYAML(yaml_data, default_section);
	}

	static std::string readData(const std::string& file_path);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	std::string m_DefaultSection;
	c4::yml::Tree* m_Tree;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_YAML_H_ */
