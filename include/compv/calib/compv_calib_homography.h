/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CALIB_HOMOGRAPHY_H_)
#define _COMPV_CALIB_HOMOGRAPHY_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"
#include "compv/compv_box.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVHomography
{
public:
	static COMPV_ERROR_CODE find(const CompVPtrArray(T) &src, const CompVPtrArray(T) &dst, CompVPtrArray(T) &H, COMPV_MODELEST_TYPE model = COMPV_MODELEST_TYPE_RANSAC);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CALIB_HOMOGRAPHY_H_ */
