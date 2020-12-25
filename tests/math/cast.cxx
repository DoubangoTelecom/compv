#include "../tests_common.h"

#define TAG_TEST								"TestCast"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/test_images/"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/test_images/"
#else
#	define COMPV_TEST_IMAGE_FOLDER				"/something/"
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"

#define MD5_to_32f		"03b773b5412dee7494795b4233b27a17"

#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR

#define TYP_SRC			int16_t
#define TYP_DST			compv_float32_t

#define LOOP_COUNT			1

template<typename srcType, typename dstType>
static COMPV_ERROR_CODE __cast(CompVMatPtrPtr output);

template<typename srcType, typename dstType>
static std::string __md5();

COMPV_ERROR_CODE cast()
{
	CompVMatPtr result;
	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN((__cast<TYP_SRC, TYP_DST>(&result))); // IMPORTANT: Extra check signed dstType because the data is from uint8_t (always >= 0)
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Math Cast Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(result).c_str());

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(result).compare(__md5<TYP_SRC, TYP_DST>()) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Math Cast mismatch");

	return COMPV_ERROR_CODE_S_OK;
}

template<typename srcType, typename dstType>
static COMPV_ERROR_CODE __cast(CompVMatPtrPtr output)
{
	CompVBufferPtr buffer;
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(
		CompVFileUtils::patchFullPath((std::string(COMPV_TEST_IMAGE_FOLDER) + std::string(FILE_NAME)).c_str()).c_str(),
		&buffer
	));

	CompVMatPtr input;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&input, 720, 1282));

	COMPV_CHECK_CODE_RETURN(CompVImageUtils::copy(
		COMPV_SUBTYPE_PIXELS_Y,
		buffer->ptr(), 1282, 720, 1282,
		input->ptr<void>(), input->cols(), input->rows(), input->stride()
	));

	if (!std::is_same<srcType, uint8_t>::value) {
		COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<uint8_t, srcType>(input, &input)));
	}
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<srcType, dstType>(input, output)));
	return COMPV_ERROR_CODE_S_OK;
}

template<typename srcType, typename dstType>
static std::string __md5()
{
	if (std::is_same<dstType, compv_float32_t>::value) {
		return MD5_to_32f;
	}

	COMPV_ASSERT(false);
	return "";
}