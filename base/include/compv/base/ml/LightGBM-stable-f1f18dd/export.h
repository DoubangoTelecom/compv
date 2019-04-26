#ifndef LIGHTGBM_EXPORT_H_
#define LIGHTGBM_EXPORT_H_

/** Macros for exporting symbols in MSVC/GCC/CLANG **/

#include "compv/base/compv_config.h"

#define LIGHTGBM_EXPORT		COMPV_BASE_API
#define LIGHTGBM_EXTERN_C	COMPV_EXTERNC
#define LIGHTGBM_C_EXPORT LIGHTGBM_EXTERN_C LIGHTGBM_EXPORT

#endif /** LIGHTGBM_EXPORT_H_ **/
