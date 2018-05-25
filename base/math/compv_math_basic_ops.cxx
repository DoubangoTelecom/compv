/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

#include "compv/base/math/compv_math_basic_ops.h"
#include "compv/base/compv_cpu.h"

COMPV_NAMESPACE_BEGIN()

template<typename T>
class CompVMathBasicUnsignedOpSubs { 
public:
	COMPV_ALWAYS_INLINE T operator()(const T& a, const T& b) const {
		const T r = (a - b);
		return (r > a) ? 0 : r;
	}
};
typedef CompVMathBasicUnsignedOpSubs<uint8_t> CompVMathBasicUnsignedOpSubsUInt8;
typedef CompVMathBasicUnsignedOpSubs<uint16_t> CompVMathBasicUnsignedOpSubsUInt16;

template<typename T>
class CompVMathBasicUnsignedOpAdds {
public:
	COMPV_ALWAYS_INLINE T operator()(const T& a, const T& b) const {
		static const T maxx = std::numeric_limits<T>::max();
		const T r = (a + b);
		return (r < a) ? maxx : r;
	}
};
typedef CompVMathBasicUnsignedOpAdds<uint8_t> CompVMathBasicUnsignedOpAddsUInt8;
typedef CompVMathBasicUnsignedOpAdds<uint16_t> CompVMathBasicUnsignedOpAddsUInt16;

template<typename T, class Op>
static void CompVMathBasicOp_C(const T* aPtr, const T* bPtr, T* rPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t astride, const compv_uscalar_t bstride, const compv_uscalar_t rstride)
{
	Op op;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			rPtr[i] = op(aPtr[i], bPtr[i]);
		}
		rPtr += rstride;
		aPtr += astride;
		bPtr += bstride;
	}
}

class CompVMathBasicOpsGeneric {
public:
	template<typename T, class Op>
	static COMPV_ERROR_CODE processABR(const CompVMatPtr& a, const CompVMatPtr& b, CompVMatPtrPtr r)
	{
		const size_t width = a->cols();
		const size_t height = a->rows();
		const size_t astride = a->stride();
		const size_t bstride = b->stride();
		CompVMatPtr r_ = *r;
		if (r_ != a && r_ != b) { // (a == b) or (a == b) allowed
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<T>(&r_, height, width, astride));
		}
		const T* aPtr = a->ptr<const T>();
		const T* bPtr = b->ptr<const T>();
		T* rPtr = r_->ptr<T>();
		const size_t rstride = r_->stride();

		/* Hook to processing function */
		typedef void(*processABRFunPtr)(const T* aPtr, const T* bPtr, T* rPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t astride, const compv_uscalar_t bstride, const compv_uscalar_t rstride);
		processABRFunPtr processABR = [](const T* aPtr, const T* bPtr, T* rPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t astride, const compv_uscalar_t bstride, const compv_uscalar_t rstride) {
			CompVMathBasicOp_C<T, Op >(aPtr, bPtr, rPtr, width, height, astride, bstride, rstride);
		};

		// Uint8
		if (std::is_same<T, uint8_t>::value && std::is_same<CompVMathBasicUnsignedOpSubs<uint8_t>, Op>::value) {
			void(*processABR_8u)(const uint8_t* aPtr, const uint8_t* bPtr, uint8_t* rPtr, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t astride, const compv_uscalar_t bstride, const compv_uscalar_t rstride)
				= nullptr;
#if COMPV_ARCH_X86
			if (CompVCpu::isEnabled(kCpuFlagSSE2) && a->isAlignedSSE() && b->isAlignedSSE() && r_->isAlignedSSE()) {
				COMPV_EXEC_IFDEF_INTRIN_X86(processABR_8u = nullptr);
				COMPV_EXEC_IFDEF_ASM_X64(processABR_8u = nullptr);
			}
			if (CompVCpu::isEnabled(kCpuFlagAVX2) && a->isAlignedAVX() && b->isAlignedAVX() && r_->isAlignedAVX()) {
				COMPV_EXEC_IFDEF_INTRIN_X86(processABR_8u = nullptr);
				COMPV_EXEC_IFDEF_ASM_X64(processABR_8u = nullptr);
			}
#elif COMPV_ARCH_ARM
			if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && a->isAlignedNEON() && b->isAlignedNEON() && r_->isAlignedNEON()) {
				COMPV_EXEC_IFDEF_INTRIN_ARM(processABR_8u = nullptr);
				COMPV_EXEC_IFDEF_ASM_ARM32(processABR_8u = nullptr);
				COMPV_EXEC_IFDEF_ASM_ARM64(processABR_8u = nullptr);
			}
#endif
			if (processABR_8u) {
				processABR = reinterpret_cast<processABRFunPtr>(processABR_8u);
			}
		}
		
		processABR(aPtr, bPtr, rPtr, width, height, astride, bstride, rstride);

		return COMPV_ERROR_CODE_S_OK;
	}
};

#define CompVMathBasicOpsProcess(op, a, b, r) { \
	COMPV_CHECK_EXP_RETURN( \
	!a || a->isEmpty() || a->planeCount() != 1 || \
	!b || b->cols() != a->cols() || b->rows() != a->rows() || b->planeCount() != a->planeCount() || a->subType() != b->subType() || \
	!r, \
	COMPV_ERROR_CODE_E_INVALID_PARAMETER); \
	 \
	switch (a->subType()) { \
	case COMPV_SUBTYPE_RAW_UINT8: \
		COMPV_CHECK_CODE_RETURN((CompVMathBasicOpsGeneric::processABR<uint8_t, CompVMathBasicUnsignedOp##op<uint8_t> >(a, b, r))); \
		break; \
	case COMPV_SUBTYPE_RAW_UINT16: \
		COMPV_CHECK_CODE_RETURN((CompVMathBasicOpsGeneric::processABR<uint16_t, CompVMathBasicUnsignedOp##op<uint16_t> >(a, b, r))); \
		break; \
	default: \
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED); \
		break; \
	} \
	 \
	return COMPV_ERROR_CODE_S_OK; \
}

COMPV_ERROR_CODE CompVMathBasicOps::subs(const CompVMatPtr& a, const CompVMatPtr& b, CompVMatPtrPtr r)
{
	CompVMathBasicOpsProcess(Subs, a, b, r);
}

COMPV_ERROR_CODE CompVMathBasicOps::adds(const CompVMatPtr& a, const CompVMatPtr& b, CompVMatPtrPtr r)
{
	COMPV_DEBUG_INFO_CODE_NOT_TESTED("Placeholder for future implementation");
	CompVMathBasicOpsProcess(Adds, a, b, r);
}

COMPV_NAMESPACE_END()
