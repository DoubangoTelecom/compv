/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GL_INFO_H_)
#define _COMPV_GL_INFO_H_

#include "compv/gl/compv_gl_config.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_common.h"
#include "compv/gl/compv_gl_utils.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class COMPV_GL_API CompVGLInfo
{
public:
    struct COMPV_GL_API extensions {
        friend class CompVGLInfo;
    public:
        static bool vertex_array_object() {
            return s_bvertex_array_object;
        }
        static bool texture_float() {
            return s_btexture_float;
        }
    private:
        static bool s_bvertex_array_object;
        static bool s_btexture_float;
    };
    static COMPV_ERROR_CODE gather();
	static GLint versionMajor() {
		return s_iVersionMajor;
	}
	static GLint versionMinor() {
		return s_iVersionMinor;
	}
private:
    static bool s_bGathered;
    static GLint s_iVersionMajor;
    static GLint s_iVersionMinor;
    static GLint s_iMaxColorAttachments;
    static GLint s_iMaxDrawBuffers;
};

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */

#endif /* _COMPV_GL_INFO_H_ */
