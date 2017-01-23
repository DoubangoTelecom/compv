/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/drawing/compv_mvp.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVMat4f
//

CompVMat4f::CompVMat4f()
{

}

CompVMat4f::~CompVMat4f()
{

}

//
//	CompVModel
//

CompVModel::CompVModel()
{

}

CompVModel::~CompVModel()
{

}

//
//	CompVView
//

CompVView::CompVView()
{

}

CompVView::~CompVView()
{

}

//
//	CompVProj
//
CompVProj::CompVProj(COMPV_PROJECTION eType)
    : m_eType(eType)
{

}

CompVProj::~CompVProj()
{
}

//
//	CompVProj2D
//

CompVProj2D::CompVProj2D()
    : CompVProj(COMPV_PROJECTION_2D)
{

}

CompVProj2D::~CompVProj2D()
{

}


//
//	CompVProj3D
//

CompVProj3D::CompVProj3D()
    : CompVProj(COMPV_PROJECTION_3D)
{

}

CompVProj3D::~CompVProj3D()
{

}


//
//	CompVMVP
//

CompVMVP::CompVMVP()
{

}

CompVMVP::~CompVMVP()
{

}

COMPV_NAMESPACE_END()

