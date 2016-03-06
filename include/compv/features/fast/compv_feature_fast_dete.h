/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#if !defined(_COMPV_FEATURES_FAST_DETE_H_)
#define _COMPV_FEATURES_FAST_DETE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/features/compv_feature.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct RangeFAST {
	const uint8_t* IP;
	const uint8_t* IPprev;
	uint8_t* strengths;
	int32_t rowStart;
	int32_t rowEnd;
	int32_t rowCount;
	int32_t width;
	int32_t stride;
	int32_t threshold;
	int32_t N;
	const compv_scalar_t(*pixels16)[16];
};

class CompVFeatureDeteFAST : public CompVFeatureDete
{
protected:
	CompVFeatureDeteFAST();
public:
	virtual ~CompVFeatureDeteFAST();
	virtual COMPV_INLINE const char* getObjectId() { return "CompVFeatureDeteFAST"; };
	
	// override CompVSettable::set
	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
	// override CompVFeatureDete::process
	virtual COMPV_ERROR_CODE process(const CompVObjWrapper<CompVImage*>& image, CompVObjWrapper<CompVBoxInterestPoint* >& interestPoints);

	static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVFeatureDete* >* fast);

private:
	int32_t m_iThreshold;
	int32_t m_iType;
	int32_t m_iNumContinuous;
	int32_t m_iMaxFeatures;
	bool m_bNonMaximaSupp;
	int32_t m_nWidth;
	int32_t m_nHeight;
	int32_t m_nStride;
	RangeFAST* m_pRanges;
	int32_t m_nRanges;
	uint8_t* m_pStrengthsMap;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_FAST_DETE_H_ */
