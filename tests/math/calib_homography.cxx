#include "../tests_common.h"

#define TAG_TEST			"TestCalibHomography"
#define NUM_POINTS			(5000 + 15) // +15 to make it SIMD-unfriendly for testing
#define LOOP_COUNT			1
#define TYP					compv_float32_t
#define ERR_MAX_F64			8.5209617139980764e-17
#define ERR_MAX_F32			6.80796802e-09

COMPV_ERROR_CODE buildHomographyMatrixEq()
{
	COMPV_ALIGN_DEFAULT() TYP srcX[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() TYP srcY[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() TYP dstX[NUM_POINTS];
	COMPV_ALIGN_DEFAULT() TYP dstY[NUM_POINTS];
	CompVMatPtr M;
	const std::string& expectedMD5 = std::is_same<TYP, compv_float32_t>::value
		? "de09ded7d0dd1ef55aa09757a4570e0b"
		: "540181662bad9a3d001b8b8969a7cb5f";

	for (signed i = 0; i < NUM_POINTS; ++i) {
		srcX[i] = static_cast<TYP>(((i & 1) ? i : -i) + 0.5);
		srcY[i] = static_cast<TYP>((srcX[i] * 0.2) + i + 0.7);
		dstX[i] = static_cast<TYP>((srcX[i] * 8.2) + i + 0.7);
		dstY[i] = static_cast<TYP>(((i & 1) ? i : -i) + 8.5);
	}

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMatrix::buildHomographyEqMatrix<TYP>(&M, &srcX[0], &srcY[0], &dstX[0], &dstY[0], NUM_POINTS));
	}
	uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(buildHomographyMatrixEq) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO("MD5: %s", compv_tests_md5(M).c_str());

	COMPV_CHECK_EXP_RETURN(expectedMD5.compare(compv_tests_md5(M)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "buildHomographyMatrixEq: MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}