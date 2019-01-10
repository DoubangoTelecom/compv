/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GPU_FEATURE_FAST_DETE_H_)
#define _COMPV_GPU_FEATURE_FAST_DETE_H_

#include "compv/gpu/compv_gpu_config.h"
#include "compv/gpu/compv_gpu.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(GpuCornerDeteFAST);

typedef COMPV_ERROR_CODE(*newObjGpuCornerDeteFAST)(CompVGpuCornerDeteFASTPtrPtr fast);

class COMPV_GPU_API CompVGpuCornerDeteFAST : public CompVObj
{
	friend class CompVGpu;
protected:
	CompVGpuCornerDeteFAST();
public:
	virtual ~CompVGpuCornerDeteFAST();
	COMPV_OBJECT_GET_ID(CompVGpuCornerDeteFAST);

	virtual COMPV_ERROR_CODE processData(
		const uint8_t* IP, 
		compv_uscalar_t width, 
		compv_uscalar_t height,
		compv_uscalar_t stride,
		compv_uscalar_t N, 
		compv_uscalar_t threshold, 
		uint8_t* strengths) = 0;
	
	static COMPV_ERROR_CODE newObj(CompVGpuCornerDeteFASTPtrPtr fast);

private:
	static newObjGpuCornerDeteFAST s_ptrNewObj;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_GPU_FEATURE_FAST_DETE_H_ */
