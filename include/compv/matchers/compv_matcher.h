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
#if !defined(_COMPV_MATCHERS_MATCHER_H_)
#define _COMPV_MATCHERS_MATCHER_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"
#include "compv/compv_settable.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

class CompVMatcher;

struct CompVMatcherFactory {
    int id;
    const char* name;
    COMPV_ERROR_CODE(*newObj)(CompVPtr<CompVMatcher* >* matcher);
};

/* Matchers setters and getters */
enum {
    /* Common to all matchers */
    // COMPV_MATCHER_SET_XXX_YYY,

    /* Brute force */
    COMPV_BRUTEFORCE_ID,
    COMPV_BRUTEFORCE_SET_INT32_KNN,
    COMPV_BRUTEFORCE_SET_INT32_NORM,
    COMPV_BRUTEFORCE_SET_BOOL_CROSS_CHECK,
    COMPV_BRUTEFORCE_NORM_HAMMING,
    COMPV_BRUTEFORCE_NORM_HAMMING2,
    COMPV_BRUTEFORCE_NORM_L2,

    /*  FLANN */
    COMPV_FLANN_ID,
};

// Class: CompVMatcher
class COMPV_API CompVMatcher : public CompVObj, public CompVSettable
{
protected:
    CompVMatcher();
public:
    virtual ~CompVMatcher();
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE addFactory(const CompVMatcherFactory* factory);
    static const CompVMatcherFactory* findFactory(int matcherId);
    static COMPV_ERROR_CODE newObj(int matcherId, CompVPtr<CompVMatcher* >* matcher);
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVArray<uint8_t>* >&queryDescriptions, const CompVPtr<CompVArray<uint8_t>* >&trainDescriptions, CompVPtr<CompVArray<CompVDMatch>* >* matches) = 0;

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    static std::map<int, const CompVMatcherFactory*> s_Factories;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATCHERS_MATCHER_H_ */
