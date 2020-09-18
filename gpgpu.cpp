#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <glog/logging.h>
#include <sys/time.h> //test
#include <unistd.h>

#include "context.h"

#define DISPLAY 
const int H = 2400;
const int W = 3200;
const int num_channels = 1;
const int SLEEP_TIME = 0;

using TYPE = uint32_t;
const GLenum TYPE_ID = GL_UNSIGNED_BYTE;
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

using ARRAY_TYPE = std::vector<TYPE>;
// std::uniform_real_distribution<float> DIST(1.0f, 2.0f);
std::uniform_int_distribution<int> DIST(1, 3);
const std::string fs_src = "../matrix_mul_float.glsl";

GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;

const GLfloat vertices[] = {
    -1.0, -1.0, 0.0,    // bottomleft
    -1.0, 1.0, 0.0,     // topleft
    1.0, 1.0, 0.0,      // topright
    1.0, -1.0, 0.0      // bottom right
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
            std::cout << "\033[32muse time(" << message << "): \033[31m" << timeuse << "\033[32mms\033[37m" << std::endl;
        }
        Start();

        return timeuse;

    }

private:
    void Start(){gettimeofday(&start, nullptr);}

private:
    struct timeval start, end;
};

void PrintMatrix(const TYPE* data)
{
    #ifdef DISPLAY
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++)
            std::cout << std::setw(7) << data[i*W+j];
        std::cout << std::endl;
    }
    std::cout << std::endl;
    #endif
}

void GenData(const int num_elements, ARRAY_TYPE& texture)
{
    std::random_device rd;
    std::mt19937 mt(rd());

    texture.resize(num_elements, 0);
    
    for (size_t i = 0; i != num_elements; ++i) {
        texture[i] = DIST(mt);
    }

    PrintMatrix(texture.data());
}

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


void prepare_data(const int num_elements, TYPE* data) 
{
    std::random_device rd;
    std::mt19937 mt(rd());

    for (size_t i = 0; i != num_elements; ++i) {
        data[i] = DIST(mt);
    }

    PrintMatrix(data);
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
    GLuint shader = glCreateShader(shader_kind);        //用来编译源码，返回ID
    glShaderSource(shader, 1, &shader_src, nullptr);    // 传 glsl 进去，count 是源码个数，null 不限长度
    glCompileShader(shader);                            // GPU 编译

    // Check compile errors.
    GLint err;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &err);     // 查看状态

    GLint info_log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);       // 取值
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
    fragment_shader = CreateShader(GL_FRAGMENT_SHADER, ReadKernel(fs_src).c_str());

    // Create the program and link the shaders.
    program = glCreateProgram();                // 用于包裹 VS、PS，然后整个送给 GPU
    glAttachShader(program, vertex_shader);     // 把 shader 绑定到 program
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);                     // 封装 program，不再更新
    glUseProgram(program);                      // 同一时间只能用一个，来指定哪个被使用

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

GLuint CreateGPUData(const TYPE *data, GLsizei W, GLsizei H)
{
    GLuint texture;

    // Create a texture.
    OPENGL_CALL(glGenTextures(1, &texture));     // 同buffer，创建名字

    // Upload to temporary unit.
    // workspace.BindTextureUnit(workspace.NumTextureUnits() - 1, texture);
    glBindTexture(GL_TEXTURE_2D, texture);       // 同buffer，创建 texture
    // 通过设置纹理属性，把纹理映射到buffer上；此处设置了过大过小时需要作的插值算法，和尺寸以外的 padding； 
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    OPENGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    // 1. ------
    // Similar to cudaMemcpy.
    OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, TYPE_ID, data));      // CPU ——》GPU 传数据

    // 2. ------
    // OPENGL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, TYPE_ID, nullptr));
    // OPENGL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, W, H, GL_RGBA, TYPE_ID, data));        //同上，传一部分数据
    
    return texture;
}

void Upload()
{

}

void InitFrameBuffer(int W, int H, TYPE output_texture){
    OPENGL_CALL(glViewport(0, 0, W, H));        // 设置画布上需要绘制的位置

    // Set "renderedTexture" as our colour attachement #0
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture , 0);
    // Always check that our framebuffer is ok
    // GLenum flag = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // LOG_IF(FATAL, flag != GL_FRAMEBUFFER_COMPLETE) << "Framebuffer not complete.";
}

void SetInt(const std::string name, const int value)
{
    glUniform1i(glGetUniformLocation(program, name.c_str()), value);    // 获取 uniform 的位置； 赋值给 uniform
}

void SetFloat(const std::string name, const float value)
{
    glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}

