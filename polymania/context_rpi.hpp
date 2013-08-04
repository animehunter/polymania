/*
 * Abstract OpenGL context
 */

class RaspberryPiContext: public Context {
    EGLDisplay _display;
    EGLSurface _surface;
    EGLContext _context;

public:
    int Initialize(int hintWidth, int hintHeight, bool hintFullscreen, bool hintVerticalSync);
    int SwapBuffers();
    int Terminate();
};

