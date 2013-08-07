#include <GLFW/glfw3.h>

#include "../context.hpp"
#include "../controller.hpp"
#include "context_glfw.hpp"
#include "controller_glfw.hpp"

void GlfwController::Poll(Context *ctx) {
    GlfwContext *gctx = static_cast<GlfwContext*>(ctx);
    GLFWwindow *gwin = gctx->GetGlfwWindow();

    if(glfwGetKey(gwin, GLFW_KEY_S) != GLFW_RELEASE) {
        down = true;
    }
    if(glfwGetKey(gwin, GLFW_KEY_W) != GLFW_RELEASE) {
        up = true;
    }
    if(glfwGetKey(gwin, GLFW_KEY_A) != GLFW_RELEASE) {
        left = true;
    }
    if(glfwGetKey(gwin, GLFW_KEY_D) != GLFW_RELEASE) {
        right = true;
    }

    if(glfwGetKey(gwin, GLFW_KEY_4) != GLFW_RELEASE) {
        a = true;
    }
    if(glfwGetKey(gwin, GLFW_KEY_5) != GLFW_RELEASE) {
        b = true;
    }
    if(glfwGetKey(gwin, GLFW_KEY_1) != GLFW_RELEASE) {
        x = true;
    }
    if(glfwGetKey(gwin, GLFW_KEY_2) != GLFW_RELEASE) {
        y = true;
    }

    if(glfwGetMouseButton(gwin, GLFW_MOUSE_BUTTON_1) != GLFW_RELEASE) {
        nTouches = 1;
        glfwGetCursorPos(gwin, &touches[0].x, &touches[0].y);
    }
}

