#pragma once

/*
 * Multi-platform GLFW implementation of the controller
 */
class GlfwController : public Controller {
public:
    void Poll(Context *ctx);
};
