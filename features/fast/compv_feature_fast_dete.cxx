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

/* @description
This class implement FAST (Features from Accelerated Segment Test) algorithm.
Some literature about FAST:
	- http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/AV1011/AV1FeaturefromAcceleratedSegmentTest.pdf
	- https://en.wikipedia.org/wiki/Features_from_accelerated_segment_test
	- http://www.edwardrosten.com/work/fast.html
	- http://web.eecs.umich.edu/~silvio/teaching/EECS598_2010/slides/11_16_Hao.pdf
*/
#include "compv/features/fast/compv_feature_fast_dete.h"
#include "compv/compv_mathutils.h"

COMPV_NAMESPACE_BEGIN()

CompVFeatureDeteFAST::CompVFeatureDeteFAST()
    : m_iThreshold(COMPV_FEATURE_DETE_FAST_THRESHOLD_DEFAULT)
    , m_iN(COMPV_FEATURE_DETE_FAST_N_DEFAULT)
    , m_bNonMaximaSupp(COMPV_FEATURE_DETE_FAST_NON_MAXIMA_SUPP)
{
}

CompVFeatureDeteFAST::~CompVFeatureDeteFAST()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDeteFAST::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case COMPV_SET_INT32_FAST_THRESHOLD: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int32_t), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        int32_t threshold = *((int32_t*)valuePtr);
        m_iThreshold = COMPV_MATH_CLIP3(0, 255, threshold);
        return COMPV_ERROR_CODE_S_OK;
    }
    case COMPV_SET_BOOL_FAST_NON_MAXIMA_SUPP: {
        COMPV_CHECK_EXP_RETURN(valueSize != sizeof(bool), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        m_bNonMaximaSupp = *((bool*)valuePtr);
        return COMPV_ERROR_CODE_S_OK;
    }
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}



COMPV_ERROR_CODE CompVFeatureDeteFAST::process(const CompVObjWrapper<CompVImage*>& image, std::vector<CompVInterestPoint >& interestPoints)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

    const uint8_t* dataPtr = (const uint8_t*)image->getDataPtr();
    int width = (int)image->getWidth();
    int height = (int)image->getHeight();
    int stride = (int)image->getStride();
	int pad = (stride - width);
	int ndarker, ndrighter;
	uint8_t drighter, darker, Ip, Ix;
	int row, candIdx, p16Idx;
	bool isInterestPoint;

	// clear old points
	interestPoints.clear();

	static const int pixels16[16] = {
		-(stride * 3) + 0, // 1
		-(stride * 3) + 1, // 2
		-(stride * 2) + 2, // 3
		-(stride * 1) + 3, // 4
		+(stride * 0) + 3, // 5
		+(stride * 1) + 3, // 6
		+(stride * 2) + 2, // 7
		+(stride * 3) + 1, // 8
		+(stride * 3) + 0, // 9
		+(stride * 3) - 1, // 10
		+(stride * 2) - 2, // 11
		+(stride * 1) - 3, // 12
		+(stride * 0) - 3, // 13
		-(stride * 1) - 3, // 14
		-(stride * 2) - 2, // 15
		-(stride * 3) - 1, // 16
	};

	/*int num_corners = 0, ret_num_corners = 0;
	xy* corners = fast12_detect((const byte*)dataPtr, width, height, stride, m_iThreshold, &num_corners);
	int* scores = fast12_score((const byte*)dataPtr, stride, corners, num_corners, m_iThreshold);
	xy* nonmax = nonmax_suppression(corners, scores, num_corners, &ret_num_corners);*/
#if 0
	for (int i = 0; i < num_corners; ++i) {
		interestPoints.push_back(CompVInterestPoint(corners[i].x, corners[i].y));
	}
	return err_;
#elif 0
	for (int i = 0; i < ret_num_corners; ++i) {
		interestPoints.push_back(CompVInterestPoint(nonmax[i].x, nonmax[i].y));
	}
	return err_;
#endif
	
	
    for (int j = 3; j < height - 3; ++j) {
		row = j * stride;
		for (int col = 3; col < width - 3; ++col) {
			candIdx = row + col;
			Ip = dataPtr[candIdx];
			drighter = CompVMathUtils::clampPixel8(Ip + m_iThreshold);
			darker = CompVMathUtils::clampPixel8(Ip - m_iThreshold);
			for (int m = 0; m < 16; ++m) {
				ndarker = 0;
				ndrighter = 0;
				for (int k = 0; k < 16; ++k) {
					p16Idx = (m + k) % 16;
					Ix = dataPtr[candIdx + pixels16[p16Idx]];
					ndrighter += (Ix > drighter) ? 1 : -ndrighter;
					ndarker += (Ix < darker) ? 1 : -ndarker;
					if (isInterestPoint = (ndarker == m_iN || ndrighter == m_iN)) {
						// COMPV_DEBUG_INFO("%d,%d is an interest point", col, j);
						interestPoints.push_back(CompVInterestPoint(col, j));
						goto next_point;
					}
				}
			}
		next_point:;
        }
    }

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDeteFAST::newObj(CompVObjWrapper<CompVFeatureDete* >* fast)
{
    COMPV_CHECK_EXP_RETURN(fast == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVFeatureDeteFAST* >_fast = new CompVFeatureDeteFAST();
    if (!_fast) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *fast = *_fast;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
