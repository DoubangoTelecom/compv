#include "../tests_common.h"

// Depending on the number of threads, the mean value could be "+-1" compared to the single threaded version
// -> Unit-test -> use 8 threads

#define TAG_TEST								"TestImageScale"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER				"C:/Projects/GitHub/data/test_images"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/test_images"
#else
#	define COMPV_TEST_IMAGE_FOLDER				NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

#define LOOP_COUNT				1
#define FILE_NAME				FILE_NAME_EQUIRECTANGULAR
#define CANNY_THRESHOLD_LOW		0.8f
#define CANNY_THRESHOLD_HIGH	CANNY_THRESHOLD_LOW*2.f	

#define HOUGHSTD_RHO			1.f
#define HOUGHSTD_THETA			kfMathTrigPiOver180 // radian(1d)
#define HOUGHSTD_THRESHOLD		100

static const struct compv_unittest_houghstd {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_HOUGHSTD[] =
{
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "db04608c566f9a7837b467d6e06254ed" },
	{ FILE_NAME_OPENGLBOOK, 200, 258, 200, "5aac2c4794613d4c20a1b158927db80f" },
	{ FILE_NAME_GRIOTS, 480, 640, 480, "fb95536e46704238f1ebf76cb52cebe8" },
};
static const size_t COMPV_UNITTEST_HOUGHSTD_COUNT = sizeof(COMPV_UNITTEST_HOUGHSTD) / sizeof(COMPV_UNITTEST_HOUGHSTD[0]);

static const std::string houghstd_md5(const CompVHoughLineVector& lines) {
	if (lines.empty()) {
		return std::string(COMPV_MD5_EMPTY);
	}
	CompVMatPtr entries;
	const size_t count = lines.size();
	COMPV_CHECK_CODE_ASSERT((CompVMat::newObjAligned<CompVHoughLine, COMPV_MAT_TYPE_STRUCT>(&entries, 1, count)));
	CompVHoughLine* entriesPtr = entries->ptr<CompVHoughLine>();
	for (size_t i = 0; i < count; ++i) {
		entriesPtr[i] = lines[i];
	}
	return compv_tests_md5(entries);
}

COMPV_ERROR_CODE houghstd()
{
	CompVEdgeDetePtr canny;
	CompVHoughPtr houghstd;
	CompVMatPtr image, edges;
	CompVHoughLineVector lines;
	const compv_unittest_houghstd* test = NULL;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_HOUGHSTD_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_HOUGHSTD[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_HOUGHSTD[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&canny, COMPV_CANNY_ID, CANNY_THRESHOLD_LOW, CANNY_THRESHOLD_HIGH));
	COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&houghstd, COMPV_HOUGH_STANDARD_ID, HOUGHSTD_RHO, HOUGHSTD_THETA, HOUGHSTD_THRESHOLD));
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &image));
	COMPV_CHECK_CODE_RETURN(canny->process(image, &edges));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(houghstd->process(edges, lines));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Houghstd Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5: %s", houghstd_md5(lines).c_str());

#if COMPV_OS_WINDOWS
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(edges, "houghstd.gray"));
#endif

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(houghstd_md5(lines)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Houghstd MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
