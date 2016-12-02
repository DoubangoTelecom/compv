/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_HOUGHSTD_H_)
#define _COMPV_FEATURES_HOUGHSTD_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_box.h"
#include "compv/features/compv_feature.h"
#include "compv/parallel/compv_mutex.h"

#include <vector>

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

// Class not requiring memset(0) to zero buffer
class CompVHoughAcc : public CompVObj
{
protected:
    CompVHoughAcc(size_t rows, size_t cols) : m_nRows(rows), m_nCols(cols) {
        size_t strideInBytes = CompVMem::alignForward(cols * sizeof(int32_t));
        m_pMem = static_cast<uint8_t*>(::calloc(((strideInBytes * rows)) + CompVMem::getBestAlignment(), sizeof(uint8_t))); // Do not use CompVMem::calloc() which call memset
        m_pPtr = reinterpret_cast<int32_t*>(CompVMem::alignForward(reinterpret_cast<uintptr_t>(m_pMem)));
        m_nStride = strideInBytes / sizeof(int32_t);
    }
public:
    virtual ~CompVHoughAcc() {
        if (m_pMem) {
            free(m_pMem);
        }
    }
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVHoughAcc";
    };
    COMPV_INLINE int32_t* ptr(size_t row = 0, size_t col = 0) {
        return (m_pPtr + (row * m_nStride)) + col;
    }
    COMPV_INLINE size_t cols() {
        return m_nCols;
    }
    COMPV_INLINE size_t rows() {
        return m_nRows;
    }
    COMPV_INLINE size_t stride() {
        return m_nStride;
    }
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVHoughAcc*>* acc, size_t rows, size_t cols) {
        COMPV_CHECK_EXP_RETURN(!acc || !rows || !cols, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        CompVHoughAcc* acc_ = new CompVHoughAcc(rows, cols);
        COMPV_CHECK_EXP_RETURN(!acc_, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_CHECK_EXP_RETURN(!acc_->m_pPtr, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        *acc = acc_;
        return COMPV_ERROR_CODE_S_OK;
    }

private:
    int32_t* m_pPtr;
    size_t m_nCols;
    size_t m_nRows;
    size_t m_nStride;
    uint8_t* m_pMem;
};

struct CompVHoughAccThreadsCtx {
    CompVPtr<CompVMutex*> mutex;
    CompVPtr<CompVHoughAcc* > acc;
    size_t threadsCount;
};

class CompVHoughStd : public CompVHough
{
protected:
    CompVHoughStd(float rho = 1.f, float theta = kfMathTrigPiOver180, int32_t threshold = 1);
public:
    virtual ~CompVHoughStd();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVHoughStd";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVHough::process
    virtual COMPV_ERROR_CODE process(const CompVPtrArray(uint8_t)& edges, CompVPtrArray(compv_float32x2_t)& coords);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVHough* >* hough, float rho = 1.f, float theta = kfMathTrigPiOver180, int32_t threshold = 1);

private:
    COMPV_ERROR_CODE initCoords(float fRho, float fTheta, int32_t nThreshold, size_t nWidth = 0, size_t nHeight = 0);
    COMPV_ERROR_CODE acc_gather(size_t rowStart, size_t rowCount, const CompVPtrArray(uint8_t)& edges, CompVHoughAccThreadsCtx* threadsCtx);
    COMPV_ERROR_CODE nms_gather(size_t rowStart, size_t rowCount, CompVPtr<CompVHoughAcc* >& acc);
    COMPV_ERROR_CODE nms_apply(size_t rowStart, size_t rowCount, CompVPtr<CompVHoughAcc* >& acc, CompVPtrBox(CompVCoordPolar2f)& coords);

private:
    float m_fRho;
    float m_fTheta;
    int32_t m_nThreshold;
    int32_t m_nMaxLines;
    size_t m_nWidth;
    size_t m_nHeight;
    size_t m_nBarrier;
    CompVPtrArray(int32_t) m_SinRho;
    CompVPtrArray(int32_t) m_CosRho;
    CompVPtrArray(uint8_t) m_NMS;
    CompVPtrBox(CompVCoordPolar2f) m_Coords;
};

void HoughStdNmsGatherRow_C(const int32_t * pAcc, size_t nAccStride, uint8_t* pNms, int32_t nThreshold, size_t colStart, size_t maxCols);
void HoughStdNmsApplyRow_C(int32_t* pACC, uint8_t* pNMS, int32_t threshold, compv_float32_t theta, int32_t barrier, int32_t row, size_t colStart, size_t maxCols, CompVPtrBox(CompVCoordPolar2f)& coords);

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_HOUGHSTD_H_ */
