#include "../tests/tests_common.h"

#define TAG_TEST							"UnitTestConnectedComponentLabeling"
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
#define TEST_TYPE_TEXT			"text_1122x1182_white_gray.yuv" // text is white and background black

static const struct compv_unittest_ccl {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5_labels;
	const char* md5_segments;
}
COMPV_UNITTEST_CCL[] =
{
	{
		TEST_TYPE_DIFFRACT, 1285, 1285, 1285,
		"74b5978b6e1979eb7481983d4db0ccb0", // md5_labels
		"073475af4a73d28163a9cbf648f63368" // md5_segments
	},
	{
		TEST_TYPE_DUMMY, 1285, 803, 1285,
		"d7a95a99385e64a58b190334562698c4", // md5_labels
		"b7a1c3d0d1b20feb987ece24d97be610" // md5_segments
	},
	{
		TEST_TYPE_LABYRINTH, 800, 600, 800,
		"bd4bdfccb0ea70467421abf7d573b51d", // md5_labels
		"dad6291e26bb45ddb55c28b12ba575ed" // md5_segments
	},
	{
		TEST_TYPE_CHECKER, 800, 600, 800,
		"43e40b4efe74bc924fb9239d14f1387d", // md5_labels
		"eeec4ea068dd274d34932d41f211babd" // md5_segments
	},
	{
		TEST_TYPE_SHAPE, 960, 720, 960,
		"dafbf5d4265ee4a1e1e44984e0e00e4f", // md5_labels
		"bf2627a2bd2e472b35cce7bb1daf5357" // md5_segments
	},
	{
		TEST_TYPE_TEXT, 1122, 1182, 1122,
		"5d416e9164481180f279f807e42ef5b0", // md5_labels
		"266b623426994c24b668d8b02141729a" // md5_segments
	}
};
static const size_t COMPV_UNITTEST_CCL_COUNT = sizeof(COMPV_UNITTEST_CCL) / sizeof(COMPV_UNITTEST_CCL[0]);

static const std::string compv_unittest_ccl_to_string(const compv_unittest_ccl* test) {
	return std::string("filename:") + std::string(test->filename);
}

static COMPV_ERROR_CODE check_labels(const CompVConnectedComponentLabelingResultPtr& result, const compv_unittest_ccl* test);
static COMPV_ERROR_CODE check_segments(const CompVConnectedComponentLabelingResultPtr& result, const compv_unittest_ccl* test);
static COMPV_ERROR_CODE check_blobs(const CompVConnectedComponentLabelingResultPtr& result, const CompVMatPtr& ptr8uInput);

COMPV_ERROR_CODE unittest_ccl()
{
	const compv_unittest_ccl* test;
	CompVMatPtr binar;
	CompVConnectedComponentLabelingResultPtr result;

	// Create the LSL ccl and set default settings
	CompVConnectedComponentLabelingPtr ccl_obj;
	COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabeling::newObj(&ccl_obj, COMPV_PLSL_ID));
	COMPV_CHECK_CODE_RETURN(ccl_obj->setInt(COMPV_PLSL_SET_INT_TYPE, COMPV_PLSL_TYPE_XRLEZ));

	for (size_t i = 0; i < COMPV_UNITTEST_CCL_COUNT; ++i) {
		test = &COMPV_UNITTEST_CCL[i];
		COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: CCL -> %s ==", compv_unittest_ccl_to_string(test).c_str());		
		COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &binar));
		COMPV_CHECK_CODE_RETURN(CompVImage::thresholdGlobal(binar, &binar, 128));
		COMPV_CHECK_CODE_RETURN(ccl_obj->process(binar, &result));
		//const CompVConnectedComponentLabelingResultLSL* result_lsl =
		//	CompVConnectedComponentLabeling::reinterpret_castr<CompVConnectedComponentLabelingResultLSL>(result);
		COMPV_CHECK_CODE_RETURN(check_labels(result, test));
		COMPV_CHECK_CODE_RETURN(check_segments(result, test));
		COMPV_CHECK_CODE_RETURN(check_blobs(result, binar));
		COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");
	}

	return COMPV_ERROR_CODE_S_OK;
}


static COMPV_ERROR_CODE __extract(const CompVConnectedComponentLabelingResultPtr& result, const COMPV_CCL_EXTRACT_TYPE type, const size_t width, const size_t height, CompVMatPtrPtr ptr8uOut);
static COMPV_ERROR_CODE __blitPoints(const CompVConnectedComponentPointsVector& points, const size_t width, const size_t height, CompVMatPtrPtr ptr8uOut);

static COMPV_ERROR_CODE check_labels(const CompVConnectedComponentLabelingResultPtr& result, const compv_unittest_ccl* test)
{
	CompVMatPtr ptr32sLabels;
	COMPV_CHECK_CODE_RETURN(result->debugFlatten(&ptr32sLabels));
	COMPV_CHECK_EXP_RETURN(std::string(test->md5_labels).compare(compv_tests_md5(ptr32sLabels)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "CCL MD5 mismatch (labels)");
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE check_segments(const CompVConnectedComponentLabelingResultPtr& result, const compv_unittest_ccl* test)
{
	CompVMatPtr ptr8uSegments;
	COMPV_CHECK_CODE_RETURN(__extract(result, COMPV_CCL_EXTRACT_TYPE_SEGMENT, test->width, test->height, &ptr8uSegments));
#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(ptr8uSegments, TEST_TYPE));
#endif
	COMPV_CHECK_EXP_RETURN(compv_tests_md5(ptr8uSegments).compare(test->md5_segments) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "CCL MD5 mismatch (segments)");
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE check_blobs(const CompVConnectedComponentLabelingResultPtr& result, const CompVMatPtr& ptr8uInput)
{
	CompVMatPtr ptr8uRestored;
	COMPV_CHECK_CODE_RETURN(__extract(result, COMPV_CCL_EXTRACT_TYPE_BLOB, ptr8uInput->cols(), ptr8uInput->rows(), &ptr8uRestored));
	COMPV_CHECK_EXP_RETURN(compv_tests_md5(ptr8uRestored).compare(compv_tests_md5(ptr8uInput)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "CCL MD5 mismatch (blobs)");
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE __extract(const CompVConnectedComponentLabelingResultPtr& result, const COMPV_CCL_EXTRACT_TYPE type, const size_t width, const size_t height, CompVMatPtrPtr ptr8uOut)
{
	CompVConnectedComponentPointsVector points;
	COMPV_CHECK_CODE_RETURN(result->extract(points, type));
	COMPV_CHECK_CODE_RETURN(__blitPoints(points, width, height, ptr8uOut));
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE __blitPoints(const CompVConnectedComponentPointsVector& points, const size_t width, const size_t height, CompVMatPtrPtr ptr8uOut)
{
	CompVMatPtr ptr8uOut_ = *ptr8uOut;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&ptr8uOut_, height, width));
	COMPV_CHECK_CODE_RETURN(ptr8uOut_->zero_all());
	for (CompVConnectedComponentPointsVector::const_iterator i = points.begin(); i < points.end(); ++i) {
		for (CompVConnectedComponentPoints::const_iterator j = i->begin(); j < i->end(); ++j) {
			*ptr8uOut_->ptr<uint8_t>(static_cast<size_t>(j->y), static_cast<size_t>(j->x)) = 0xff;
		}
	}
	*ptr8uOut = ptr8uOut_;
	return COMPV_ERROR_CODE_S_OK;
}