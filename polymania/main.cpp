#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef __arm__
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#define GLFW_INCLUDE_ES2
#else
#include <GL/glew.h>
#endif

#if defined(_WIN32) || defined(WIN32)
#include <GL/wglew.h>
#endif

#include <GLFW/glfw3.h>

#include <vector>
#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <iostream>
#include <type_traits>
#include <limits>
#include <memory>

#include "types.hpp"

#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#endif

#include "context.hpp"
#include "controller.hpp"
#include "timer.hpp"

#ifdef __arm__
#include "rpi/context_rpi.hpp"
#include "rpi/controller_rpi.hpp"
#include "rpi/timer_rpi.hpp"
#else
#include "other/context_glfw.hpp"
#include "other/controller_glfw.hpp"
#include "other/timer_glfw.hpp"
#endif

///////////////////////////////////////////////////////////
// Shaders
#ifdef __arm__
const char *vertshader = "attribute vec2 pos;\n"
                         "attribute vec4 color;\n"
                         "varying vec4 colorIn; \n"
                         "void main()\n"
                         "{\n"
                         "   colorIn = color;\n"
                         "   gl_Position = vec4(pos, 0, 1.0);\n"
                         "}\n";


const char *fragshader = "varying vec4 colorIn;\n"
                         "void main()\n"
                         "{\n"
                         "   gl_FragColor = colorIn; \n"
                         "}\n";
#else
const char *vertshader = "#version 130\n"
                         "in vec2 pos;\n"
                         "in vec4 color;\n"
                         "out vec4 colorIn; \n"
                         "void main()\n"
                         "{\n"
                         "   colorIn = color;\n"
                         "   gl_Position = vec4(pos, 0, 1.0);\n"
                         "}\n";


const char *fragshader = "#version 130\n"
                         "in vec4 colorIn;\n"
                         "out vec4 fragcolor;\n"
                         "void main()\n"
                         "{\n"
                         "   fragcolor = colorIn; \n"
                         "}\n";
#endif

                         

///////////////////////////////////////////////////////////
// Utils

inline bool FloatEquals(F32 a, F32 b, const F32 epsilon = std::numeric_limits<F32>::epsilon())
{
    F32 diff = b - a;
    return (diff < epsilon) && (diff > -epsilon);
}

inline bool FloatInside(F32 a, F32 low, F32 high, const F32 epsilon = std::numeric_limits<F32>::epsilon())
{
    return a > (low+epsilon) && a < (high-epsilon);
}

inline void OrthoMatrix(F32 matrix[16], F32 left, F32 right, F32 bottom, F32 top, F32 znear, F32 zfar)
{
    F32 r_l = right - left;
    F32 t_b = top - bottom;
    F32 f_n = zfar - znear;
    F32 tx = - (right + left) / (right - left);
    F32 ty = - (top + bottom) / (top - bottom);
    F32 tz = - (zfar + znear) / (zfar - znear);

    matrix[0] = 2.0f / r_l;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = tx;

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / t_b;
    matrix[6] = 0.0f;
    matrix[7] = ty;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = 2.0f / f_n;
    matrix[11] = tz;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

///////////////////////////////////////////////////////////

const int WIDTH = 800;
const int HEIGHT = 600;

int GWindowWidth = WIDTH;
int GWindowHeight = HEIGHT;

class Scene;

class SceneObject
{
public:
    virtual ~SceneObject(){};
    virtual void Update(Scene &scene, std::shared_ptr<Controller> k)=0;
    virtual void Draw(Scene &scene)=0;
};

class Scene
{
public:
    F64 interp; //an interpolation value between the previous and the current frame for the purpose of drawing

    U32 progID, vshaderID, fshaderID;
    U32 vaoID;
    
    Scene();
    ~Scene();
    void Update(std::shared_ptr<Controller> k);
    void Draw();
};

Scene::Scene() 
{
#ifndef __arm__
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
#endif

    progID = glCreateProgram();
    vshaderID = glCreateShader(GL_VERTEX_SHADER);
    fshaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(vshaderID, 1, &vertshader, 0);
    glShaderSource(fshaderID, 1, &fragshader, 0);
    glCompileShader(vshaderID);
    glCompileShader(fshaderID);
    
    GLint compiled, linked;
    glGetShaderiv(vshaderID, GL_COMPILE_STATUS, &compiled);
    if (compiled)
    {
        // yes it compiled!
        std::cout << "Compiled vertshader" << std::endl;
    } 
    glGetShaderiv(fshaderID, GL_COMPILE_STATUS, &compiled);
    if (compiled)
    {
        // yes it compiled!
        std::cout << "Compiled fragshader" << std::endl;
    }
    glAttachShader(progID, vshaderID);
    glAttachShader(progID, fshaderID);

#ifndef __arm__
    glBindFragDataLocation(progID, 0, "fragcolor");
#endif

    glLinkProgram(progID);

    glGetProgramiv(progID, GL_LINK_STATUS, &linked);
    if (linked)
    {
       std::cout << "Linked prog" << std::endl;
    }
    
    glUseProgram(progID);
}
Scene::~Scene()
{
#ifndef __arm__
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vaoID);
#endif
    glDeleteShader(vshaderID);
    glDeleteShader(fshaderID);
    glDeleteProgram(progID);
}
void Scene::Update(std::shared_ptr<Controller> k)
{

}
void Scene::Draw()
{
}

