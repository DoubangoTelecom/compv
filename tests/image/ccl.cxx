#include "../tests_common.h"

#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/morpho"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/morpho"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define TEST_TYPE_DIFFRACT		"diffract_1285x1285_gray.yuv"
#define TEST_TYPE_DUMMY			"dummy_1285x803_gray.yuv"
#define TEST_TYPE_LABYRINTH		"labyrinth_800x600_gray.yuv"
#define TEST_TYPE_CHECKER		"checker_800x600_gray.yuv"
#define TEST_TYPE_SHAPE			"shape_960x720_gray.yuv"

static const struct compv_unittest_ccl {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_CCL[] =
{
	{ TEST_TYPE_DIFFRACT, 1285, 1285, 1285, "74b5978b6e1979eb7481983d4db0ccb0" },
	{ TEST_TYPE_DUMMY, 1285, 803, 1285, "d7a95a99385e64a58b190334562698c4" },
	{ TEST_TYPE_LABYRINTH, 800, 600, 800, "bd4bdfccb0ea70467421abf7d573b51d" },
	{ TEST_TYPE_CHECKER, 800, 600, 800, "43e40b4efe74bc924fb9239d14f1387d" },
	{ TEST_TYPE_SHAPE, 960, 720, 960, "dafbf5d4265ee4a1e1e44984e0e00e4f" },
};
static const size_t COMPV_UNITTEST_CCL_COUNT = sizeof(COMPV_UNITTEST_CCL) / sizeof(COMPV_UNITTEST_CCL[0]);

#define LOOP_COUNT		1
#define TEST_TYPE		TEST_TYPE_DIFFRACT

COMPV_ERROR_CODE ccl()
{
	const compv_unittest_ccl* test = nullptr;

	for (size_t i = 0; i < COMPV_UNITTEST_CCL_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_CCL[i].filename).compare(TEST_TYPE) == 0) {
			test = &COMPV_UNITTEST_CCL[i];
			break;
		}
	}
	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	// Read file
	CompVMatPtr binar;
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &binar));
	// CCL expect binar image
	COMPV_CHECK_CODE_RETURN(CompVImageThreshold::global(binar, &binar, 128));

	// Create the LSL ccl and set default settings
	CompVConnectedComponentLabelingPtr ccl_obj;
	COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabeling::newObj(&ccl_obj, COMPV_LSL_ID));
	COMPV_CHECK_CODE_RETURN(ccl_obj->setInt(COMPV_LSL_SET_INT_TYPE, COMPV_LSL_TYPE_STD));
	
	const uint64_t timeStart = CompVTime::nowMillis();
	CompVConnectedComponentLabelingResult result;
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(ccl_obj->process(binar, result));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO("Elapsed time (TestConnectedComponentLabeling) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");

	const size_t width = result.labels->cols();
	const size_t height = result.labels->rows();
	const size_t stride = result.labels->stride();

	CompVMatPtr imageOut;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&imageOut, height, width, stride));

	const compv_ccl_indice_t* labelsPtr = result.labels->ptr<const compv_ccl_indice_t>();
	uint8_t* imageOutPtr = imageOut->ptr<uint8_t>();

	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			imageOutPtr[i] = (labelsPtr[i] != result.label_background) ? 0xff : 0x00;
		}
		labelsPtr += stride;
		imageOutPtr += stride;
	}

	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, TEST_TYPE));
#endif

	COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(result.labels).c_str());

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(result.labels)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "CCL MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
