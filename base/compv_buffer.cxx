/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_buffer.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

#define COMPV_THIS_CLASSNAME	"CompVBuffer"

COMPV_NAMESPACE_BEGIN()

CompVBuffer::CompVBuffer(const void* pcPtr COMPV_DEFAULT(nullptr), size_t size COMPV_DEFAULT(0))
    : CompVObj()
    , m_pPtr(nullptr)
    , m_nSize(0)
    , m_bOweMem(true)
{
    if (size > 0) { // NULL pcPtr means create a buffer with random data
        COMPV_ASSERT(COMPV_ERROR_CODE_IS_OK(copyData(pcPtr, size)));
    }
}

CompVBuffer::~CompVBuffer()
{
    if (m_bOweMem) {
        CompVMem::free(&m_pPtr);
    }
}

COMPV_ERROR_CODE CompVBuffer::copyData(const void* pcPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    if (m_bOweMem) {
        COMPV_CHECK_EXP_RETURN(!(m_pPtr = CompVMem::realloc(m_pPtr, size + sizeof(double))), COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    else {
		COMPV_CHECK_EXP_RETURN(!(m_pPtr = CompVMem::malloc(size + sizeof(double))), COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        m_bOweMem = true;
    }
    m_nSize = size;
    if (pcPtr) {
        CompVMem::copy(m_pPtr, pcPtr, size);
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBuffer::refData(const void* pcPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!pcPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    if (m_bOweMem) {
        CompVMem::free(&m_pPtr);
    }
    m_pPtr = (void*)pcPtr;
    m_nSize = size;
    m_bOweMem = false;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBuffer::takeData(void** ppPtr, size_t size)
{
    COMPV_CHECK_EXP_RETURN(!ppPtr || !*ppPtr || !size, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    if (m_bOweMem) {
        CompVMem::free(&m_pPtr);
    }
    m_pPtr = *ppPtr, *ppPtr = NULL;
    m_nSize = size;
    m_bOweMem = true;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBuffer::newObj(const void* pcPtr, size_t size, CompVBufferPtrPtr buffer)
{
    COMPV_CHECK_EXP_RETURN(!buffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVBufferPtr buffer_;

    buffer_ = new CompVBuffer(pcPtr, size);
    if (!buffer_ || buffer_->m_nSize != size) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *buffer = buffer_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBuffer::newObjAndNullData(CompVBufferPtrPtr buffer)
{
    return CompVBuffer::newObj(nullptr, 0, buffer);
}

COMPV_ERROR_CODE CompVBuffer::newObjAndTakeData(void** ppPtr, size_t size, CompVBufferPtrPtr buffer)
{
    COMPV_CHECK_EXP_RETURN(!buffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    CompVBufferPtr buffer_;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    COMPV_CHECK_CODE_BAIL(err = CompVBuffer::newObjAndNullData(&buffer_));
    COMPV_CHECK_CODE_BAIL(err = buffer_->takeData(ppPtr, size));
    *buffer = buffer_;

bail:
    return err;
}

COMPV_ERROR_CODE CompVBuffer::newObjAndCopyData(const void* pcPtr, size_t size, CompVBufferPtrPtr buffer)
{
    COMPV_CHECK_EXP_RETURN(!buffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    CompVBufferPtr buffer_;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    COMPV_CHECK_CODE_BAIL(err = CompVBuffer::newObjAndNullData(&buffer_));
    COMPV_CHECK_CODE_BAIL(err = buffer_->copyData(pcPtr, size));
    *buffer = buffer_;

bail:
    return err;
}

COMPV_ERROR_CODE CompVBuffer::newObjAndRefData(const void* pcPtr, size_t size, CompVBufferPtrPtr buffer)
{
    COMPV_CHECK_EXP_RETURN(!buffer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVBufferPtr buffer_;
    COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

    COMPV_CHECK_CODE_BAIL(err = CompVBuffer::newObjAndNullData(&buffer_));
    COMPV_CHECK_CODE_BAIL(err = buffer_->refData(pcPtr, size));
    *buffer = buffer_;

bail:
    return err;
}

COMPV_NAMESPACE_END()
