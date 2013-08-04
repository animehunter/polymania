/*
 * Abstract OpenGL context
 */

class Context {
public:
    virtual int Initialize(const char *hintTitle, int hintWidth, int hintHeight, bool hintFullscreen, bool hintVerticalSync) = 0;
    virtual int SwapBuffers() = 0;
    virtual int MakeCurrent() = 0;
    virtual int Poll() = 0;
    virtual int Terminate() = 0;
};
