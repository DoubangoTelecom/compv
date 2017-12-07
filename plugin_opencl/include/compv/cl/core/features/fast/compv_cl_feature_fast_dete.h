/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CL_FEATURE_FAST_DETE_H_)
#define _COMPV_CL_FEATURE_FAST_DETE_H_

#include "compv/cl/compv_cl_config.h"
#include "compv/gpu/core/features/fast/compv_gpu_feature_fast_dete.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"

#include <CL/opencl.h>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CLCornerDeteFAST);

class COMPV_CL_API CompVCLCornerDeteFAST : public CompVGpuCornerDeteFAST
{
protected:
	CompVCLCornerDeteFAST();
public:
	virtual ~CompVCLCornerDeteFAST();
	COMPV_OBJECT_GET_ID(CompVCLCornerDeteFAST);

	virtual COMPV_ERROR_CODE processData(
		const uint8_t* IP,
		compv_uscalar_t width,
		compv_uscalar_t height,
		compv_uscalar_t stride,
		compv_uscalar_t N,
		compv_uscalar_t threshold,
		uint8_t* strengths) override;

	static COMPV_ERROR_CODE newObj(CompVCLCornerDeteFASTPtrPtr fast);

private:
	COMPV_ERROR_CODE deInit();
	COMPV_ERROR_CODE init(const uint8_t* IP, uint8_t* strengths, unsigned int width, unsigned int height, unsigned int stride, unsigned char N, unsigned char threshold);

private:
	bool m_bInitialized;
	cl_program m_clProgram;
	cl_kernel m_clKernel;
	cl_mem m_clMemIP;
	cl_mem m_clMemStrengths;
	cl_mem m_clMemPixels16;
	compv_uscalar_t m_nWidth;
	compv_uscalar_t m_nHeight;
	compv_uscalar_t m_nStride;
	int m_arrayPixels16[16];
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CL_FEATURE_FAST_DETE_H_ */