void SetInput2D( std::string name, GLuint id,  int tex_id)
{
    GLint location= glGetUniformLocation(program, name.c_str());
    glUniform1i(location, tex_id);
    glActiveTexture(GL_TEXTURE0+tex_id);        //切换纹理单元
    glBindTexture(GL_TEXTURE_2D, id);           // 同buffer，创建 texture
}

void SetVertex(const GLfloat* vertices)
{
    auto loc = GLuint(glGetAttribLocation(program, "position"));       //获取顶点着色器中变量的位置，也可以直接用0
    OPENGL_CALL(glEnableVertexAttribArray(loc));                    // 让该变量可以访问
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, vertices);           
    // 指定顶点属性，传数据给 Attribute：【客户端对象】CPU 往 GPU 传数据；3个变量*4个单元，共12个值

    // 指定顶点属性，传数据给 Attribute：【顶点缓冲区对象】CPU 往 GPU 传数据；3个变量*4个单元，共12个值
    // genbuffer bindbuffer binddata
    // OPENGL_CALL(glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(vertices), nullptr));  
}

void SetFragment(const GLuint& input0, const float a)
{
    SetInput2D("A", input0, 0);
    SetFloat("a", a);
}

void Download()
{
    Timer timer_post;
    // 读取结果：
    // 1. 主动获取：glReadPixels、glCopyTexImage2D和glCopyTexSubImage2D
    // 2. 绑定 framebuffer：
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, result);
    OPENGL_CHECK_ERROR;
    timer_post.Timing("download");
}

void Render()
{
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);            // 指定需要绘制的信息，用于后续的绘制操作； 从第0个开始，绘制4个点； 扇形的三角形
    // OPENGL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));      // 绘制独立的三角形
    glFinish();                                     // 强制完成所有 gl 命令； 
}

GLuint CreateVertexShader()
{
    GLuint input;

    OPENGL_CALL(glGenTextures(1, &input));     // 同buffer，创建名字
    glBindTexture(GL_TEXTURE_2D, input);       // 同buffer，创建 texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, TYPE_ID, NULL);      // CPU ——》GPU 传数据
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, input, 0); // 把数据传给 FBO，从纹理，（也可以是RBO）

    OPENGL_CHECK_ERROR;

    return input;
}

void Sleep()
{
    std::cout << "sleep..." << std::endl;
    sleep(SLEEP_TIME);
    std::cout << "sleep end" << std::endl;
}

void Compute()
{
    Sleep();
    Timer timer;
    const int count = 1;
    int i = count;
    while (i--)
    {
        Render();
    }
    timer.Timing("compute : in iterator " + std::to_string(count));
    // Get data

    Sleep();
}

GLuint GetInput(const size_t num_elements)
{
    ARRAY_TYPE texture0;
    GenData(num_elements, texture0);
    GLuint input0 = CreateGPUData(texture0.data(), W, H);

    return input0;
}

void Activate()
{
    OPENGL_CALL(glUseProgram(program));
}

void InitContext()
{
    CreateVertexShader();
    CreateProgram("");
}

void SetOutput(GLuint texture)
{
    // Set "renderedTexture" as our colour attachement #0
    OPENGL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                       texture , 0));
    LOG_IF(FATAL, glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            << "Framebuffer not complete.";
}

void Layer(GLuint gpuInput, GLuint gpuOutput)
{
    std::cout << "-------------layer--------------" << std::endl;
    Activate();
    OPENGL_CALL(glViewport(0, 0, W, H));
    if (gpuOutput > 0)
        SetOutput(gpuOutput);

    Timer timer_pre;
    SetVertex(vertices);
    SetFragment(gpuInput, 2);
    timer_pre.Timing("upload");
}

int main() 
{
    std::cout << "H*W: " << H << "*" << W << std::endl;
    Timer timer_all;
    const size_t num_elements = W * H * num_channels;
    opengl::example::InitContext();
    GLuint frameBuffer = InitFrameBuffer();

    // Get max texture size
    GLint maxtexsize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
    std::cout << "max texture size: " << maxtexsize << std::endl;
    
    
    GLuint input0 = GetInput(num_elements);
    GLuint output0 = CreateGPUData(nullptr, W, H);
    
    InitContext();

    Layer(input0, output0);

    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);           // 对画布进行清空一下
    glClear(GL_COLOR_BUFFER_BIT);                   // 涂抹背景色
    Compute();

    Layer(output0, 0);
    Compute();

    Download();

    timer_all.Timing("total");
    PrintMatrix(result);
    DestoryFrameBuffer(frameBuffer);
    opengl::example::DestroyContext();
}
