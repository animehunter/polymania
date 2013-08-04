/*
 * Abstract OpenGL context
 */

class Context {
public:
    virtual int Initialize(int hintWidth, int hintHeight, bool hintFullscreen, bool hintVerticalSync) = 0;
    virtual int SwapBuffers() = 0;
    virtual int Terminate() = 0;
};
