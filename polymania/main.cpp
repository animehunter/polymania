#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <GL/glew.h>

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

#include "types.h"

#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#endif


///////////////////////////////////////////////////////////
// Shaders
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

inline bool GLExtensionExists(GLboolean ext, const char *name)
{
    if(!ext)
    {
        std::cerr << "Extension does not exist: " << name << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

#define GL_EXTENSION_EXISTS(ext) GLExtensionExists(ext, #ext)


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


class Keys
{
public:
    bool up, down, left, right;
    Keys()
    {
        Reset();
    }
    void Reset()
    {
        up = down = left = right = false;
    }
};

class SceneObject
{
public:
    virtual ~SceneObject(){};
    virtual void Update(Scene &scene, const Keys &k)=0;
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
    void Update(const Keys &k);
    void Draw();
};

Scene::Scene() 
{

    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    progID = glCreateProgram();
    vshaderID = glCreateShader(GL_VERTEX_SHADER);
    fshaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(vshaderID, 1, &vertshader, 0);
    glShaderSource(fshaderID, 1, &fragshader, 0);
    glCompileShader(vshaderID);
    glCompileShader(fshaderID);
    
    GLint compiled, linked;
    glGetObjectParameterivARB(vshaderID, GL_COMPILE_STATUS, &compiled);
    if (compiled)
    {
        // yes it compiled!
        std::cout << "Compiled vertshader" << std::endl;
    } 
    glGetObjectParameterivARB(fshaderID, GL_COMPILE_STATUS, &compiled);
    if (compiled)
    {
        // yes it compiled!
        std::cout << "Compiled fragshader" << std::endl;
    }
    glAttachShader(progID, vshaderID);
    glAttachShader(progID, fshaderID);
    glBindFragDataLocation(progID, 0, "fragcolor");
    
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
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vaoID);
    glDeleteShader(vshaderID);
    glDeleteShader(fshaderID);
    glDeleteProgram(progID);
}
void Scene::Update(const Keys &k)
{

}
void Scene::Draw()
{
}

inline void EnableVSync()
{
#if defined(_WIN32) || defined(WIN32)
    if(WGLEW_EXT_swap_control)
        wglSwapIntervalEXT(1);
#else
    glfwSwapInterval(1);
#endif
}
inline bool CheckRequiredGLExtension()
{
    return GL_EXTENSION_EXISTS(GLEW_ARB_vertex_buffer_object);
}
static void OnResize(GLFWwindow *window, int w, int h)
{
    GWindowWidth = w;
    GWindowHeight = h;
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}
static void OnError(int code, const char *err)
{
    std::cerr << err << std::endl;
}

static void EngineMain(GLFWwindow *mainWindow)
{
    const F64 SEC_PER_TICK = 1/20.0;
    S32 fpsFrames = 0;
    F64 fpsElapsed = 0.0;
    F64 timeFrame = 0.0;
    F64 timeNextTick = 0.0;
    bool  running = true;
    Keys keys;
    Scene scene;
    
    //EnableVSync();
    
    glDisable(GL_DEPTH_TEST); 
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);     
    glDisable(GL_SCISSOR_TEST); 
    glDisable(GL_STENCIL_TEST);
    
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glfwSetTime(0);
    while(running)
    {
        F64 timeStart = glfwGetTime();
        
        glfwPollEvents();
        
        if(glfwGetKey(mainWindow, GLFW_KEY_DOWN))
        {
            keys.down = true;
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_UP))
        {
            keys.up = true;
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_LEFT))
        {
            keys.left = true;
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_RIGHT))
        {
            keys.right = true;
        }
        
        int frameSkips = 10; // allow up to 8 frame skips
        while(glfwGetTime() > timeNextTick && frameSkips > 0)
        {
            scene.Update(keys);
            frameSkips--;
            timeNextTick += SEC_PER_TICK;
        }
        if(frameSkips < 10)
            keys.Reset();
        
        scene.interp = (glfwGetTime() + SEC_PER_TICK - timeNextTick)/SEC_PER_TICK;
        
        glClear(GL_COLOR_BUFFER_BIT);
        scene.Draw();
        glfwSwapBuffers(mainWindow);
        
        timeFrame += (glfwGetTime() - timeStart);
        fpsElapsed += timeFrame;

        timeFrame = 0.0;

        ++fpsFrames;
        if(fpsElapsed >= 3.0)
        {
            std::cout << (fpsFrames/fpsElapsed) << std::endl;
            fpsElapsed = 0.0;
            fpsFrames = 0;
        }

        // exit if ESC was pressed or window was closed
        running = !glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(mainWindow);
    }
}
int main()
{
    glfwInit();
    glfwSetErrorCallback(&OnError);
    glfwDefaultWindowHints();
    // prefer 3.0 forward compat context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1); //forward compatibility removes deprecated functions
    GLFWwindow *mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Polymania Project", 0, 0);
    
    if(!mainWindow)
    {
        // select 3.2 core as fall back
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);
        mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Polymania Project", 0, 0);
    }
    if(!mainWindow)
    {
        // finally try 3.1 forward compat as fall back
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
        mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Polymania Project", 0, 0);
    }
    if(!mainWindow)
    {
        std::cerr << "failed to create gl context" << std::endl;
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(mainWindow);
    if(glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to init glew" << std::endl;
    }
    if(!CheckRequiredGLExtension())
    {
        std::cerr << "One or more extension is not found" << std::endl;
    }
    std::cout << "Renderer: " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << (const char*)glGetString(GL_VERSION) << std::endl;
    OnResize(mainWindow, WIDTH, HEIGHT);
    glfwSetWindowSizeCallback(mainWindow, &OnResize);

    EngineMain(mainWindow);

    glfwTerminate();

    return 0;
}
