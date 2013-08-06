#pragma once

/*
 * Multi-platform GLFW implementation of the GL context class
 */
class GlfwContext: public Context {
public:
    int Initialize(const char *hintTitle, int hintWidth, int hintHeight, bool hintFullscreen, bool hintVerticalSync);
    int SwapBuffers();
    int MakeCurrent();
    int Poll();
    int Terminate();

    GLFWwindow *GetGlfwWindow();

private:
    GLFWwindow *context;
    bool vsync;
};

