#include <GLFW/glfw3.h>

#include "context.hpp"
#include "context_glfw.hpp"
#include "controller.hpp"
#include "controller_glfw.hpp"

void GlfwController::Poll(Context *ctx) {
    GlfwContext *gctx = static_cast<GlfwContext*>(ctx);
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_DOWN)) {
        down = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_UP)) {
        up = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_LEFT)) {
        left = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_RIGHT)) {
        right = true;
    }
}

