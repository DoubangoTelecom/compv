/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#include "compv/base/compv_yaml.h"
#include "compv/base/compv_fileutils.h"

#define RYML_SINGLE_HDR_DEFINE_NOW

COMPV_VS_DISABLE_WARNINGS_BEGIN(4800)
#include <rapidyaml-0.9.0/ryml_all.hpp>
COMPV_VS_DISABLE_WARNINGS_END()

#define THIS_CLASSNAME	"CompVYAML"

COMPV_NAMESPACE_BEGIN()

#define COMPV_YAML_GET_IMPL() \
	if (!isValid()) { \
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "YAML not valid"); \
		return default_val; \
	} \
	return CompVYAMLGet(m_Tree->rootref(), section, name, m_DefaultSection, default_val);

#define COMPV_YAML_SET_IMPL() \
	if (!isValid()) { \
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "YAML not valid"); \
		return COMPV_ERROR_CODE_E_NOT_INITIALIZED; \
	} \
	return CompVYAMLSet(m_Tree->rootref(), section, name, val);

template <typename T>
static COMPV_ERROR_CODE CompVYAMLSet(ryml::NodeRef root, const std::string& section, const std::string& name, const T& val);

template <typename T>
static T CompVYAMLGet(ryml::ConstNodeRef root, const std::string& section, const std::string& entry_name, const std::string& default_section, const T& default_val);

CompVYAML::CompVYAML(const std::string& yaml_data, const std::string& default_section COMPV_DEFAULT("default"))
	: m_Tree(nullptr)
	, m_DefaultSection(default_section)
{
	if (yaml_data.empty()) {
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "Provided YAML data is empty");
	}
	else {
		try {
			m_Tree = new ryml::Tree(ryml::parse_in_arena(c4::to_csubstr(yaml_data)));
		}
		catch (const std::exception& e) {
			COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "Failed to parse YAML data: %s", e.what());
		}
	}
}

CompVYAML::~CompVYAML()
{
	if (m_Tree) {
		delete m_Tree;
		m_Tree = nullptr;
	}
}

COMPV_ERROR_CODE CompVYAML::setInt(const std::string& section, const std::string& name, const int& val) const
{
	COMPV_YAML_SET_IMPL();
}

COMPV_ERROR_CODE CompVYAML::setString(const std::string& section, const std::string& name, const std::string& val) const
{
	COMPV_YAML_SET_IMPL();
}

COMPV_ERROR_CODE CompVYAML::setFloat(const std::string& section, const std::string& name, const float& val) const
{
	COMPV_YAML_SET_IMPL();
}

COMPV_ERROR_CODE CompVYAML::setBool(const std::string& section, const std::string& name, const bool& val) const
{
	COMPV_YAML_SET_IMPL();
}

int CompVYAML::getInt(const std::string& section, const std::string& name, const int& default_val COMPV_DEFAULT(0)) const
{
	COMPV_YAML_GET_IMPL();
}

std::string CompVYAML::getString(const std::string& section, const std::string& name, const std::string& default_val COMPV_DEFAULT("")) const
{
	COMPV_YAML_GET_IMPL();
}

float CompVYAML::getFloat(const std::string& section, const std::string& name, const float& default_val COMPV_DEFAULT(.f)) const
{
	COMPV_YAML_GET_IMPL();
}

bool CompVYAML::getBool(const std::string& section, const std::string& name, const bool& default_val COMPV_DEFAULT(false)) const
{
	COMPV_YAML_GET_IMPL();
}

// Most likely called before Engine init() -> cannot use CompVFileUtils::read()
std::string CompVYAML::readData(const std::string& file_path)
{
	std::string result = "";
	const size_t cnt = CompVFileUtils::getSize(file_path.c_str());
	if (!cnt) {
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "File at %s doesn't exist or is empty", file_path.c_str());
		return result;
	}
	void* mem = malloc(cnt);
	if (!mem) {
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "Failed to allocate memory with size %zu", cnt);
		return result;
	}

	FILE* file = CompVFileUtils::open(file_path.c_str(), "rb");
	if (!file) {
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "Failed to open file at: %s", file_path.c_str());
		free(mem);
		return result;
	}

	const size_t read = fread(mem, 1, cnt, file);
	COMPV_CHECK_CODE_NOP(CompVFileUtils::close(&file));
	if (read == cnt) {
		result = std::string(reinterpret_cast<const char*>(mem), cnt);
	}
	else {
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "Size mismatch: %zu != %zu", read, cnt);
	}
	free(mem);
	return result;
}

template <typename T>
static COMPV_ERROR_CODE CompVYAMLSet(ryml::NodeRef root, const std::string& section, const std::string& name, const T& val)
{
	auto yml_section = root.find_child(section.c_str());
	if (!yml_section.valid()) {
		COMPV_DEBUG_ERROR_EX(THIS_CLASSNAME, "Cannot find YAML section %s", section.c_str());
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}
	yml_section[name.c_str()] << val;
	return COMPV_ERROR_CODE_S_OK;
}

template <typename T>
static T CompVYAMLGet(ryml::ConstNodeRef root, const std::string& section, const std::string& entry_name, const std::string& default_section, const T& default_val)
{
	// https://github.com/biojppm/rapidyaml/issues/389#issuecomment-1719945531
	auto yml_section = root.find_child(section.c_str());
	if (yml_section.valid()) {
		const auto& yml_val = yml_section.find_child(entry_name.c_str());
		if (yml_val.valid()) {
			T result;
			yml_val >> result;
			return result;
		}
		else {
			if (!default_section.empty() && section != default_section) {
				return CompVYAMLGet(root, default_section, entry_name, default_section, default_val);
			}
		}
	}
	COMPV_DEBUG_WARN_EX(THIS_CLASSNAME, "No YAML entry %s", entry_name.c_str());
	return default_val;
}

COMPV_NAMESPACE_END()

