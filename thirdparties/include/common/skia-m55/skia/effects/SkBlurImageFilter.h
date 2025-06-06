/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurImageFilter_DEFINED
#define SkBlurImageFilter_DEFINED

#include "SkImageFilter.h"

class SK_API SkBlurImageFilter
{
public:
    static sk_sp<SkImageFilter> Make(SkScalar sigmaX, SkScalar sigmaY,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr) {
        return SkImageFilter::MakeBlur(sigmaX, sigmaY, input, cropRect);
    }

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(SkScalar sigmaX, SkScalar sigmaY,
                                 SkImageFilter * input = nullptr,
                                 const SkImageFilter::CropRect* cropRect = nullptr) {
        return SkImageFilter::MakeBlur(sigmaX, sigmaY, sk_ref_sp<SkImageFilter>(input),
                                       cropRect).release();
    }
#endif
};

#endif
