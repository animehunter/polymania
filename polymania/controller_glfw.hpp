#ifndef controller_glfw_h__
#define controller_glfw_h__

class GlfwController : public Controller {
public:
    void Poll(Context *ctx);
};

#endif // controller_glfw_h__
