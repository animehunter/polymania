#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "bcm_host.h"

#include "context.hpp"
#include "context_rpi.hpp"

int RaspberryPiContext::Initialize(int hintWidth, int hintHeight, bool hintFullscreen, bool hintVerticalSync) {
    // Initialize the Raspberry Pi GPU
    bcm_host_init();

    // Initialization data
    EGLBoolean result;
    EGLConfig config;
    EGLint config_index;
    EGL_DISPMANX_WINDOW_T dispman_window;
    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    // Framebuffer configuration
    static const EGLint framebuffer_attrs[] =
    {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE
    };

    // Get the default display
    _display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (_display == EGL_NO_DISPLAY) return -1;

    // Initialize a connection with the display
    result = eglInitialize(_display, NULL, NULL);
    if (result == EGL_FALSE) return -1;

    // Choose frame buffer configuration
    result = eglChooseConfig(_display, framebuffer_attrs, &config, 1, &config_index);
    if (result == EGL_FALSE) return -1;

    // create an EGL rendering context
    _context = eglCreateContext(_display, config, EGL_NO_CONTEXT, NULL);
    if (_context == EGL_NO_CONTEXT) return -1;

    // Get display size
    uint32_t screen_width;
    uint32_t screen_height;
    int32_t success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
    if (success < 0) return -1;

    // Setup the window surface
    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = screen_width;
    dst_rect.height = screen_height;
    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = screen_width << 16;
    src_rect.height = screen_height << 16;        

    dispman_display = vc_dispmanx_display_open(0 /* LCD */);
    dispman_update  = vc_dispmanx_update_start(0);         
    dispman_element = vc_dispmanx_element_add(
        dispman_update, dispman_display, 0/*layer*/, &dst_rect, 0/*src*/, &src_rect,
        DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T) 0/*transform*/);
      
    dispman_window.element = dispman_element;
    dispman_window.width   = screen_width;
    dispman_window.height  = screen_height;
    vc_dispmanx_update_submit_sync(dispman_update);
      
    _surface = eglCreateWindowSurface(_display, config, &dispman_window, NULL);
    if (_surface == EGL_NO_SURFACE) return -1;

    // Make surface current
    result = eglMakeCurrent(_display, _surface, _surface, _context);
    if (result == EGL_FALSE) return -1;

    // Success
    return 0;
}

int RaspberryPiContext::SwapBuffers() {
    eglSwapBuffers(_display, _surface);
    return 0;
}

int RaspberryPiContext::Terminate() {
   // Destroy surface, context and display
   eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroySurface(_display, _surface);
   eglDestroyContext(_display, _context);
   eglTerminate(_display);
   return 0;
}