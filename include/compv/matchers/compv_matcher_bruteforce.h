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
#if !defined(_COMPV_MATCHERS_MATCHER_BRUTEFORCE_H_)
#define _COMPV_MATCHERS_MATCHER_BRUTEFORCE_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"
#include "compv/matchers/compv_matcher.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

// Class: CompVMatcher
class COMPV_API CompVMatcherBruteForce : public CompVMatcher
{
protected:
	CompVMatcherBruteForce();
public:
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVMatcherBruteForce";
	};
	virtual ~CompVMatcherBruteForce();
	// override CompVSettable::set
	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
	// override CompVSettable::get
	virtual COMPV_ERROR_CODE get(int id, const void*& valuePtr, size_t valueSize);
	// override CompVMatcher::process
	virtual COMPV_ERROR_CODE process(const CompVPtr<CompVArray<uint8_t>* >&queryDescriptions, const CompVPtr<CompVArray<uint8_t>* >&trainDescriptions, CompVPtr<CompVArray<CompVDMatch>* >* matches);
	
	static COMPV_ERROR_CODE newObj(CompVPtr<CompVMatcher* >* matcher);

private:
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	bool m_bCrossCheck;
	int32_t m_nNormType;
	int32_t m_nKNN;
	CompVPtr<CompVArray<compv_scalar_t>* > m_hammingDistances;
	COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATCHERS_MATCHER_BRUTEFORCE_H_ */
