/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_mat.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVMat::CompVMat()
    : m_pDataPtr(NULL)
    , m_nCompCount(0)
    , m_bCompInterleaved(false)
    , m_nCols(0)
    , m_nRows(0)
    , m_nStrideInBytes(0)
    , m_nStrideInElts(0)
    , m_nElmtInBytes(0)
    , m_nAlignV(0)
    , m_nDataSize(0)
    , m_nDataCapacity(0)
    , m_bOweMem(true)
    , m_eType(COMPV_MAT_TYPE_RAW)
    , m_eSubType(COMPV_MAT_SUBTYPE_RAW)
{
    for (size_t compId = 0; compId < COMPV_MAT_MAX_COMP_COUNT; ++compId) {
        m_nCompCols[compId] = 0;
        m_nCompRows[compId] = 0;
        m_pCompPtr[compId] = NULL;
        m_nCompSizeInBytes[compId] = 0;
        m_nCompStrideInBytes[compId] = 0;
        m_nCompStrideInElts[compId] = 0;
        m_bCompStrideInEltsIsIntegral[compId] = true;
    }
}
CompVMat::~CompVMat()
{
    if (m_bOweMem) {
        CompVMem::free((void**)&m_pDataPtr);
    }
}

COMPV_NAMESPACE_END()
