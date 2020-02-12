/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_json.h"
#include "compv/base/jsoncpp-1.8.4/json.h"
#include "compv/base/compv_fileutils.h"

#define COMPV_THIS_CLASS_NAME "CompVJSON"

COMPV_NAMESPACE_BEGIN()

template<typename T>
static COMPV_ERROR_CODE writeMat(Json::Value& root, const CompVMatPtr& mat);

static COMPV_ERROR_CODE readMat(const Json::Value& root, CompVMatPtrPtr mat);

static const struct CompVJSONSubtypeMap {
	COMPV_SUBTYPE subtype;
	const char* name;
}
COMPVJSON_SUBTYPE_MAP[] =
{
	{ COMPV_SUBTYPE_RAW_INT8, "8s" },
	{ COMPV_SUBTYPE_RAW_UINT8, "8u" },
	{ COMPV_SUBTYPE_RAW_INT16, "16s" },
	{ COMPV_SUBTYPE_RAW_UINT16, "16u" },
	{ COMPV_SUBTYPE_RAW_INT32, "32s" },
	{ COMPV_SUBTYPE_RAW_UINT32, "32u" },
	{ COMPV_SUBTYPE_RAW_SIZE, "sz" },
	{ COMPV_SUBTYPE_RAW_FLOAT32, "32f" },
	{ COMPV_SUBTYPE_RAW_FLOAT64, "64f" },
	{ COMPV_SUBTYPE_RAW_USCALAR, "ureg" },
	{ COMPV_SUBTYPE_RAW_SCALAR, "sreg" },
};
static const size_t COMPVJSON_SUBTYPE_MAP_COUNT = sizeof(COMPVJSON_SUBTYPE_MAP) / sizeof(COMPVJSON_SUBTYPE_MAP[0]);

COMPV_SUBTYPE CompVJSON::subtype(const char* name)
{
	if (name) {
		const std::string name_ = name;
		for (size_t i = 0; i < COMPVJSON_SUBTYPE_MAP_COUNT; ++i) {
			if (!name_.compare(COMPVJSON_SUBTYPE_MAP[i].name)) {
				return COMPVJSON_SUBTYPE_MAP[i].subtype;
			}
		}
	}
	return COMPV_SUBTYPE_NONE;
}

const char* CompVJSON::subtype(COMPV_SUBTYPE st)
{
	for (size_t i = 0; i < COMPVJSON_SUBTYPE_MAP_COUNT; ++i) {
		if (st == COMPVJSON_SUBTYPE_MAP[i].subtype) {
			return COMPVJSON_SUBTYPE_MAP[i].name;
		}
	}
	return nullptr;
}

