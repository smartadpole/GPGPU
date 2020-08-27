#include <iostream>
#include <iomanip>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <sys/time.h> //test

// #define DISPLAY 
const int H = 2400;
const int W = 3200;

using TYPE = uint32_t;
TYPE *data = new TYPE[H * W];
TYPE *result = new TYPE[H * W];

EGLDisplay eglDisplay;
EGLContext eglContext;
EGLSurface eglSurface;

const GLchar *vs_src =
    "attribute vec4 position;\n"
    "varying vec2 tex_coord;\n"
    "void main() {\n"
    "  gl_Position = position;\n"
    "  tex_coord = 0.5 * vec2(position.x + 1.0, position.y + 1.0);\n"
    "}\n";

const GLchar *fs_src =
    "precision mediump float;\n"
    "uniform sampler2D img;\n"
    "varying vec2 tex_coord;\n"
    "uniform float a;\n"
    "void main() {\n"
    "  vec4 color = texture2D(img, tex_coord);\n"
    "  gl_FragColor = color * vec4(a, 1.0, 1.0, 1.0);\n"
    "}\n";

const GLfloat vertices[] = {
    -1.0, -1.0, 0.0,
    -1.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, -1.0, 0.0
};


class Timer
{
public:
    Timer(){Start();}

public:

    /**
    * @brief get time(second)
    * @param message
    *
    * @return time
    */
    double Timing(const std::string message="")
    {
        gettimeofday(&end, nullptr);
        double timeuse = (1000000*(end.tv_sec-start.tv_sec) + end.tv_usec-start.tv_usec) * 1.0 / 1000;
        if ("" != message)
        {
            std::cout << "\033[32muse time(" << message << "): \033[31m" << timeuse << "\033[32mms\033[30m" << std::endl;
        }
        Start();

        return timeuse;

    }

private:
    void Start(){gettimeofday(&start, nullptr);}

private:
    struct timeval start, end;
};


void prepare_data() {
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            data[i*W+j] = (i + j) % 256;

    #ifdef DISPLAY
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++)
            std::cout << std::setw(4) << data[i*W+j];
        std::cout << std::endl;
    }
    #endif
}

void create_context() {
    eglDisplay  = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDisplay, nullptr, nullptr);

    EGLint numConfigs;
    EGLConfig eglConfig;

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // !
        EGL_NONE
    };

    eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numConfigs);

    const EGLint pbufferAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE,
    };

    eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbufferAttribs);

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, // !
        EGL_NONE
    };

    eglContext = eglCreateContext(eglDisplay, eglConfig,  EGL_NO_CONTEXT, contextAttribs);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
}

void destroy_context() {
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);

    eglDisplay = EGL_NO_DISPLAY;
    eglSurface = EGL_NO_SURFACE;
    eglContext = EGL_NO_CONTEXT;
}

void create_framebuffer() {
    GLuint fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
}

int main() 
{
    Timer timer_all;
    prepare_data();
    create_context();
    create_framebuffer();

    // Get max texture size
    GLint maxtexsize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
    // std::cout << "Max texture size = " << maxtexsize << std::endl;

    // Textures
    GLuint tex[2];
    glGenTextures(2, tex);

    glBindTexture(GL_TEXTURE_2D, tex[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, tex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[1], 0);

    // Shaders
    GLuint program = glCreateProgram();
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, &vs_src, NULL);
    glShaderSource(fs, 1, &fs_src, NULL);

    // Compiling and linking

    GLint shader_compiled;
    GLsizei log_length;
    GLchar message[1024];

    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &shader_compiled);
    if (shader_compiled != GL_TRUE)
    {
        glGetShaderInfoLog(vs, 1024, &log_length, message);
        std::cout << "Error: " << message << std::endl;
    }

    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &shader_compiled);
    if (shader_compiled != GL_TRUE)
    {
        glGetShaderInfoLog(fs, 1024, &log_length, message);
        std::cout << "Error: " << message << std::endl;
    }

    glBindAttribLocation(program, 0, "position");

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    glUseProgram(program);

    // Preparing framebuffer

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, W, H);

    // Passing data to shaders
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex[0]);

    Timer timer_pre;
    GLint img_loc = glGetUniformLocation(program, "img");
    glUniform1i(img_loc, 0);

    GLint a_loc = glGetUniformLocation(program, "a");
    glUniform1f(a_loc, 2.0);
    timer_pre.Timing("upload");

    Timer timer;
    const int count = 10;
    int i = count;
    while (i--)
    {
        // Drawing
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glFinish();
    }
    timer.Timing("compute : in iterator " + std::to_string(count));
    // Get data

    Timer timer_post;
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, result);
    timer_post.Timing("download");

    timer_all.Timing("total");
    #ifdef DISPLAY
    
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            std::cout << std::setw(4) << (TYPE)result[r*W + c]; // %250
        }
        std::cout << std::endl;
    }
    #endif 

    destroy_context();
}
