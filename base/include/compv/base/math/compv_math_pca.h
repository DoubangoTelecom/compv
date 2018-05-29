/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_PCA_H_)
#define _COMPV_BASE_MATH_PCA_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(MathPCA)

class COMPV_BASE_API CompVMathPCA : public CompVObj
{
protected:
	CompVMathPCA();
public:
	virtual ~CompVMathPCA();
	COMPV_OBJECT_GET_ID(CompVMathPCA);

	COMPV_ERROR_CODE compute(const CompVMatPtr& observations, const int maxDimensions, const bool rowBased);

	static COMPV_ERROR_CODE read(const char* filePath, CompVMathPCAPtrPtr pca);
	static COMPV_ERROR_CODE write(const char* filePath, const CompVMathPCAPtr& pca);
	static COMPV_ERROR_CODE newObj(CompVMathPCAPtrPtr pca);

private:
	COMPV_INLINE bool isReady() const {
		return (m_ptr32fMean && m_ptr32fEigenVectors && m_ptr32fEigenValues);
	}

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVMatPtr m_ptr32fMean;
	CompVMatPtr m_ptr32fEigenVectors;
	CompVMatPtr m_ptr32fEigenValues;
	int m_nMaxDimensions;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATH_PCA_H_ */
