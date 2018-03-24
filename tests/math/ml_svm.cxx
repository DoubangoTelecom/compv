#include "../tests_common.h"

#define TAG_TEST			"TestMLSVM"
#define LOOP_COUNT			1

#define SVM_MODEL_FILE		"C:/Projects/libsvm-322/windows/multiclass.train.model"

COMPV_ERROR_CODE ml_svm_predict()
{
	CompVMachineLearningSVMPtr mlSVM;

	// Load model
	COMPV_CHECK_CODE_RETURN(CompVMachineLearningSVM::load(SVM_MODEL_FILE, &mlSVM));


	return COMPV_ERROR_CODE_S_OK;
}
