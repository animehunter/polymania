#include <GLFW/glfw3.h>

#include "../context.hpp"
#include "../controller.hpp"
#include "context_glfw.hpp"
#include "controller_glfw.hpp"

void GlfwController::Poll(Context *ctx) {
    GlfwContext *gctx = static_cast<GlfwContext*>(ctx);
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_S)) {
        down = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_W)) {
        up = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_A)) {
        left = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_D)) {
        right = true;
    }

    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_4)) {
        a = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_5)) {
        b = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_1)) {
        x = true;
    }
    if(glfwGetKey(gctx->GetGlfwWindow(), GLFW_KEY_2)) {
        y = true;
    }
}

