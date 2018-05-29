#include "../tests_common.h"

#define TAG_TEST			"TestPCA"
#define LOOP_COUNT			1


#define NUM_OBSERVATIONS		100
#define OBSERVATION_DIM			441
#define OBSERVATION_ROW_BASED	true // each row is an observation
#define PCA_DIM					92
#define FILE_OUT_PATH			"pca.json"

COMPV_ERROR_CODE pca()
{
	CompVMathPCAPtr pca;

	CompVMatPtr observations;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&observations, NUM_OBSERVATIONS, OBSERVATION_DIM));
	COMPV_CHECK_CODE_RETURN(observations->zero_all());
	
	COMPV_CHECK_CODE_RETURN(CompVMathPCA::newObj(&pca));

	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(pca->compute(observations, PCA_DIM, OBSERVATION_ROW_BASED));
		COMPV_CHECK_CODE_RETURN(pca->write(FILE_OUT_PATH));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();
	
	COMPV_DEBUG_INFO_EX(TAG_TEST, "Elapsed time(PCA) = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	return COMPV_ERROR_CODE_S_OK;
}
