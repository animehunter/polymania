#ifndef context_glfw_h__
#define context_glfw_h__

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

#endif // context_glfw_h__

