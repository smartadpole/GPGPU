#include <iostream>
#include <fstream>
#include <iomanip>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <glog/logging.h>
#include <sys/time.h> //test

#include "context.h"

#define DISPLAY 
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

GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;

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


std::string ReadKernel(const std::string file)
{
    std::ifstream fd(file);
    std::string src = std::string(std::istreambuf_iterator<char>(fd),
            (std::istreambuf_iterator<char>()));
    if(src.empty()){
        LOG(FATAL) << "Read File ERROR from " << file;
    }

    return src;
}


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

/*!
 * \brief Create and compile a shader from a source string.
 * \param shader_kind The kind of shader.
 * Could be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 * \param shader_src The source string of the shader.
 * \return The compiled shader ID.
 */
GLuint CreateShader(GLenum shader_kind, const char *shader_src) {
    // Create the shader.
    GLuint shader = glCreateShader(shader_kind);
    glShaderSource(shader, 1, &shader_src, nullptr);
    glCompileShader(shader);

    // Check compile errors.
    GLint err;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &err);

    GLint info_log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
    // LOG(INFO) << "\n\n\nshader src: " << shader_src << " res: " << err << " log len: " << info_log_len;

    if (!err && info_log_len > 0) {
        std::unique_ptr<char[]> err_msg(new char[info_log_len + 1]);
        glGetShaderInfoLog(shader, info_log_len, nullptr, err_msg.get());
        LOG(FATAL) << err_msg.get();
    }

    OPENGL_CHECK_ERROR;

    return shader;
}


/*!
 * \brief Create a program that uses the given vertex and fragment shaders.
 * \param fragment_shader The **compiled** fragment shader.
 * \return The program ID.
 */
void CreateProgram(const std::string file) {
    
    // std::string fragment = ReadKernel(file);

    vertex_shader = CreateShader(GL_VERTEX_SHADER, vs_src);
    fragment_shader = CreateShader(GL_FRAGMENT_SHADER, fs_src);

    // Create the program and link the shaders.
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glUseProgram(program);

    OPENGL_CHECK_ERROR;
    // OPENGL_CALL(glDetachShader(program, vertex_shader));
    // OPENGL_CALL(glDetachShader(program, fragment_shader));
}

GLuint InitFrameBuffer() 
{
    GLuint fb;
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    return fb;
}

void DestoryFrameBuffer(const GLuint& frameBuffer)
{
    glDeleteFramebuffers(1, &frameBuffer);
}

GLuint CreateTexture(const TYPE *data, GLsizei W, GLsizei H)
{
    GLuint texture;

    // Create a texture.
    OPENGL_CALL(glGenTextures(1, &texture));

    // Bind to temporary unit.
    // workspace.BindTextureUnit(workspace.NumTextureUnits() - 1, texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // TODO(zhixunt): What are these?
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    // Similar to cudaMemcpy.
    OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    OPENGL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, data));

    return texture;
}

void SetFrameBuffer(int W, int H, TYPE output_texture){
    OPENGL_CALL(glViewport(0, 0, W, H));

    // Set "renderedTexture" as our colour attachement #0
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture , 0);
    // Always check that our framebuffer is ok
    // GLenum flag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // LOG_IF(FATAL, flag != GL_FRAMEBUFFER_COMPLETE) << "Framebuffer not complete.";
}

void SetInt(const std::string name, const int value)
{
    glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

void SetFloat(const std::string name, const float value)
{
    glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}

void SetInput2D( std::string name, GLuint id,  int tex_id)
{
    GLint location= glGetUniformLocation(program, name.c_str());
    glUniform1i(location, tex_id);
    glActiveTexture(GL_TEXTURE0+tex_id);
    glBindTexture(GL_TEXTURE_2D, id);
}

void SetVertexShader()
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    // auto point_attrib = GLuint(glGetAttribLocation(program, "point"));
    // OPENGL_CALL(glEnableVertexAttribArray(point_attrib));
    // OPENGL_CALL(glVertexAttribPointer(point_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr));
}

void Bind(const int W, const int H, const GLuint& input0, const float a)
{
    OPENGL_CALL(glUseProgram(program));

    SetFrameBuffer(W, H, 0);
    SetVertexShader();
    SetInput2D("img", input0, 0);

    // set uniform
    SetFloat("a", a);
}

void Render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // OPENGL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));
    glFinish();
}

int main() 
{
    std::cout << "H*W: " << H << "*" << W << std::endl;
    Timer timer_all;
    prepare_data();
    opengl::example::InitContext();
    GLuint frameBuffer = InitFrameBuffer();

    // Get max texture size
    GLint maxtexsize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
    // std::cout << "Max texture size = " << maxtexsize << std::endl;

    // Textures
    GLuint input0 = CreateTexture(data, W, H);
    GLuint input1;

    OPENGL_CALL(glGenTextures(1, &input1));
    glBindTexture(GL_TEXTURE_2D, input1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, input1, 0);
    
    CreateProgram("");

    Timer timer_pre;
    Bind(W, H, input0, 1.5);
    timer_pre.Timing("upload");

    Timer timer;
    const int count = 10;
    int i = count;
    while (i--)
    {
        Render();
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

    DestoryFrameBuffer(frameBuffer);
    opengl::example::DestroyContext();
}
