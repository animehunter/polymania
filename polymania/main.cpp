#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef __arm__
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#define GLFW_INCLUDE_ES2
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <iostream>
#include <type_traits>
#include <limits>
#include <memory>
#include <string>
#include <fstream>
#include <iterator>
#include <queue>

#include "types.hpp"

#ifdef _MSC_VER
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#endif

#include "context.hpp"
#include "controller.hpp"
#include "timer.hpp"
#include "asyncmodel.hpp"
#include "resource.hpp"
#include "shader.hpp"
#include "object.hpp"

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
// Utils

struct AutoVao {
    UInt32 vaoID;
    AutoVao() {
#ifndef __arm__
        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);
#endif
    }
    ~AutoVao() {
#ifndef __arm__
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vaoID);
#endif
    }
};

inline bool FloatEquals(float a, float b, const float epsilon = std::numeric_limits<float>::epsilon()) {
    float diff = b - a;
    return (diff < epsilon) && (diff > -epsilon);
}

inline bool FloatInside(float a, float low, float high, const float epsilon = std::numeric_limits<float>::epsilon()) {
    return a > (low+epsilon) && a < (high-epsilon);
}

inline std::string FileRead(const std::string &path) {
    std::ifstream f(path);

    if(!f) {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}


///////////////////////////////////////////////////////////

const int WIDTH = 800;
const int HEIGHT = 600;

int GWindowWidth = WIDTH;
int GWindowHeight = HEIGHT;

class Scene {
public:
    double interp; //an interpolation value between the previous and the current frame for the purpose of drawing
    ResourceManager resMan;
    RenderBatcher batch;
    Shader shader;
    float pcamx, pcamy;
    float camx, camy;

    Scene();
    ~Scene();
    void Update(std::shared_ptr<Controller> k);
    void Draw();
};

Scene::Scene() : camx(0), camy(0) {
    resMan.AddResourceLoader<ResourceShader>("glf");
    resMan.AddResourceLoader<ResourceShader>("glv");

    Shader::SetBlendFunc(Shader::BLEND_Transparent);

    std::string vertshader = resMan.Load<ResourceShader>("shaders/default.glv")->string;
    std::string fragshader = resMan.Load<ResourceShader>("shaders/default.glf")->string;
    shader.Initialize(vertshader, fragshader, true);
    shader["projection"] = glm::perspective(60.0f, float(GWindowWidth)/float(GWindowHeight), 0.1f, 100.0f);
    shader["modelview"] = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(camx, camy, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    batch.SetShader(shader);

    batch.Queue( 0.0f,  0.0f, 1.0f, 255, 120, 120, 255);
    batch.Queue(-0.5f,  0.0f, 1.0f, 0, 255, 255, 150);
    batch.Queue(-0.5f, -0.5f, 1.0f, 0, 255, 0, 0);

    batch.Queue( 0.0f,  0.0f, 1.0f, 255, 120, 120, 255);
    batch.Queue(-0.5f,  -0.5f, 1.0f, 0, 255, 0, 0);
    batch.Queue( 0.0f, -0.5f, 1.0f, 0, 255, 255, 150);
    batch.Upload(RenderBatcher::USAGE_Static);
}
Scene::~Scene() {

}
void Scene::Update(std::shared_ptr<Controller> k) {
    pcamx = camx; 
    pcamy = camy; 
    if(k->left)  camx -= 0.1f;
    if(k->right)  camx += 0.1f;
    if(k->up) camy += 0.1f;
    if(k->down) camy -= 0.1f;
}

void Scene::Draw() {
    if(pcamx != camx || pcamy != camy) {
        float icamx = pcamx+(camx-pcamx)*float(interp);
        float icamy = pcamy+(camy-pcamy)*float(interp);
        shader["modelview"] = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(icamx, icamy, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader["camx"] = icamx;
        shader["camy"] = icamy;
    }
    batch.Draw();
}

// TODO add this to message queue
static void OnResize(GLFWwindow *window, int w, int h) {
    GWindowWidth = w;
    GWindowHeight = h;
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

static void EngineMain(std::shared_ptr<Context> mainWindow) {
#ifdef __arm__
    auto timer = std::make_shared<RaspberryPiTimer>();
    auto ctlr = std::make_shared<RaspberryPiController>();
#else
    auto timer = std::make_shared<GlfwTimer>();
    auto ctlr = std::make_shared<GlfwController>();
#endif
    
    // setup GL
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);     
    glDisable(GL_SCISSOR_TEST); 
    glDisable(GL_STENCIL_TEST);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS); //use GL_LEQUAL for multipass shaders
    glDepthRange(0.0f, 1.0f);
    glClearDepth(1.0f);

    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);


    AutoVao autoVao; // auto VAO for opengl 3+
    const double SEC_PER_TICK = 1/20.0;
    Int32 fpsFrames = 0;
    double fpsElapsed = 0.0;
    double timeFrame = 0.0;
    double timeNextTick = 0.0;
    bool  running = true;
    Scene scene;

    while(running) {
        double timeStart = timer->Seconds();
        
        mainWindow->Poll();

        ctlr->Poll(mainWindow.get());
        
        int frameSkips = 10; // allow up to 8 frame skips
        while(timer->Seconds() > timeNextTick && frameSkips > 0) {
            scene.Update(ctlr);
            frameSkips--;
            timeNextTick += SEC_PER_TICK;
        }
        if(frameSkips < 10)
            ctlr->Reset();
        
        scene.interp = (timer->Seconds() + SEC_PER_TICK - timeNextTick)/SEC_PER_TICK;
        
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        scene.Draw();
        mainWindow->SwapBuffers();
        
        timeFrame += (timer->Seconds() - timeStart);
        fpsElapsed += timeFrame;

        timeFrame = 0.0;

        ++fpsFrames;
        if(fpsElapsed >= 3.0) {
            std::cout << (fpsFrames/fpsElapsed) << std::endl;
            fpsElapsed = 0.0;
            fpsFrames = 0;
        }
        
        // TODO implement quit message
        //running = !glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(mainWindow);
    }
}

int main() {
    Object::StaticInit();
    Object* testInstance = Object::StaticConstructObject(Object::StaticFindClass("Test"));
    if(testInstance) testInstance->Send(Event("TestEvent"));
    if(testInstance) testInstance->Send(Event("BadEventName"));

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
