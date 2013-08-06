#pragma once

/*
 * Raspberry Pi implementation of the GL context class
 */
class RaspberryPiContext: public Context {
    EGL_DISPMANX_WINDOW_T _window;
    EGLDisplay _display;
    EGLSurface _surface;
    EGLContext _context;

public:
    int Initialize(const char *hintTitle, int hintWidth, int hintHeight, bool hintFullscreen, bool hintVerticalSync);
    int SwapBuffers();
    int MakeCurrent();
    int Poll();
    int Terminate();
};
