#include "../tests_common.h"

#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/ocr"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/ocr"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define TEST_TYPE_OCR0			"ocr0_826x633_gray.yuv"
#define TEST_TYPE_OCR1			"ocr1_354x328_gray.yuv"
#define TEST_TYPE_OCR2			"ocr2_1122x1182_gray.yuv"

static const struct compv_unittest_ccl {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5_moments;
	const char* md5_boxes;
}
COMPV_UNITTEST_CCL[] =
{
	{
		TEST_TYPE_OCR0, 826, 633, 826,
		"-", // md5_moments
		"-", // md5_boxes
	},
	{
		TEST_TYPE_OCR1, 354, 328, 354,
		"-", // md5_moments
		"-", // md5_boxes
	}
	,
	{
		TEST_TYPE_OCR2, 1122, 1182, 1122,
		"-", // md5_moments
		"-", // md5_boxes
	}
};
static const size_t COMPV_UNITTEST_CCL_COUNT = sizeof(COMPV_UNITTEST_CCL) / sizeof(COMPV_UNITTEST_CCL[0]);

#define LOOP_COUNT		1
#define TEST_TYPE		TEST_TYPE_OCR1

#define DELTA			2
#define MIN_AREA		0.00005
#define MAX_AREA		0.1
#define MAX_VARIATION	0.5
#define MIN_DIVERSITY	0.33
#define CONNECTIVITY	8

static COMPV_ERROR_CODE check_moments(const CompVConnectedComponentLabelingResultPtr result);
static COMPV_ERROR_CODE check_boxes(const CompVConnectedComponentLabelingResultPtr result, const CompVMatPtr& image);

COMPV_ERROR_CODE ccl_mser()
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
	CompVMatPtr image;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));

	// Create the LSL ccl and set default settings
	CompVConnectedComponentLabelingPtr ccl_obj;
	COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabeling::newObj(&ccl_obj, COMPV_LMSER_ID,
		DELTA,		
		MIN_AREA,	
		MAX_AREA,
		MAX_VARIATION,
		MIN_DIVERSITY,
		CONNECTIVITY)
	);

	CompVConnectedComponentLabelingResultPtr result;
	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(ccl_obj->process(image, &result));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO("Elapsed time (TestConnectedComponentLabeling(MSER)) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_CHECK_CODE_RETURN(check_moments(result));
	COMPV_CHECK_CODE_RETURN(check_boxes(result, image));

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE check_moments(const CompVConnectedComponentLabelingResultPtr result)
{
	const CompVConnectedComponentLabelingResultLMSER* result_lmser =
		CompVConnectedComponentLabeling::reinterpret_castr<CompVConnectedComponentLabelingResultLMSER>(result);

	const CompVConnectedComponentMomentsVector& moments = result_lmser->moments();

	CompVConnectedComponentMoments moments_ = { { 0.0 } };

	for (CompVConnectedComponentMomentsVector::const_iterator i = moments.begin(); i < moments.end(); ++i) {
		moments_[0] += (*i)[0];
		moments_[1] += (*i)[1];
		moments_[2] += (*i)[2];
		moments_[3] += (*i)[3];
		moments_[4] += (*i)[4];
		moments_[5] += (*i)[5];
	}

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE check_boxes(const CompVConnectedComponentLabelingResultPtr result, const CompVMatPtr& image)
{
	CompVMatPtr ptr8uBoxes_;
	COMPV_CHECK_CODE_RETURN(image->clone(&ptr8uBoxes_));
	const size_t stride = ptr8uBoxes_->stride();

	const CompVConnectedComponentLabelingResultLMSER* result_lmser =
		CompVConnectedComponentLabeling::reinterpret_castr<CompVConnectedComponentLabelingResultLMSER>(result);

	const CompVConnectedComponentBoundingBoxesVector& boundingBoxes = result_lmser->boundingBoxes();
	
	for (CompVConnectedComponentBoundingBoxesVector::const_iterator i = boundingBoxes.begin(); i < boundingBoxes.end(); ++i) {
		uint8_t* top = ptr8uBoxes_->ptr<uint8_t>(static_cast<size_t>(i->top));
		uint8_t* bottom = ptr8uBoxes_->ptr<uint8_t>(static_cast<size_t>(i->bottom));
		// top and bottom hz lines
		for (int16_t k = i->left; k <= i->right; ++k) {
			top[k] = 255;
			bottom[k] = 255;
		}
		// vt lines
		for (top = top + 1; top < bottom; top += stride) {
			top[i->left] = 255;
			top[i->right] = 255;
		}
	}

#if COMPV_OS_WINDOWS && 1
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(ptr8uBoxes_, TEST_TYPE));
#endif

	return COMPV_ERROR_CODE_S_OK;
}