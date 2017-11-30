#include "../tests_common.h"

#define TAG_TEST			"TestDistance"
#define LOOP_COUNT			1



static COMPV_ERROR_CODE __buildPoints(CompVMatPtrPtr points);

COMPV_ERROR_CODE distance_hamming()
{
	static const  size_t width = 2005;
	static const struct compv_unittest_hamming {
		size_t width;
		size_t height;
		const char* md5;
	}
	COMPV_UNITTEST_HAMMING[] = {
		{ 2005, 101, "58de498ac979c64c085f287b14d78241" },
		{ 1280, 720, "2416808864658a997a39007fc1a548c4" },
		{ 32, 725, "a797d9d20176dbd97a801d9b8e2c1778" }, // hamming256
	};

	const compv_unittest_hamming* test = NULL;
	for (size_t i = 0; i < sizeof(COMPV_UNITTEST_HAMMING) / sizeof(COMPV_UNITTEST_HAMMING[i]); ++i) {
		if (COMPV_UNITTEST_HAMMING[i].width == width) {
			test = &COMPV_UNITTEST_HAMMING[i];
			break;
		}
	}

	COMPV_CHECK_EXP_RETURN(!test, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Failed to find test");

	CompVMatPtr patch1xn;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&patch1xn, 1, test->width));
	uint8_t* dataPtr = patch1xn->ptr<uint8_t>();
	for (size_t i = 0; i < patch1xn->cols(); ++i) {
		dataPtr[i] = static_cast<uint8_t>(((i << 1) + 1) & 0xff);
	}

	CompVMatPtr data;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&data, test->height, test->width));
	for (size_t j = 0; j < data->rows(); ++j) {
		dataPtr = data->ptr<uint8_t>(j);
		size_t row = (test->width * j);
		for (size_t i = 0; i < data->cols(); ++i) {
			dataPtr[i] = static_cast<uint8_t>((i + row) & 0xff);
		}
	}

	CompVMatPtr dist;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&dist, 1, test->height)); // each row produce #1 hamming distance -> distance size = (1 x height)

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathDistance::hamming(data->ptr<const uint8_t>(), data->cols(), data->rows(), data->stride(), patch1xn->ptr<const uint8_t>(), dist->ptr<int32_t>()));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(Distance Hamming) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	//COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(dist).c_str());

	COMPV_CHECK_EXP_RETURN(std::string(test->md5).compare(compv_tests_md5(dist)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Distance Hamming: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE distance_line()
{
#define EXPECTED_MD5_LINE		"71436528e135149d19cde751c98d80ac"
#define EXPECTED_MD5_LINE_FMA	"0746fe43fb46124c78ada87d86d8e54e"
	const double lineEq[3] = { -7.258, 45852.0, 5485.65 };
	
	CompVMatPtr points, distance;
	COMPV_CHECK_CODE_RETURN(__buildPoints(&points));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathDistance::line(points, lineEq, &distance));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(Distance line) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(distance).c_str());

	COMPV_CHECK_EXP_RETURN(std::string(compv_tests_is_fma_enabled() ? EXPECTED_MD5_LINE_FMA : EXPECTED_MD5_LINE).compare(compv_tests_md5(distance)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Line distance: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE distance_parabola()
{
#define EXPECTED_MD5_PARABOLA		"d152dac9988060c0659ae393cddfb9f0"
#define EXPECTED_MD5_PARABOLA_FMA	"d152dac9988060c0659ae393cddfb9f0"
	const double parabolaEq[3] = { -7.258, 45852.0, 5485.65 };

	CompVMatPtr points, distance;
	COMPV_CHECK_CODE_RETURN(__buildPoints(&points));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMathDistance::parabola(points, parabolaEq, &distance, COMPV_MATH_PARABOLA_TYPE_SIDEWAYS));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(Distance parabola) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(distance).c_str());

	COMPV_CHECK_EXP_RETURN(std::string(compv_tests_is_fma_enabled() ? EXPECTED_MD5_PARABOLA_FMA : EXPECTED_MD5_PARABOLA).compare(compv_tests_md5(distance)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "Line distance: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}



static COMPV_ERROR_CODE __buildPoints(CompVMatPtrPtr points)
{
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(points, 2, 1000003));
	const size_t count = (*points)->cols();
	const size_t stride = (*points)->stride();
	compv_float32_t* pointsX = (*points)->ptr<compv_float32_t>(0);
	compv_float32_t* pointsY = (*points)->ptr<compv_float32_t>(1);
	for (size_t i = 0; i < count; ++i) {
		pointsX[i] = static_cast<compv_float32_t>(((i * 1983.584) + (i*9.0)) * ((i & 3) ? 1 : -1));
		pointsY[i] = static_cast<compv_float32_t>(((i * 584.1983) + (i*12.0)) * ((i & 8) ? 1 : -1));
	}
	return COMPV_ERROR_CODE_S_OK;
}