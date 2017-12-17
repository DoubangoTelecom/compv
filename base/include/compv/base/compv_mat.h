/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MAT_H_)
#define _COMPV_BASE_MAT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_buffer.h"
#include "compv/base/compv_mem.h"
#include "compv/base/image/compv_image_utils.h"
#include "compv/base/math/compv_math.h"

COMPV_NAMESPACE_BEGIN()
COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Warray-bounds")
COMPV_GCC_DISABLE_WARNINGS_BEGIN("-Wc++11-extensions")

COMPV_OBJECT_DECLARE_PTRS(Mat)

class COMPV_BASE_API CompVMat : public CompVObj
{
protected:
    CompVMat();
public:
    virtual ~CompVMat();
	COMPV_OBJECT_GET_ID(CompVMat);

    COMPV_ALWAYS_INLINE COMPV_MAT_TYPE type()const {
        return m_eType;
    }

    COMPV_ALWAYS_INLINE COMPV_SUBTYPE subType()const {
        return m_eSubType;
    }

    COMPV_ALWAYS_INLINE bool isEmpty()const {
        return !ptr() || !rows() || !cols();
    }

	COMPV_ALWAYS_INLINE bool isPacked()const {
		return m_bPlanePacked;
	}

	COMPV_ALWAYS_INLINE bool isPlanar()const {
		return !m_bPlanePacked;
	}

    COMPV_ALWAYS_INLINE size_t planeCount()const {
        return static_cast<size_t>(m_nPlaneCount);
    }

	COMPV_ALWAYS_INLINE size_t dataSizeInBytes()const {
		return m_nDataSize;
	}

	template<class ptrType = const void>
	COMPV_ALWAYS_INLINE ptrType* data()const {
		return (ptrType*)m_pDataPtr;
	}

	COMPV_ALWAYS_INLINE bool isMemoryOwed()const {
		return m_bOweMem;
	}

    template<class ptrType = const void>
    COMPV_ALWAYS_INLINE ptrType* ptr(size_t row = 0, size_t col = 0, int planeId = -1)const {
		const int planeId_ = planeId < 0 ? 0 : planeId;
        if (planeId_ >= 0 && planeId_ < m_nPlaneCount) {
            return (row > m_nPlaneRows[planeId_] || col > m_nPlaneCols[planeId_]) ? NULL : (ptrType*)(static_cast<const uint8_t*>(m_pCompPtr[planeId_]) + (row * m_nPlaneStrideInBytes[planeId_]) + (col * m_nPlaneElmtInBytes[planeId_]));
        }
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
        return NULL;
    }

    COMPV_ALWAYS_INLINE size_t rows(int planeId = -1)const { // In samples
        if (planeId < 0) {
            return m_nRows;
        }
        else if (planeId < m_nPlaneCount) {
            return m_nPlaneRows[planeId];
        }
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
        return 0;
    }

    COMPV_ALWAYS_INLINE size_t cols(int planeId = -1)const { // In samples
        if (planeId < 0) {
            return m_nCols;
        }
        else if (planeId < m_nPlaneCount) {
            return m_nPlaneCols[planeId];
        }
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
        return 0;
    }

    COMPV_ALWAYS_INLINE size_t elmtInBytes(int planeId = -1) const {
		if (planeId < 0) {
			return m_nElmtInBytes;
		}
		else if (planeId < m_nPlaneCount) {
			return m_nPlaneElmtInBytes[planeId];
		}
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
		return 0;
    }

    COMPV_ALWAYS_INLINE size_t rowInBytes(int planeId = -1)const { // in bytes
        if (planeId < 0) {
            return m_nCols * m_nElmtInBytes;
        }
        else if (planeId < m_nPlaneCount) {
            return m_nPlaneCols[planeId] * m_nPlaneElmtInBytes[planeId];
        }
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
        return 0;
    }

    COMPV_ALWAYS_INLINE size_t strideInBytes(int planeId = -1)const { // in bytes
        if (planeId < 0) {
            return m_nStrideInBytes;
        }
        else if (planeId < m_nPlaneCount) {
            return m_nPlaneStrideInBytes[planeId];
        }
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
        return 0;
    }

