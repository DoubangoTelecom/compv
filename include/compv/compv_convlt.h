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
#if !defined(_COMPV_CONVLT_H_)
#define _COMPV_CONVLT_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVConvlt : public CompVObj
{
protected:
    CompVConvlt();
public:
    virtual ~CompVConvlt();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVConvlt";
    };
    virtual COMPV_INLINE const void* getResultPtr() const {
		return m_pResultPtr;
    }
    virtual COMPV_INLINE size_t getResultSize() const {
		return m_nResultSize;
    }
	COMPV_ERROR_CODE convlt2(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const double* kern_ptr, int kern_size, uint8_t* out_ptr = NULL, int img_border = 0);
	COMPV_ERROR_CODE convlt1(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const T* vkern_ptr, const T* hkern_ptr, int kern_size, uint8_t* out_ptr = NULL, int img_border = 0);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVConvlt* >* convlt);

private:
	static void convlt1_verthz(const uint8_t* in_ptr, uint8_t* out_ptr, int width, int height, int stride, int pad, const T* vhkern_ptr, int kern_size);

private:
    void* m_pDataPtr;
    void* m_pDataPtr0;
	const void* m_pResultPtr;
    size_t m_nDataSize;
    size_t m_nDataSize0;
	size_t m_nResultSize;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CONVLT_H_ */
