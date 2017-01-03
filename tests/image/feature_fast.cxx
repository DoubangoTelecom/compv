#include "../tests_common.h"

COMPV_ERROR_CODE feature_fast()
{
	COMPV_ERROR_CODE err;
	CompVMatPtr image;
	std::string md5; // FIXME: remove

	COMPV_CHECK_CODE_BAIL(err = COMPV_ERROR_CODE_S_OK, "Just to avoid 'bail not referenced warning'");

	// Decode the jpeg image
	COMPV_CHECK_CODE_RETURN(CompVImageDecoder::decodeFile("C:/Projects/GitHub/pan360/tests/sphere_mapping/7019363969_a80a5d6acc_o.jpg", &image));
	md5 = compv_tests_md5(image);

bail:
	return err;
}