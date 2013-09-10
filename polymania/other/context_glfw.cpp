#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>

#include <GL/glew.h>

#if defined(_WIN32) || defined(WIN32)
#include <GL/wglew.h>
#endif

#include <GLFW/glfw3.h>

#include "../context.hpp"
#include "context_glfw.hpp"

#include "../types.hpp"
#include "../object.hpp"
#include "../globals.hpp"

#define GL_EXTENSION_EXISTS(ext) GLExtensionExists(ext, #ext)

static void OnResize(GLFWwindow *window, Int32 w, Int32 h) {
    Event::Data data;
    data["inWidth"] = w;
    data["inHeight"] = h;
    ((Object*)GGameSys)->Send(Event("ResizedWindow", data));
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

inline GLFWwindow *CreateContext(const char *hintTitle,  int hintWidth, int hintHeight, bool hintFullscreen) {
    GLFWwindow *window=0;

    // prefer 3.0 forward compat context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1); //forward compatibility removes deprecated functions
    window = glfwCreateWindow(hintWidth, hintHeight, hintTitle, 0, 0);

    if(!window) {
        // select 3.2 core as fall back
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);
        window = glfwCreateWindow(hintWidth, hintHeight, hintTitle, 0, 0);
    }
    if(!window) {
        // finally try 3.1 forward compat as fall back
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
        window = glfwCreateWindow(hintWidth, hintHeight, hintTitle, 0, 0);
    }

    glfwSetWindowSizeCallback(window, &OnResize);

    return window;
}

inline bool GLExtensionExists(GLboolean ext, const char *name) {
    if(!ext) {
        std::cerr << "Extension does not exist: " << name << std::endl;
        return false;
    } else {
        return true;
    }
}

inline bool CheckRequiredGLExtension() {
    return GL_EXTENSION_EXISTS(GLEW_ARB_vertex_buffer_object);
}

inline void EnableVSync() {
    glfwSwapInterval(1);
}

static void OnError(int code, const char *err) {
    std::cerr << err << std::endl;
}

int GlfwContext::Initialize(const char *hintTitle, int hintWidth, int hintHeight, bool hintFullscreen, bool hintVerticalSync) {
    glfwInit();
    glfwSetErrorCallback(&OnError);
    glfwDefaultWindowHints();

    context = CreateContext(hintTitle, hintWidth, hintHeight, hintFullscreen);
    if(!context) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(context);

    if(glewInit() != GLEW_OK) {
        std::cerr << "Failed to init glew" << std::endl;
        return -1;
    }

    if(!CheckRequiredGLExtension()) {
        std::cerr << "One or more extension is not found" << std::endl;
        return -1;
    }

    vsync = hintVerticalSync;
    if(vsync) EnableVSync();

    return 0;
}

int GlfwContext::SwapBuffers() {
    glfwSwapBuffers(context);
    return 0;
}

int GlfwContext::Poll() {
    glfwPollEvents();
    if(glfwGetKey(context, GLFW_KEY_ESCAPE) || glfwWindowShouldClose(context)) {
        Event::Data ev;
        ((Object*)GGameSys)->Send(Event("CloseWindow", ev));
    }

    return 0;
}

int GlfwContext::Terminate() {
    glfwTerminate();
    return 0;
}

GLFWwindow *GlfwContext::GetGlfwWindow() {
    return context;
}
