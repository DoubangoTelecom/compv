#include "../tests_common.h"

#define TAG_TEST								"TestMLSVM"
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

#define SVM_MODEL_LIBSVM_FILE		"C:/Projects/GitHub/ultimateText/thog-trainning/en.thog.svm.model"
#define SVM_MODEL_FLAT_FILE			"C:/Projects/GitHub/ultimateText/thog-trainning/en.thog.svm.flat"
#define THOG_VECTORS				"C:/Projects/GitHub/data/thog/thog00.vectors" // from "Grillage" - should be #(63 x 408)
#define THOG_LABELS					"C:/Projects/GitHub/data/thog/thog00.labels"

// TODO(dmi): a (1654 x 2126) image with huge candidates to use as reference -> C:/Projects/ocr_datasets/Seine-et-Marne-Magazine-2017/CD77-113-WEB/CD77-113-WEB-01.jpg

#define LOOP_COUNT			1

COMPV_ERROR_CODE ml_svm_predict()
{
	CompVBufferPtr labels, vectors;
	const std::string path_labels = CompVFileUtils::patchFullPath(THOG_LABELS);
	const std::string path_vectors = CompVFileUtils::patchFullPath(THOG_VECTORS);
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(path_labels.c_str(), &labels));
	COMPV_CHECK_CODE_RETURN(CompVFileUtils::read(path_vectors.c_str(), &vectors));
	const size_t vec_count = (labels->size() / sizeof(int32_t));
	const size_t vec_len = (vectors->size() / (vec_count * sizeof(compv_float64_t)));
	COMPV_ASSERT(!((vec_count * sizeof(compv_float64_t)) == vectors->size()));

	// Create vectors
	CompVMatPtr matVectors;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&matVectors, vec_count, vec_len)); // #468 vectors
	for (size_t j = 0; j < vec_count; ++j) {
		COMPV_CHECK_CODE_RETURN(CompVMem::copy(matVectors->ptr<compv_float64_t>(j), &reinterpret_cast<const compv_float64_t*>(vectors->ptr())[j * vec_len], matVectors->rowInBytes()));
	}

	// Create expect result
	CompVMatPtr matxResult;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&matxResult, 1, vec_count)); // #468 vectors
	COMPV_CHECK_CODE_RETURN(CompVMem::copy(matxResult->ptr<int32_t>(), labels->ptr(), matxResult->rowInBytes()));

	CompVMachineLearningSVMPredictPtr mlSVM;
	CompVMatPtr matResult;
	COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVMPredict::newObjBinaryRBF(&mlSVM, SVM_MODEL_FLAT_FILE, CompVGpu::isActiveAndEnabled()));
	
	// Warm up the GPU
	if (LOOP_COUNT > 1 && CompVGpu::isActiveAndEnabled()) {
		COMPV_CHECK_CODE_RETURN(mlSVM->process(matVectors, &matResult));
	}

	// Computing
	const uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(mlSVM->process(matVectors, &matResult));
	}
	const uint64_t timeEnd = CompVTime::nowMillis();

	COMPV_DEBUG_INFO_EX(TAG_TEST, "ML SVM BINARY RBF Predict (GPU=%s) Elapsed time = [[[ %" PRIu64 " millis ]]]", CompVBase::to_string(mlSVM->isGPUAccelerated()).c_str(), (timeEnd - timeStart));

	// Check result
	COMPV_ASSERT(matResult->rows() == 1 && matResult->cols() == vec_count);
	for (size_t i = 0; i < matResult->cols(); ++i) {
		const int32_t& xlabel = *matxResult->ptr<const int32_t>(0, i);
		if (*matResult->ptr<const int32_t>(0, i) != xlabel) {
			COMPV_DEBUG_ERROR_EX(TAG_TEST, "Failed at %zu (%d != %d)", i, *matResult->ptr<const int32_t>(0, i), *matxResult->ptr<const int32_t>(0, i));
			return COMPV_ERROR_CODE_E_UNITTEST_FAILED;
		}
	}

	getchar();

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE ml_svm_rbf()
{
	CompVMatPtr yy;
	COMPV_CHECK_CODE_RETURN(CompVImage::read(COMPV_SUBTYPE_PIXELS_Y, 1282, 720, 1282, COMPV_TEST_PATH_TO_FILE(FILE_NAME_EQUIRECTANGULAR).c_str(), &yy));
	// I want the width to be odd (e.g. 1281x721) in order to have orphans
	COMPV_CHECK_CODE_RETURN(CompVImage::scale(yy, &yy, 1283, 721, COMPV_INTERPOLATION_TYPE_BICUBIC_FLOAT32));
	COMPV_CHECK_CODE_RETURN((CompVMathCast::process_static<float, double>(yy, &yy)));

	const size_t cols = yy->cols();

	CompVMatPtr x;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&x, 1, cols));
	double* xPtr = x->ptr<double>();
	xPtr[0] = 1983.745;
	for (size_t i = 1; i < cols; ++i) {
		xPtr[i] = (xPtr[i - 1] * .47777) * ((i & 1) ? -.5555 : .5555);
	}

	CompVMatPtr kValues;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<double>(&kValues, 1, yy->rows()));

	uint64_t timeStart = CompVTime::nowMillis();
	for (size_t i = 0; i < LOOP_COUNT; ++i) {
		COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::rbf(x, yy, yy->rows(), 7.99e-11, kValues));
	}
	uint64_t timeEnd = CompVTime::nowMillis();
	COMPV_DEBUG_INFO_EX(TAG_TEST, "ML SVM RBF Elapsed time = [[[ %" PRIu64 " millis ]]]", (timeEnd - timeStart));

	COMPV_DEBUG_INFO_EX(TAG_TEST, "MD5=%s", compv_tests_md5(kValues).c_str());
	COMPV_CHECK_EXP_RETURN(std::string("09c49d13f8eef8a2aa317c4ac007b642").compare(compv_tests_md5(kValues)) != 0, COMPV_ERROR_CODE_E_UNITTEST_FAILED, "ML SVM RBF MD5 mismatch");

	return COMPV_ERROR_CODE_S_OK;
}