COMPV_ERROR_CODE CompVJSON::write(Json::Value* root, const char* name, const CompVMatPtr& mat)
{
	COMPV_CHECK_EXP_RETURN(!root || !name || !mat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	Json::Value vectors;
	switch (mat->subType()) {
	case COMPV_SUBTYPE_RAW_INT8: COMPV_CHECK_CODE_RETURN(writeMat<int8_t>(vectors, mat)); break;
	case COMPV_SUBTYPE_RAW_UINT8: COMPV_CHECK_CODE_RETURN(writeMat<uint8_t>(vectors, mat)); break;
	case COMPV_SUBTYPE_RAW_INT16: COMPV_CHECK_CODE_RETURN(writeMat<int16_t>(vectors, mat)); break;
	case COMPV_SUBTYPE_RAW_UINT16: COMPV_CHECK_CODE_RETURN(writeMat<uint16_t>(vectors, mat)); break;
	case COMPV_SUBTYPE_RAW_INT32: COMPV_CHECK_CODE_RETURN(writeMat<int32_t>(vectors, mat)); break;
	case COMPV_SUBTYPE_RAW_UINT32: COMPV_CHECK_CODE_RETURN(writeMat<uint32_t>(vectors, mat)); break;
#if 0
	case COMPV_SUBTYPE_RAW_SIZE: COMPV_CHECK_CODE_RETURN(writeMat<size_t>(vectors, mat)); break;
    case COMPV_SUBTYPE_RAW_USCALAR: COMPV_CHECK_CODE_RETURN(writeMat<compv_uscalar_t>(vectors, mat)); break;
    case COMPV_SUBTYPE_RAW_SCALAR: COMPV_CHECK_CODE_RETURN(writeMat<compv_scalar_t>(vectors, mat)); break;
#endif
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN(writeMat<compv_float32_t>(vectors, mat)); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN(writeMat<compv_float64_t>(vectors, mat)); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_SUBTYPE); break;
	}
	(*root)[name] = vectors;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVJSON::read(const Json::Value* root, const char* name, CompVMatPtrPtr mat)
{
	COMPV_CHECK_EXP_RETURN(!root || !name || !mat, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const Json::Value& root_ = *root;
	COMPV_CHECK_EXP_RETURN(!root_[name].isObject(), COMPV_ERROR_CODE_E_JSON_CPP, "'mean' field is missing or invalid");
	COMPV_CHECK_CODE_RETURN(readMat(root_[name], mat));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVJSON::parse(const char* filePath, Json::Value* root, bool collectComments COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!filePath || !root, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBufferPtr content;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(filePath, &content));
	std::istringstream contentStream(std::string(reinterpret_cast<const char*>(content->ptr()), content->size()));
	COMPV_CHECK_CODE_RETURN(CompVJSON::parse(
		contentStream,
		root,
		collectComments
	));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVJSON::parse(std::istream& jsonStream, Json::Value* root, bool collectComments COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!jsonStream || !root, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	Json::CharReaderBuilder builder;
	builder["collectComments"] = collectComments;
	JSONCPP_STRING errs;
	const bool ok = Json::parseFromStream(builder, jsonStream, root, &errs);
	if (!ok) {
		std::ostringstream os;
		os << jsonStream.rdbuf();
		std::string jsonString = os.str();
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASS_NAME, "Failed to parse JSON string. Error: %s. JSON: %s", errs.c_str(), jsonString.c_str());
		return COMPV_ERROR_CODE_E_JSON_CPP;
	}
	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static COMPV_ERROR_CODE writeMat(Json::Value& root, const CompVMatPtr& mat)
{
	COMPV_CHECK_EXP_RETURN(!mat->isRawTypeMatch<T>(), COMPV_ERROR_CODE_E_INVALID_SUBTYPE);
	const char* subtypeName = CompVJSON::subtype(mat->subType());
	COMPV_CHECK_EXP_RETURN(!subtypeName, COMPV_ERROR_CODE_E_INVALID_SUBTYPE);
	Json::Value vectors_size = Json::Value(Json::arrayValue);
	Json::Value vectors_data = Json::Value(Json::arrayValue);
	const size_t vectors_rows = mat->rows();
	const size_t vectors_cols = mat->cols();
	const size_t vectors_stride = mat->stride();
	vectors_size[0] = static_cast<Json::UInt>(vectors_rows), vectors_size[1] = static_cast<Json::UInt>(vectors_cols);
	const T* vectorsPtr = mat->ptr<const T>();
	for (size_t j = 0; j < vectors_rows; ++j) {
		for (size_t i = 0; i < vectors_cols; ++i) {
			vectors_data.append(vectorsPtr[i]);
		}
		vectorsPtr += vectors_stride;
	}
	root["type"] = subtypeName;
	root["size"] = vectors_size;
	root["data"] = vectors_data;

	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
class JSONMatVal {
public:
	virtual T operator()(const Json::Value& vv) const = 0;
};
#define JSONMatVal_decl(className, typeName, funcName) \
	class className : public JSONMatVal<typeName> { \
	public: \
		virtual typeName operator()(const Json::Value& vv) const override { return static_cast<typeName>(vv.funcName()); } \
	};
JSONMatVal_decl(JSONMatValInt8, int8_t, asInt)
JSONMatVal_decl(JSONMatValUInt8, uint8_t, asUInt)
JSONMatVal_decl(JSONMatValInt16, int16_t, asInt)
JSONMatVal_decl(JSONMatValUInt16, uint16_t, asUInt)
JSONMatVal_decl(JSONMatValInt32, int32_t, asInt)
JSONMatVal_decl(JSONMatValUInt32, uint32_t, asUInt)
#if 0
JSONMatVal_decl(JSONMatValSz, size_t, asUInt)
JSONMatVal_decl(JSONMatValUscal, compv_uscalar_t, asLargestUInt)
JSONMatVal_decl(JSONMatValScal, compv_scalar_t, asLargestInt)
#endif
JSONMatVal_decl(JSONMatValFloat32, compv_float32_t, asFloat)
JSONMatVal_decl(JSONMatValFloat64, compv_float64_t, asDouble)

template<typename T>
static COMPV_ERROR_CODE fillMat(const Json::Value& data, const size_t& rows, const size_t& cols, const JSONMatVal<T>& valEx, CompVMatPtrPtr mat)
{
	CompVMatPtr mat_;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&mat_, rows, cols));
	const size_t stride = mat_->stride();
	T* matPtr = mat_->ptr<T>();
	Json::Value::const_iterator begin = data.begin();
	for (size_t j = 0; j < rows; ++j) {
		for (size_t i = 0; i < cols; ++i, ++begin) {
			matPtr[i] = valEx(*begin);
		}
		matPtr += stride;
	}
	*mat = mat_;
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE readMat(const Json::Value& root, CompVMatPtrPtr mat)
{
	COMPV_CHECK_EXP_RETURN(!root["type"].isString(), COMPV_ERROR_CODE_E_JSON_CPP, "'type' field is missing or invalid");
	COMPV_CHECK_EXP_RETURN(!root["size"].isArray(), COMPV_ERROR_CODE_E_JSON_CPP, "'size' field is missing or invalid");
	COMPV_CHECK_EXP_RETURN(!root["data"].isArray(), COMPV_ERROR_CODE_E_JSON_CPP, "'data' field is missing or invalid");

	const COMPV_SUBTYPE subtype = CompVJSON::subtype(root["type"].asCString());
	COMPV_CHECK_EXP_RETURN(subtype == COMPV_SUBTYPE_NONE, COMPV_ERROR_CODE_E_INVALID_SUBTYPE);

	const Json::Value& size = root["size"];
	COMPV_CHECK_EXP_RETURN(size.size() != 2, COMPV_ERROR_CODE_E_JSON_CPP, "Invalid size");
	const int rows = size[0].asInt();
	const int cols = size[1].asInt();
	COMPV_CHECK_EXP_RETURN(rows <= 0 || cols <= 0, COMPV_ERROR_CODE_E_JSON_CPP, "Invalid size");

	const Json::Value& data = root["data"];
	COMPV_CHECK_EXP_RETURN(data.size() != (rows * cols), COMPV_ERROR_CODE_E_JSON_CPP, "Size mismatch");
	
	switch (subtype) {
	case COMPV_SUBTYPE_RAW_INT8: COMPV_CHECK_CODE_RETURN(fillMat<int8_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValInt8(), mat)); break;
	case COMPV_SUBTYPE_RAW_UINT8: COMPV_CHECK_CODE_RETURN(fillMat<uint8_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValUInt8(), mat)); break;
	case COMPV_SUBTYPE_RAW_INT16: COMPV_CHECK_CODE_RETURN(fillMat<int16_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValInt16(), mat)); break;
	case COMPV_SUBTYPE_RAW_UINT16: COMPV_CHECK_CODE_RETURN(fillMat<uint16_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValUInt16(), mat)); break;
	case COMPV_SUBTYPE_RAW_INT32: COMPV_CHECK_CODE_RETURN(fillMat<int32_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValInt32(), mat)); break;
	case COMPV_SUBTYPE_RAW_UINT32: COMPV_CHECK_CODE_RETURN(fillMat<uint32_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValUInt32(), mat)); break;
#if 0
	case COMPV_SUBTYPE_RAW_SIZE: COMPV_CHECK_CODE_RETURN(fillMat<size_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValSz(), mat)); break;
    case COMPV_SUBTYPE_RAW_USCALAR: COMPV_CHECK_CODE_RETURN(fillMat<compv_uscalar_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValUscal(), mat)); break;
    case COMPV_SUBTYPE_RAW_SCALAR: COMPV_CHECK_CODE_RETURN(fillMat<compv_scalar_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValScal(), mat)); break;
#endif
	case COMPV_SUBTYPE_RAW_FLOAT32: COMPV_CHECK_CODE_RETURN(fillMat<compv_float32_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValFloat32(), mat)); break;
	case COMPV_SUBTYPE_RAW_FLOAT64: COMPV_CHECK_CODE_RETURN(fillMat<compv_float64_t>(data, static_cast<size_t>(rows), static_cast<size_t>(cols), JSONMatValFloat64(), mat)); break;
	default: COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_SUBTYPE); break;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