// TODO add this to message queue
static void OnResize(GLFWwindow *window, int w, int h)
{
    GWindowWidth = w;
    GWindowHeight = h;
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

static void EngineMain(std::shared_ptr<Context> mainWindow)
{
#ifdef __arm__
    auto timer = std::make_shared<RaspberryPiTimer>();
    auto ctlr = std::make_shared<RaspberryPiController>();
#else
    auto timer = std::make_shared<GlfwTimer>();
    auto ctlr = std::make_shared<GlfwController>();
#endif

    const F64 SEC_PER_TICK = 1/20.0;
    S32 fpsFrames = 0;
    F64 fpsElapsed = 0.0;
    F64 timeFrame = 0.0;
    F64 timeNextTick = 0.0;
    bool  running = true;
    Scene scene;

    glDisable(GL_DEPTH_TEST); 
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);     
    glDisable(GL_SCISSOR_TEST); 
    glDisable(GL_STENCIL_TEST);
    
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    while(running)
    {
        F64 timeStart = timer->Seconds();
        
        mainWindow->Poll();

        ctlr->Poll(mainWindow.get());
        
        int frameSkips = 10; // allow up to 8 frame skips
        while(timer->Seconds() > timeNextTick && frameSkips > 0)
        {
            scene.Update(ctlr);
            frameSkips--;
            timeNextTick += SEC_PER_TICK;
        }
        if(frameSkips < 10)
            ctlr->Reset();
        
        scene.interp = (timer->Seconds() + SEC_PER_TICK - timeNextTick)/SEC_PER_TICK;
        
        glClear(GL_COLOR_BUFFER_BIT);
        scene.Draw();
        mainWindow->SwapBuffers();
        
        timeFrame += (timer->Seconds() - timeStart);
        fpsElapsed += timeFrame;

        timeFrame = 0.0;

        ++fpsFrames;
        if(fpsElapsed >= 3.0)
        {
            std::cout << (fpsFrames/fpsElapsed) << std::endl;
            fpsElapsed = 0.0;
            fpsFrames = 0;
        }

        // TODO implement quit message
        //running = !glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(mainWindow);
    }
}

int main()
{
#ifdef __arm__
    auto ctx = std::make_shared<RaspberryPiContext>();
#else
    auto ctx = std::make_shared<GlfwContext>();
#endif

    if(ctx->Initialize("Polymania Project", WIDTH, HEIGHT, false, false) < 0) {
        std::cerr << "Failed to initialize context" << std::endl;
        return -1;
    }


    std::cout << "Renderer: " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << (const char*)glGetString(GL_VERSION) << std::endl;
    
    // TODO implement resize message
    //OnResize(mainWindow, WIDTH, HEIGHT);
    //glfwSetWindowSizeCallback(mainWindow, &OnResize);

    EngineMain(ctx);

    ctx->Terminate();

    return 0;
}
