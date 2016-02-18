/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
* Copyright (C) 2016 Mamadou DIOP.
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
#include "compv/compv_interestpoint.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"
#include "compv/compv_engine.h"

COMPV_NAMESPACE_BEGIN()

#define COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD 200*100

static void sortByStrengthRange(CompVBoxInterestPoint* self, bool(*CompVBoxPredicateCompare)(const CompVInterestPoint*, const CompVInterestPoint*), intptr_t left, intptr_t right);
static COMPV_ERROR_CODE sortByStrengthRangeAsynExec(const struct compv_asynctoken_param_xs* pc_params);

CompVBoxInterestPoint::CompVBoxInterestPoint(size_t nCapacity /*= 0*/, bool bLockable /*= false*/)
	: CompVBox<CompVInterestPoint >(nCapacity, bLockable)
{

}

CompVBoxInterestPoint::~CompVBoxInterestPoint()
{

}

COMPV_ERROR_CODE CompVBoxInterestPoint::sort(bool(*CompVBoxPredicateCompare)(const CompVInterestPoint*, const CompVInterestPoint*))
{
	sortByStrengthRange(this, CompVBoxPredicateCompare, 0, (intptr_t)size() - 1);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBoxInterestPoint::newObj(CompVObjWrapper<CompVBoxInterestPoint* >* box, size_t nCapacity /*= 0*/, bool bLockable /*= false*/)
{
	if (sizeof(CompVBoxInterestPoint) > kCompVBoxItemMaxSize) {
		COMPV_DEBUG_ERROR("Boxing is only allowed on object with size < %u, you're boxing an object with size = ", (unsigned)kCompVBoxItemMaxSize, sizeof(CompVBoxInterestPoint));
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_EXP_RETURN(!box, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVObjWrapper<CompVBoxInterestPoint* > box_;

	box_ = new CompVBoxInterestPoint(nCapacity, bLockable);
	COMPV_CHECK_EXP_RETURN(!box_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(bLockable && !box_->m_Mutex, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_EXP_RETURN(nCapacity && !box_->m_pMem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*box = box_;

	return COMPV_ERROR_CODE_S_OK;
}

static void sortByStrengthRange(CompVBoxInterestPoint* self, bool(*CompVBoxPredicateCompare)(const CompVInterestPoint*, const CompVInterestPoint*), intptr_t left, intptr_t right)
{
	CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();
	int32_t threadsCount = threadDip ? threadDip->getThreadsCount() : 0;
	uint32_t threadIdx0 = UINT_MAX, threadIdx1 = UINT_MAX;
	const CompVInterestPoint pivot = *self->at((left + right) >> 1);
	CompVInterestPoint atk, *ati = self->at(left), *atj = self->at(right);
	const CompVInterestPoint *ati_ = ati, *atj_ = atj;
	while (ati <= atj) {
		while (CompVBoxPredicateCompare(ati, &pivot)) ++ati;
		while (CompVBoxPredicateCompare(&pivot, atj)) --atj;
		if (ati > atj) break;
		atk = *ati;
		*ati = *atj;
		*atj = atk;
		++ati;
		--atj;
	}
	intptr_t i = left + (ati - ati_);
	intptr_t j = right + (atj - atj_);
	if (left < j) {
		if (threadsCount > 2 && (j - left) > COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD && !threadDip->isMotherOfTheCurrentThread()) {
			threadIdx0 = threadDip->getThreadIdxForNextToCurrentCore();
			COMPV_CHECK_CODE_ASSERT(threadDip->execute(threadIdx0, COMPV_TOKENIDX0, sortByStrengthRangeAsynExec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(self, CompVBoxPredicateCompare, left, j),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
		}
		else {
			sortByStrengthRange(self, CompVBoxPredicateCompare, left, j);
		}
	}
	if (i < right) {
		if (threadsCount > 2 && (right - i) > COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD && !threadDip->isMotherOfTheCurrentThread()) {
			threadIdx1 = threadDip->getThreadIdxForNextToCurrentCore() + 1;
			COMPV_CHECK_CODE_ASSERT(threadDip->execute(threadIdx1, COMPV_TOKENIDX1, sortByStrengthRangeAsynExec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(self, CompVBoxPredicateCompare, i, right),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
		}
		else {
			sortByStrengthRange(self, CompVBoxPredicateCompare, i, right);
		}
	}
	if (threadIdx0 != UINT_MAX) {
		COMPV_CHECK_CODE_ASSERT(threadDip->wait(threadIdx0, COMPV_TOKENIDX0));
	}
	if (threadIdx1 != UINT_MAX) {
		COMPV_CHECK_CODE_ASSERT(threadDip->wait(threadIdx1, COMPV_TOKENIDX1));
	}
}

static COMPV_ERROR_CODE sortByStrengthRangeAsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
	CompVBoxInterestPoint* self = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVBoxInterestPoint*);
	bool(*CompVBoxPredicateCompare)(const CompVInterestPoint*, const CompVInterestPoint*) = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, bool(*)(const CompVInterestPoint*, const CompVInterestPoint*));
	intptr_t left = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, intptr_t);
	intptr_t right = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, intptr_t);
	sortByStrengthRange(self, CompVBoxPredicateCompare, left, right);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
