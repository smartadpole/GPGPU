#include "context.h"

#include <EGL/egl.h>

namespace opengl{
    namespace {
        // egl
        EGLContext eglContext;
        EGLDisplay eglDisplay;
        EGLSurface eglSurface;
    }//namespace

    namespace example{
        void InitContext(){
            if(eglGetCurrentContext() == EGL_NO_CONTEXT)
            {
                eglContext = EGL_NO_CONTEXT;
                VLOG(1) << "No Current Context Found! Need to Create Again";
            }
            
            eglDisplay  = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            LOG_IF(FATAL, eglDisplay == EGL_NO_DISPLAY)<<"eglGetDisplay Failed When Creating Context!";
            eglInitialize(eglDisplay, nullptr, nullptr);

            EGLint numConfigs;
            EGLConfig eglConfig;

            const EGLint configAttribs[] = {
                EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_DEPTH_SIZE, 8,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, 
                EGL_NONE
            };

            bool choose = eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigs);

            if(!choose){
                eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglTerminate(eglDisplay);
                eglDisplay = EGL_NO_DISPLAY;
                LOG(FATAL)<<"eglChooseConfig Failed When Creating Context!";
            }

            const EGLint pbufferAttribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE,};
            const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

            eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbufferAttribs);
            eglContext = eglCreateContext(eglDisplay, eglConfig,  EGL_NO_CONTEXT, contextAttribs);
            eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
            eglBindAPI(EGL_OPENGL_ES_API);

            LOG(INFO) << "Opengl says version: " << glGetString(GL_VERSION);

            OPENGL_CHECK_ERROR;
        }

        void DestroyContext(){
            // first makecurrent
            if (eglDisplay != EGL_NO_DISPLAY) 
            {
                if (eglContext != EGL_NO_CONTEXT) eglDestroyContext(eglDisplay, eglContext);
                if (eglSurface != EGL_NO_SURFACE) eglDestroySurface(eglDisplay, eglSurface);
                eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglTerminate(eglDisplay);

                eglDisplay = EGL_NO_DISPLAY;
                eglSurface = EGL_NO_SURFACE;
                eglContext = EGL_NO_CONTEXT;
            }

            eglReleaseThread();
        }
    
    }//namespace example
}//namespace opengl




