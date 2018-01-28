#include "../tests_common.h"

#define TAG_TEST								"TestBitsOp"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER			"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_FOLDER			NULL
#endif
#define COMPV_TEST_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define FILE_NAME_EQUIRECTANGULAR		"equirectangular_1282x720_gray.yuv"
#define FILE_NAME_OPENGLBOOK			"opengl_programming_guide_8th_edition_200x258_gray.yuv"
#define FILE_NAME_GRIOTS				"mandekalou_480x640_gray.yuv"

#define BITS_OP_AND						0
#define BITS_OP_NOT_AND					1
#define BITS_OP_NOT						2





#define LOOP_COUNT			1
#define FILE_NAME			FILE_NAME_EQUIRECTANGULAR
#define BITS_OP				BITS_OP_NOT

static const struct compv_unittest_bits {
	const char* filename;
	size_t width;
	size_t height;
	size_t stride;
	const char* md5;
}
COMPV_UNITTEST_BITS[] =
{
#if BITS_OP == BITS_OP_AND
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "0d5f7f7b0cd1dce06243c62282cced51" },
#elif BITS_OP == BITS_OP_NOT_AND
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "e631fc32b9ed1a4f87689905de5d30e9" },
#elif BITS_OP == BITS_OP_NOT
	{ FILE_NAME_EQUIRECTANGULAR, 1282, 720, 1282, "836a66d41cab3dec2ad185e220feeeb7" },
#else 
#error "Not implemented"
#endif
};
static const size_t COMPV_UNITTEST_BITS_COUNT = sizeof(COMPV_UNITTEST_BITS) / sizeof(COMPV_UNITTEST_BITS[0]);

COMPV_ERROR_CODE bits_ops()
{
	CompVEdgeDetePtr dete;
	CompVMatPtr image, edges;
	const compv_unittest_bits* test = NULL;

	// Find test
	for (size_t i = 0; i < COMPV_UNITTEST_BITS_COUNT; ++i) {
		if (std::string(COMPV_UNITTEST_BITS[i].filename).compare(FILE_NAME) == 0) {
			test = &COMPV_UNITTEST_BITS[i];
			break;
		}
	}
	if (!test) {
		COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed to find test");
		return COMPV_ERROR_CODE_E_NOT_FOUND;
	}

	// Read file
	CompVMatPtr imageIn;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, test->width, test->height, test->stride, COMPV_TEST_PATH_TO_FILE(test->filename).c_str(), &imageIn));

#if BITS_OP == BITS_OP_AND || BITS_OP == BITS_OP_NOT_AND
	CompVMatPtr imageOp;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&imageOp, test->height, test->width));
	uint8_t* imageOpPtr = imageOp->ptr<uint8_t>();
	const size_t imageOpStride = imageOp->stride();
	for (size_t j = 0; j < test->height; ++j) {
		for (size_t i = 0; i < test->width; ++i) {
			imageOpPtr[i] = static_cast<uint8_t>((j * i) & 0xff);
		}
		imageOpPtr += imageOpStride;
	}
#endif /* BITS_OP == BITS_OP_AND || BITS_OP == BITS_OP_NOT_AND */

	CompVMatPtr imageOut;
	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
#if BITS_OP == BITS_OP_AND
		COMPV_CHECK_CODE_RETURN(CompVBits::and(imageIn, imageOp, &imageOut));
#elif BITS_OP == BITS_OP_NOT_AND
		COMPV_CHECK_CODE_RETURN(CompVBits::not_and(imageIn, imageOp, &imageOut));
#elif BITS_OP == BITS_OP_NOT
		COMPV_CHECK_CODE_RETURN(CompVBits::not(imageIn, &imageOut));
#else 
#error "Not implemented"
#endif
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO("Elapsed time (bits_ops()) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

#if COMPV_OS_WINDOWS && 0
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Do not write the file to the hd");
	COMPV_CHECK_CODE_RETURN(compv_tests_write_to_file(imageOut, FILE_NAME));
#endif

	COMPV_DEBUG_INFO("MD5:%s", compv_tests_md5(imageOut).c_str());

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(imageOut)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "BitsOps MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
