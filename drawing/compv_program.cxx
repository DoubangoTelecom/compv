/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_program.h"
#include "compv/drawing/compv_drawing.h"
#include "compv/drawing/opengl/compv_program_gl.h"

COMPV_NAMESPACE_BEGIN()

CompVProgram::CompVProgram()
{

}

CompVProgram::~CompVProgram()
{
	
}

COMPV_ERROR_CODE CompVProgram::newObj(CompVProgramPtrPtr program)
{
	COMPV_CHECK_CODE_RETURN(CompVDrawing::init());
	COMPV_CHECK_EXP_RETURN(!program, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVProgramPtr program_;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	CompVProgramGLPtr glProgram_;
	COMPV_CHECK_CODE_RETURN(CompVProgramGL::newObj(&glProgram_));
	program_ = *glProgram_;
#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

	COMPV_CHECK_EXP_RETURN(!(*program = program_), COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

