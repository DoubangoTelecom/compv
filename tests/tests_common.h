#/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_TESTS_COMMON_H_)
#define _COMPV_TESTS_COMMON_H_

#include <compv/compv_api.h>

using namespace compv;

const COMPV_ERROR_CODE compv_tests_init();

const COMPV_ERROR_CODE compv_tests_deInit();

const std::string compv_tests_path_from_file(const char* filename, const char* optional_folder = NULL);

COMPV_ERROR_CODE compv_tests_write_to_file(const CompVMatPtr& mat, const char* filename);

const std::string compv_tests_md5(const CompVMatPtr& mat);

bool compv_tests_is_fma_enabled();

bool compv_tests_is_rcp();

#endif /* _COMPV_TESTS_COMMON_H_ */