	COMPV_ALWAYS_INLINE COMPV_ERROR_CODE strideInElts(size_t &strideInElts, int planeId = 0) {
		COMPV_CHECK_EXP_RETURN(planeId >= m_nPlaneCount, COMPV_ERROR_CODE_E_OUT_OF_BOUND, "planeId out of bound index");
		if (!m_bPlaneStrideInEltsIsIntegral[planeId]) {
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_CALL, "Stride in elements not integral value");
		}
		strideInElts = (planeId == 0) ? m_nStrideInElts : m_nPlaneStrideInElts[planeId];
		return COMPV_ERROR_CODE_S_OK;
	}

    COMPV_ALWAYS_INLINE size_t stride(int planeId = -1)const { // in samples
        if (planeId < 0) {
            return m_nStrideInElts;
        }
        else if (planeId < m_nPlaneCount) {
            return m_nPlaneStrideInElts[planeId];
        }
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
        return 0;
    }

	COMPV_ALWAYS_INLINE size_t planeSizeInBytes(int planeId = -1)const { // in bytes
		if (planeId <= 0) {
			return m_nPlaneSizeInBytes[0];
		}
		else if (planeId < m_nPlaneCount) {
			return m_nPlaneSizeInBytes[planeId];
		}
		COMPV_DEBUG_ERROR_EX("CompVMat", "Invalid parameter %d >= %d", planeId, m_nPlaneCount);
		return 0;
	}

	COMPV_ALWAYS_INLINE size_t alignment()const {
		return m_nAlignV;
	}

	COMPV_ALWAYS_INLINE bool isAligned(int alignv, int planeId = -1)const {
		const void* dataPtr = ptr<const void*>(0, 0, planeId);
		const size_t dataStrideInBytes = strideInBytes(planeId);
		return COMPV_IS_ALIGNED(dataPtr, alignv) && COMPV_IS_ALIGNED(dataStrideInBytes, alignv);
	}
	COMPV_ALWAYS_INLINE bool isAlignedSSE(int planeId = -1)const {
		return isAligned(COMPV_ALIGNV_SIMD_SSE, planeId);
	};
	COMPV_ALWAYS_INLINE bool isAlignedAVX(int planeId = -1)const {
		return isAligned(COMPV_ALIGNV_SIMD_AVX, planeId);
	};
	COMPV_ALWAYS_INLINE bool isAlignedAVX512(int planeId = -1)const {
		return isAligned(COMPV_ALIGNV_SIMD_AVX512, planeId);
	};
	COMPV_ALWAYS_INLINE bool isAlignedNEON(int planeId = -1)const {
		return isAligned(COMPV_ALIGNV_SIMD_NEON, planeId);
	};

	template <typename RawType>
	COMPV_ALWAYS_INLINE bool isRawTypeMatch() const {
		if (m_eType == COMPV_MAT_TYPE_RAW) {
			switch (m_eSubType) {
			case COMPV_SUBTYPE_RAW_INT8: return std::is_same<RawType, int8_t>::value;
			case COMPV_SUBTYPE_RAW_UINT8: return std::is_same<RawType, uint8_t>::value;
			case COMPV_SUBTYPE_RAW_INT16: return std::is_same<RawType, int16_t>::value;
			case COMPV_SUBTYPE_RAW_UINT16: return std::is_same<RawType, uint16_t>::value;
			case COMPV_SUBTYPE_RAW_INT32: return std::is_same<RawType, int32_t>::value;
			case COMPV_SUBTYPE_RAW_UINT32: return std::is_same<RawType, uint32_t>::value;
			case COMPV_SUBTYPE_RAW_SIZE: return std::is_same<RawType, size_t>::value;
			case COMPV_SUBTYPE_RAW_FLOAT32: return std::is_same<RawType, compv_float32_t>::value;
			case COMPV_SUBTYPE_RAW_FLOAT64: return std::is_same<RawType, compv_float64_t>::value;
			case COMPV_SUBTYPE_RAW_USCALAR: return std::is_same<RawType, compv_uscalar_t>::value;
			case COMPV_SUBTYPE_RAW_SCALAR: return std::is_same<RawType, compv_scalar_t>::value;
			default: return false;
			}
		}
		return false;
	}

	COMPV_ERROR_CODE clone(CompVMatPtrPtr clone)const {
		COMPV_CHECK_EXP_RETURN(!clone, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVMatPtr clone_ = *clone;
		if (!clone_) {
			clone_ = new CompVMat();
			COMPV_CHECK_EXP_RETURN(!clone_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
			*clone = clone_;
		}
		COMPV_CHECK_CODE_RETURN(clone_->__alloc_priv(this->rows(), this->cols(), this->alignment(), this->stride(), this->elmtInBytes(), this->type(), this->subType()));
		const uint8_t* src = this->ptr<const uint8_t>();
		uint8_t* dst = clone_->ptr<uint8_t>();
		const size_t rowInBytes = this->rowInBytes();
		const size_t strideInBytes = this->strideInBytes();
		const size_t rows = this->rows();
		for (size_t row = 0; row < rows; ++row) {
			CompVMem::copy(dst, src, rowInBytes);
			dst += strideInBytes;
			src += strideInBytes;
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE copy(CompVMatPtrPtr dst)const {
		COMPV_CHECK_CODE_RETURN(this->clone(dst));
		return COMPV_ERROR_CODE_S_OK;
	}

	// Set all (even paddings) values to zero
	COMPV_ERROR_CODE zero_all() {
		void* ptr_ = this->ptr<void>();
		if (ptr_ && this->rows() && this->cols()) {
			CompVMem::zero(m_pDataPtr, m_nDataSize);
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE zero_row(size_t row, int planeId = -1) {
		void* ptr_ = this->ptr<void>(row, 0, planeId);
		COMPV_CHECK_EXP_RETURN(!ptr_ || row >= this->rows(planeId), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_CODE_RETURN(CompVMem::zero(ptr_, this->rowInBytes(planeId)));
		return COMPV_ERROR_CODE_S_OK;
	}

	template<typename elmType>
	COMPV_ERROR_CODE one_row(size_t row, int planeId = -1) {
		COMPV_CHECK_EXP_RETURN(isEmpty() || row >= this->rows(planeId) || sizeof(elmType) != m_nElmtInBytes, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		elmType* ptr_ = this->ptr<elmType>(row, 0, planeId);
		const size_t cols = this->cols(planeId);
		for (size_t col = 0; col < cols; ++col) {
			ptr_[col] = static_cast<elmType>(1);
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE zero_rows(int planeId = -1) {
		const int planeIdStart = planeId < 0 ? 0 : planeId;
		const int planeIdEnd = planeId < 0 ? m_nPlaneCount : (planeIdStart + 1);
		for (int pId = planeIdStart; pId < planeIdEnd; ++pId) {
			size_t row_, rows_ = this->rows(pId), rowInBytes_ = this->rowInBytes(pId);
			for (row_ = 0; row_ < rows_; ++row_) {
				CompVMem::zero(this->ptr<void>(row_, 0, pId), rowInBytes_);
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// returned object doesn't have ownership on the internal memory and depends on its creator
	template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_SUBTYPE dataSubType = COMPV_SUBTYPE_RAW_OPAQUE>
	COMPV_ERROR_CODE shrink(CompVMatPtrPtr mat, size_t newRows, size_t newCols) const {
		COMPV_CHECK_EXP_RETURN(!mat || newCols > m_nCols || newRows > m_nRows || sizeof(elmType) != m_nElmtInBytes || m_eType != dataType || m_eSubType != dataSubType, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj<elmType>(mat, 0, 0, m_nAlignV));
		(*mat)->m_nCols = newCols;
		(*mat)->m_nRows = newRows;
		(*mat)->m_nElmtInBytes = m_nElmtInBytes;
		(*mat)->m_nStrideInBytes = m_nStrideInBytes;
		(*mat)->m_nStrideInElts = m_nStrideInElts;
		(*mat)->m_nAlignV = m_nAlignV;
		(*mat)->m_bOweMem = false;
		(*mat)->m_pDataPtr = m_pDataPtr;
		(*mat)->m_nPlaneCount = m_nPlaneCount;
		for (int p = 0; p < m_nPlaneCount; ++p) {
			(*mat)->m_nPlaneCols[p] = m_nPlaneCols[p];
			(*mat)->m_nPlaneRows[p] = m_nPlaneRows[p];
			(*mat)->m_pCompPtr[p] = m_pCompPtr[p];
			(*mat)->m_nPlaneSizeInBytes[p] = m_nPlaneSizeInBytes[p];
			(*mat)->m_nPlaneStrideInBytes[p] = m_nPlaneStrideInBytes[p];
			(*mat)->m_nPlaneStrideInElts[p] = m_nPlaneStrideInElts[p];
			(*mat)->m_bPlaneStrideInEltsIsIntegral[p] = m_bPlaneStrideInEltsIsIntegral[p];
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// returned object doesn't have ownership on the internal memory and depends on its creator
	COMPV_ERROR_CODE bind(CompVMatPtrPtr mat, const CompVRectFloat32& roi) const {
		COMPV_CHECK_EXP_RETURN(!mat || roi.left < 0.f || roi.right < 0.f || roi.top < 0.f || roi.bottom < 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const size_t colStart = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.left, size_t);
		COMPV_CHECK_EXP_RETURN(colStart > m_nCols, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
		size_t colEnd = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.right, size_t);
		colEnd = std::min(colEnd, (m_nCols - 1)); // roi is up right which means 'roi.right' have to be included
		COMPV_CHECK_EXP_RETURN(colStart >= colEnd, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
		const size_t colCount = (colEnd - colStart) + 1; // +1 because it's from 'colStart' up to 'colEnd'

		const size_t rowStart = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.top, size_t);
		COMPV_CHECK_EXP_RETURN(rowStart > m_nRows, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
		size_t rowEnd = COMPV_MATH_ROUNDFU_2_NEAREST_INT(roi.bottom, size_t);
		rowEnd = std::min(rowEnd, (m_nRows - 1)); // roi is up bottom which means 'roi.bottom' have to be included
		COMPV_CHECK_EXP_RETURN(rowStart >= rowEnd, COMPV_ERROR_CODE_E_OUT_OF_BOUND);
		const size_t rowCount = (rowEnd - rowStart) + 1; // +1 because it's from 'rowStart' up to 'rowEnd'

		CompVMatPtr mat_;
		mat_ = new CompVMat();
		COMPV_CHECK_EXP_RETURN(!mat_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		COMPV_CHECK_CODE_RETURN(mat_->__alloc_priv(0, 0, m_nAlignV, m_nStrideInElts, m_nElmtInBytes, m_eType, m_eSubType));

		// Set bound size
		mat_->m_nCols = colCount;
		mat_->m_nRows = rowCount;

		// Set stride (same, doesn't change regardless the bound size)
		mat_->m_nStrideInBytes = m_nStrideInBytes;
		mat_->m_nStrideInElts = m_nStrideInElts;

		if (m_eType == COMPV_MAT_TYPE_RAW || m_eType == COMPV_MAT_TYPE_STRUCT) {
			mat_->m_nPlaneCols[0] = colCount;
			mat_->m_nPlaneRows[0] = rowCount;
			mat_->m_pCompPtr[0] = this->ptr<const void>(rowStart, colStart, 0);
			mat_->m_nPlaneSizeInBytes[0] = m_nPlaneStrideInBytes[0] * rowCount;
			mat_->m_nPlaneStrideInBytes[0] = m_nPlaneStrideInBytes[0];
			mat_->m_nPlaneStrideInElts[0] = m_nPlaneStrideInElts[0];
		}
		else if (m_eType == COMPV_MAT_TYPE_PIXELS) {
			const COMPV_SUBTYPE pixelFormat = m_eSubType;
			size_t rowStartInPlane, colStartInPlane;
			size_t rowEndInPlane, colEndInPlane;
			for (int planeId = 0; planeId < m_nPlaneCount; ++planeId) {
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(pixelFormat, planeId, colStart, rowStart, &colStartInPlane, &rowStartInPlane));
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(pixelFormat, planeId, colEnd, rowEnd, &colEndInPlane, &rowEndInPlane));
				mat_->m_nPlaneCols[planeId] = (colEndInPlane - colStartInPlane) + 1; // +1 because it's from 'colStart' up to 'colEnd'
				mat_->m_nPlaneRows[planeId] = (rowEndInPlane - rowStartInPlane) + 1; // +1 because it's from 'rowStart' up to 'rowEnd'
				mat_->m_pCompPtr[planeId] = this->ptr<const void>(rowStartInPlane, colStartInPlane, planeId);
				mat_->m_nPlaneSizeInBytes[planeId] = m_nPlaneStrideInBytes[planeId] * (mat_->m_nPlaneRows[planeId]);
				mat_->m_nPlaneStrideInBytes[planeId] = m_nPlaneStrideInBytes[planeId];
				mat_->m_nPlaneStrideInElts[planeId] = m_nPlaneStrideInElts[planeId];
			}
		}

		//!\\ We don't own the memory
		mat_->m_bOweMem = false;
		mat_->m_pDataPtr = const_cast<void*>(mat_->m_pCompPtr[0]);

		*mat = mat_;

		return COMPV_ERROR_CODE_S_OK;
	}

    template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_SUBTYPE dataSubType = COMPV_SUBTYPE_RAW_OPAQUE>
    static COMPV_ERROR_CODE newObj(CompVMatPtrPtr mat, size_t rows, size_t cols, size_t alignv, size_t stride = 0) {
        COMPV_CHECK_EXP_RETURN(!mat || !alignv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        CompVMatPtr mat_ = *mat;
        if (!mat_) {
            mat_ = new CompVMat();
            COMPV_CHECK_EXP_RETURN(!mat_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
            *mat = mat_;
        }
        COMPV_CHECK_CODE_RETURN((mat_->alloc<elmType, dataType, dataSubType>(rows, cols, alignv, stride)));
        return COMPV_ERROR_CODE_S_OK;
    }

    template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_SUBTYPE dataSubType = COMPV_SUBTYPE_RAW_OPAQUE>
    static COMPV_ERROR_CODE newObjStrideless(CompVMatPtrPtr mat, size_t rows, size_t cols) {
        return CompVMat::newObj<elmType, dataType, dataSubType>(mat, rows, cols, 1);
    }

	template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_SUBTYPE dataSubType = COMPV_SUBTYPE_RAW_OPAQUE>
	static COMPV_ERROR_CODE newObjAligned(CompVMatPtrPtr mat, size_t rows, size_t cols, size_t stride = 0) {
		return CompVMat::newObj<elmType, dataType, dataSubType>(mat, rows, cols, CompVMem::bestAlignment(), stride);
	}

protected:
    template<class elmType = uint8_t, COMPV_MAT_TYPE dataType = COMPV_MAT_TYPE_RAW, COMPV_SUBTYPE dataSubType_ = COMPV_SUBTYPE_RAW_OPAQUE>
    COMPV_ERROR_CODE alloc(size_t rows, size_t cols, size_t alignv = 1, size_t stride = 0) {
		COMPV_SUBTYPE dataSubType = dataSubType_;
        // Patch SubType
		// Opaque means user-undefined (weak-type), patch and set a strong-type
		if (dataType == COMPV_MAT_TYPE_RAW && dataSubType == COMPV_SUBTYPE_RAW_OPAQUE) {
			if (std::is_same<elmType, int8_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_INT8;
			}
			else if (std::is_same<elmType, uint8_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_UINT8;
			}
			else if (std::is_same<elmType, int16_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_INT16;
			}
			else if (std::is_same<elmType, uint16_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_UINT16;
			}
			else if (std::is_same<elmType, int32_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_INT32;
			}
			else if (std::is_same<elmType, uint32_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_UINT32;
			}
			else if (std::is_same<elmType, size_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_SIZE;
			}
			else if (std::is_same<elmType, compv_float32_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_FLOAT32;
			}
			else if (std::is_same<elmType, compv_float64_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_FLOAT64;
			}
			else if (std::is_same<elmType, compv_uscalar_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_USCALAR;
			}
			else if (std::is_same<elmType, compv_scalar_t>::value) {
				dataSubType = COMPV_SUBTYPE_RAW_SCALAR;
			}
			else {
				COMPV_DEBUG_ERROR_EX("CompVMat", "Cannot patch unknown type");
				return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
			}
		}
		return __alloc_priv(rows, cols, alignv, stride, sizeof(elmType), dataType, dataSubType);
    }

private:
		COMPV_ERROR_CODE __alloc_priv(size_t rows, size_t cols, size_t alignv, size_t stride, size_t elmtInBytes, COMPV_MAT_TYPE dataType, COMPV_SUBTYPE dataSubType) {
			COMPV_CHECK_EXP_RETURN(!alignv || (stride && stride < cols), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

			size_t nNewDataSize = 0;
			bool bPlanePacked = true;
			size_t nElmtInBytes = elmtInBytes;
			size_t nPlaneCount = 0;
			size_t nBestStrideInBytes = 0;
			size_t nPlaneStrideInBytes[COMPV_PLANE_MAX_COUNT];
			size_t nPlaneCols[COMPV_PLANE_MAX_COUNT];
			size_t nPlaneRows[COMPV_PLANE_MAX_COUNT];
			size_t nPlaneSizeInBytes[COMPV_PLANE_MAX_COUNT];
			size_t nPlaneElmtInBytes[COMPV_PLANE_MAX_COUNT];
			void* pMem = NULL;
			
			if (stride >= cols) {
				// stride is already valid
				nBestStrideInBytes = stride * elmtInBytes;
			}
			else {
				// stride not valid or not provided
				nBestStrideInBytes = (dataType == COMPV_MAT_TYPE_PIXELS)
					? static_cast<size_t>(CompVMem::alignForward(((cols + 1) >> 1), static_cast<int>(alignv))) * (elmtInBytes << 1) // make sure both stride and strideInBytes are aligned, also that (stride/2) is aligned
					: static_cast<size_t>(CompVMem::alignForward((cols * elmtInBytes), static_cast<int>(alignv))); // make sure strideInBytes is aligned
			}
#if 0 // User-defined stride doesn't match -> let him die if he makes mistake :)
			COMPV_CHECK_EXP_BAIL(!COMPV_IS_ALIGNED(nBestStrideInBytes, alignv), (err_ = COMPV_ERROR_CODE_E_MEMORY_NOT_ALIGNED), "Stride not aligned with request alignment value");
#endif

			if (dataType == COMPV_MAT_TYPE_RAW || dataType == COMPV_MAT_TYPE_STRUCT) {
				nPlaneCount = 1;
				bPlanePacked = false;
				nNewDataSize = nBestStrideInBytes * rows;
				nPlaneCols[0] = cols;
				nPlaneRows[0] = rows;
				nPlaneStrideInBytes[0] = nBestStrideInBytes;
				nPlaneSizeInBytes[0] = nNewDataSize;
				nPlaneElmtInBytes[0] = nElmtInBytes;
			}
			else if (dataType == COMPV_MAT_TYPE_PIXELS) {
				size_t elmtInBits;
				size_t nPlaneStrideInSamples[COMPV_PLANE_MAX_COUNT];
				const size_t nBestStrideInSamples = nBestStrideInBytes; // For images re-compute 'nBestStrideInBytes' if packed (e.g. RGB)
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeCount(dataSubType, &nPlaneCount)); // get number of comps
				COMPV_CHECK_EXP_RETURN(!nPlaneCount || nPlaneCount > COMPV_PLANE_MAX_COUNT, COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT); // check the number of comps
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::isPlanePacked(dataSubType, &bPlanePacked)); // check whether the comps are packed
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(dataSubType, 0, nBestStrideInSamples, 1, &nBestStrideInBytes)); // get stride in bytes (nBestStrideInBytes)
				COMPV_CHECK_CODE_RETURN(CompVImageUtils::sizeForPixelFormat(dataSubType, nBestStrideInSamples, rows, &nNewDataSize)); // get overal size in bytes
				size_t nSumPlaneSizeInBytes = 0;
				for (size_t planeId = 0; planeId < nPlaneCount; ++planeId) {
					COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(dataSubType, planeId, cols, rows, &nPlaneCols[planeId], &nPlaneRows[planeId])); // get 'cols' and 'rows' for the comp
					COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(dataSubType, planeId, nBestStrideInSamples, rows, &nPlaneStrideInSamples[planeId], &nPlaneRows[planeId])); // get 'stride' and 'rows' for the comp
					COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeSizeForPixelFormat(dataSubType, planeId, nBestStrideInSamples, rows, &nPlaneSizeInBytes[planeId])); // get comp size in bytes
					COMPV_CHECK_CODE_RETURN(CompVImageUtils::planeBitsCountForPixelFormat(dataSubType, planeId, &elmtInBits));
					nPlaneElmtInBytes[planeId] = (elmtInBits >> 3);
					nPlaneStrideInBytes[planeId] = nPlaneStrideInSamples[planeId] * nPlaneElmtInBytes[planeId];
					nSumPlaneSizeInBytes += nPlaneSizeInBytes[planeId];
				}
				// Make sure that sum(comp sizes) = data size
				COMPV_CHECK_EXP_BAIL(nSumPlaneSizeInBytes != nNewDataSize, err_ = COMPV_ERROR_CODE_E_INVALID_PIXEL_FORMAT);
			}
			else {
				COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED, "Invalid data type");
			}

			if (nNewDataSize > m_nDataCapacity) {
				pMem = CompVMem::mallocAligned(nNewDataSize, static_cast<int>(alignv));
				COMPV_CHECK_EXP_BAIL(!pMem, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY), "Failed to allocate memory");
				m_nDataCapacity = nNewDataSize;
				// realloc()
				if (m_bOweMem) {
					CompVMem::freeAligned((void**)&m_pDataPtr);
				}
				m_pDataPtr = pMem;
				m_bOweMem = true;
			}

			//!\\ Do not update capacity unless shorter (see above)

			m_eType = dataType;
			m_eSubType = dataSubType;
			m_nCols = cols;
			m_nRows = rows;
			m_nStrideInBytes = nBestStrideInBytes;
			m_nStrideInElts = (nBestStrideInBytes / nPlaneElmtInBytes[0]);
			m_nDataSize = nNewDataSize;
			m_nAlignV = alignv;
			m_nElmtInBytes = nPlaneElmtInBytes[0];
			m_bPlanePacked = bPlanePacked;
			m_nPlaneCount = static_cast<int>(nPlaneCount);
			m_pCompPtr[0] = m_pDataPtr;
			m_nPlaneSizeInBytes[0] = nPlaneSizeInBytes[0];
			m_nPlaneCols[0] = nPlaneCols[0];
			m_nPlaneRows[0] = nPlaneRows[0];
			m_nPlaneElmtInBytes[0] = nPlaneElmtInBytes[0];
			m_nPlaneStrideInBytes[0] = nPlaneStrideInBytes[0];
			m_nPlaneStrideInElts[0] = (nPlaneStrideInBytes[0] / nPlaneElmtInBytes[0]);
			m_bPlaneStrideInEltsIsIntegral[0] = !(nPlaneStrideInBytes[0] % nPlaneElmtInBytes[0]);
			for (size_t planeId = 1; planeId < nPlaneCount; ++planeId) {
				m_pCompPtr[planeId] = static_cast<const uint8_t*>(m_pCompPtr[planeId - 1]) + nPlaneSizeInBytes[planeId - 1];
				m_nPlaneSizeInBytes[planeId] = nPlaneSizeInBytes[planeId];
				m_nPlaneCols[planeId] = nPlaneCols[planeId];
				m_nPlaneRows[planeId] = nPlaneRows[planeId];
				m_nPlaneElmtInBytes[planeId] = nPlaneElmtInBytes[planeId];
				m_nPlaneStrideInBytes[planeId] = nPlaneStrideInBytes[planeId];
				m_nPlaneStrideInElts[planeId] = (nPlaneStrideInBytes[planeId] / nPlaneElmtInBytes[planeId]);
				m_bPlaneStrideInEltsIsIntegral[planeId] = !(nPlaneStrideInBytes[planeId] % nPlaneElmtInBytes[planeId]);
			}

		bail:
			if (COMPV_ERROR_CODE_IS_NOK(err_)) {
				CompVMem::free(reinterpret_cast<void**>(&pMem));
			}
			return err_;
		}

private:
    void* m_pDataPtr; // must be freed using 'freeAligned'
    int m_nPlaneCount;
    bool m_bPlanePacked;
    size_t m_nCols;
    size_t m_nRows;
    size_t m_nStrideInBytes;
    size_t m_nStrideInElts;
    size_t m_nPlaneCols[COMPV_PLANE_MAX_COUNT];
    size_t m_nPlaneRows[COMPV_PLANE_MAX_COUNT];
    const void* m_pCompPtr[COMPV_PLANE_MAX_COUNT];
    size_t m_nPlaneSizeInBytes[COMPV_PLANE_MAX_COUNT];
    size_t m_nPlaneStrideInBytes[COMPV_PLANE_MAX_COUNT];
    size_t m_nPlaneStrideInElts[COMPV_PLANE_MAX_COUNT];
	size_t m_nPlaneElmtInBytes[COMPV_PLANE_MAX_COUNT];
    bool m_bPlaneStrideInEltsIsIntegral[COMPV_PLANE_MAX_COUNT];
    size_t m_nElmtInBytes;
    size_t m_nAlignV;
    size_t m_nDataSize;
    size_t m_nDataCapacity;
    bool m_bOweMem;
    COMPV_MAT_TYPE m_eType;
    COMPV_SUBTYPE m_eSubType;
};

typedef std::vector<CompVMatPtr> CompVMatPtrVector;

COMPV_GCC_DISABLE_WARNINGS_END()
COMPV_GCC_DISABLE_WARNINGS_END()

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MAT_H_ */
