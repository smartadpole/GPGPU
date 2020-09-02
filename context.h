#ifndef OPENGL_EXAMPLES_FBO_CONTEXT_H_
#define OPENGL_EXAMPLES_FBO_CONTEXT_H_
#include <glog/logging.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>

namespace opengl{
    namespace example{

        void OpenGLCheckErrorWithLocation(int line);
        const char *GLGetErrorString(GLenum error);

        void InitContext();
        void DestroyContext();
    }// namespace exampel
}// namespace opengl


/*!
 * \brief Protected OpenGL call.
 * \param func Expression to call.
 */
#define OPENGL_CALL(func)                                                      \
{                                                                            \
    (func);                                                                    \
    ::opengl::example::OpenGLCheckErrorWithLocation(__LINE__);                                                      \
}

#define OPENGL_CHECK_ERROR            \
    ::opengl::example::OpenGLCheckErrorWithLocation(__LINE__)
#endif
