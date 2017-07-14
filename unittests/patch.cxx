#include "../tests/tests_common.h"

#define TAG_TEST								"UnitTestPatch"

COMPV_ERROR_CODE compv_unittest_patch_moments0110()
{
	COMPV_DEBUG_INFO_EX(TAG_TEST, "== Trying new test: Image moments -> 0110 ==");
	static const size_t diameter = 31;
	static const size_t radius = diameter >> 1;
	static const std::string& MD5_01 = "cf0847a95e09eec5115991b3b05412fd";
	static const std::string& MD5_10 = "3657f91595bc8064a0e7743d81c796f9";

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	CompVPatchPtr patch;
	COMPV_CHECK_CODE_RETURN(CompVPatch::newObj(&patch, static_cast<int>(diameter)));

	CompVMatPtr image;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned(&image, 111, 615));
	uint8_t* imagePtr = image->ptr<uint8_t>();
	for (size_t j = 0; j < image->rows(); ++j) {
		size_t rr = j * image->cols();
		for (size_t i = 0; i < image->cols(); ++i) {
			imagePtr[i] = static_cast<uint8_t>((i + rr + (i & 31) + (j & 15)) & 0xff);
		}
		imagePtr += image->stride();
	}

	size_t mcols = image->cols() - diameter;
	size_t mrows = image->rows() - diameter;
	CompVMatPtr moments01, moments10;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&moments01, mrows, mcols));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int32_t>(&moments10, mrows, mcols));
	COMPV_CHECK_CODE_RETURN(moments01->zero_rows());
	COMPV_CHECK_CODE_RETURN(moments10->zero_rows());

	int m01, m10;
	imagePtr = image->ptr<uint8_t>();
	size_t imgWidth = image->cols();
	size_t imgHeight = image->rows();
	size_t imgStride = image->stride();
	int32_t* moments01Ptr = moments01->ptr<int32_t>();
	int32_t* moments10Ptr = moments10->ptr<int32_t>();
	for (size_t j = 0; j < mrows; ++j) {
		for (size_t i = 0; i < mcols; ++i) {
			COMPV_CHECK_CODE_RETURN(patch->moments0110(image->ptr<uint8_t>(), static_cast<int>(i + radius), static_cast<int>(j + radius), imgWidth, imgHeight, imgStride, &m01, &m10));
			moments01Ptr[i] = static_cast<int32_t>(m01);
			moments10Ptr[i] = static_cast<int32_t>(m10);
		}
		moments01Ptr += moments01->stride();
		moments10Ptr += moments10->stride();
	}

	COMPV_CHECK_EXP_BAIL(MD5_01.compare(compv_tests_md5(moments01)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image moments MD5_01 mismatch");
	COMPV_CHECK_EXP_BAIL(MD5_10.compare(compv_tests_md5(moments10)) != 0, (err = COMPV_ERROR_CODE_E_UNITTEST_FAILED), "Image moments MD5_10 mismatch");

	COMPV_DEBUG_INFO_EX(TAG_TEST, "** Test OK **");

bail:
	return err;
}
