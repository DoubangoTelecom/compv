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
	const char* md5_points;
}
COMPV_UNITTEST_CCL[] =
{
	{
		TEST_TYPE_OCR0, 826, 633, 826,
		"7acca8c8f80b363ed60f7aec588039b6", // md5_moments
		"1f9a4e09e09ab1f5ab9298a9561d2ec7", // md5_boxes
		"2cb4df29eb927a183bddb23986cbb7fb", // md5_points
	},
	{
		TEST_TYPE_OCR1, 354, 328, 354,
		"997194f3d1c00ed8cd71ddf157023a71", // md5_moments
		"274bf84468e87d85a9decea09cb0fdf0", // md5_boxes
		"1bc22b3b223b6c83204f5bd959cfbc1b", // md5_points
	}
	,
	{
		TEST_TYPE_OCR2, 1122, 1182, 1122,
		"931cd6224c24b16b5794cd51c3c26486", // md5_moments
		"6d26b58743a36ea58d79233a9b3ceeeb", // md5_boxes
		"e69a039b23ce2070f2af5245968f49cc", // md5_points
	}
};
static const size_t COMPV_UNITTEST_CCL_COUNT = sizeof(COMPV_UNITTEST_CCL) / sizeof(COMPV_UNITTEST_CCL[0]);

#define LOOP_COUNT		100
#define TEST_TYPE		TEST_TYPE_OCR2

#define DELTA			2
#define MIN_AREA		(0.0055 * 0.0055)
#define MAX_AREA		(0.8 * 0.15)
#define MAX_VARIATION	0.3
#define MIN_DIVERSITY	0.2
#define CONNECTIVITY	8

static COMPV_ERROR_CODE check_points(const CompVConnectedComponentLabelingResultPtr result, const compv_unittest_ccl* test);
static COMPV_ERROR_CODE check_boxes(const CompVConnectedComponentLabelingResultPtr result, const CompVMatPtr& image, const compv_unittest_ccl* test);
static COMPV_ERROR_CODE check_moments(const CompVConnectedComponentLabelingResultPtr result, const compv_unittest_ccl* test);

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
	
	COMPV_CHECK_CODE_RETURN(check_points(result, test));
	COMPV_CHECK_CODE_RETURN(check_boxes(result, image, test));
	COMPV_CHECK_CODE_RETURN(check_moments(result, test));


	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE check_points(const CompVConnectedComponentLabelingResultPtr result, const compv_unittest_ccl* test)
{
	CompVMatPtr ptr8uPoints_;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&ptr8uPoints_, test->height, test->width));
	COMPV_CHECK_CODE_RETURN(ptr8uPoints_->zero_all());

	const CompVConnectedComponentLabelingResultLMSER* result_lmser =
		CompVConnectedComponentLabeling::reinterpret_castr<CompVConnectedComponentLabelingResultLMSER>(result);

	const CompVConnectedComponentLabelingRegionMserVector& regions = result_lmser->points();
	size_t numPoints = 0;

	for (CompVConnectedComponentLabelingRegionMserVector::const_iterator i = regions.begin(); i < regions.end(); ++i) {
		const CompVConnectedComponentPoints& pp = i->points;
		numPoints += pp.size();
		for (CompVConnectedComponentPoints::const_iterator p = pp.begin(); p < pp.end(); ++p) {
			*ptr8uPoints_->ptr<uint8_t>(p->y, p->x) = 0xff;
		}
	}

	COMPV_DEBUG_INFO("MD5:%s", compv_tests_md5(ptr8uPoints_).c_str());

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(ptr8uPoints_).compare(test->md5_points) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "CCL MD5 mismatch (points)");

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(ptr8uPoints_, TEST_TYPE));
#endif

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE check_boxes(const CompVConnectedComponentLabelingResultPtr result, const CompVMatPtr& image, const compv_unittest_ccl* test)
{
	CompVMatPtr ptr8uBoxes_;
	COMPV_CHECK_CODE_RETURN(image->clone(&ptr8uBoxes_));
	const size_t stride = ptr8uBoxes_->stride();

	const CompVConnectedComponentLabelingResultLMSER* result_lmser =
		CompVConnectedComponentLabeling::reinterpret_castr<CompVConnectedComponentLabelingResultLMSER>(result);

	const CompVConnectedComponentLabelingRegionMserVector& regions = result_lmser->boundingBoxes();
	const size_t count = regions.size();
	
	for (CompVConnectedComponentLabelingRegionMserVector::const_iterator i = regions.begin(); i < regions.end(); ++i) {
		const CompVConnectedComponentBoundingBox& bb = (*i).boundingBox;
		uint8_t* top = ptr8uBoxes_->ptr<uint8_t>(static_cast<size_t>(bb.top));
		uint8_t* bottom = ptr8uBoxes_->ptr<uint8_t>(static_cast<size_t>(bb.bottom));
		// top and bottom hz lines
		for (int16_t k = bb.left; k <= bb.right; ++k) {
			top[k] = 0;
			bottom[k] = 0;
		}
		// vt lines
		for (top = top + 1; top < bottom; top += stride) {
			top[bb.left] = 0;
			top[bb.right] = 0;
		}
	}

	//COMPV_DEBUG_INFO("MD5:%s", compv_tests_md5(ptr8uBoxes_).c_str());

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(ptr8uBoxes_).compare(test->md5_boxes) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "CCL MD5 mismatch (boxes)");

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(ptr8uBoxes_, TEST_TYPE));
#endif

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE check_moments(const CompVConnectedComponentLabelingResultPtr result, const compv_unittest_ccl* test)
{
	const CompVConnectedComponentLabelingResultLMSER* result_lmser =
		CompVConnectedComponentLabeling::reinterpret_castr<CompVConnectedComponentLabelingResultLMSER>(result);
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Moments checking disabled");
#if 0
	const CompVConnectedComponentMomentsVector& moments = result_lmser->moments();
	const size_t count = moments.size();

	CompVMatPtr ptr64fMoments;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObj<double>(&ptr64fMoments, 6, count, 1));
	double* ptr64fMomentsPtr0 = ptr64fMoments->ptr<double>(0);
	double* ptr64fMomentsPtr1 = ptr64fMoments->ptr<double>(1);
	double* ptr64fMomentsPtr2 = ptr64fMoments->ptr<double>(2);
	double* ptr64fMomentsPtr3 = ptr64fMoments->ptr<double>(3);
	double* ptr64fMomentsPtr4 = ptr64fMoments->ptr<double>(4);
	double* ptr64fMomentsPtr5 = ptr64fMoments->ptr<double>(5);

	for (size_t i = 0; i < count; ++i) {
		const CompVConnectedComponentMoments& moments_ = moments[i];
		ptr64fMomentsPtr0[i] = moments_[0];
		ptr64fMomentsPtr1[i] = moments_[1];
		ptr64fMomentsPtr2[i] = moments_[2];
		ptr64fMomentsPtr3[i] = moments_[3];
		ptr64fMomentsPtr5[i] = moments_[4];
		ptr64fMomentsPtr5[i] = moments_[5];
	}

	//COMPV_DEBUG_INFO("MD5:%s", compv_tests_md5(ptr64fMoments).c_str());

	COMPV_CHECK_EXP_RETURN(compv_tests_md5(ptr64fMoments).compare(test->md5_moments) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "CCL MD5 mismatch (moments)");
#endif
	return COMPV_ERROR_CODE_S_OK;
}