#include <GLFW/glfw3.h>
#include "timer.hpp"
#include "timer_glfw.hpp"

double GlfwTimer::Seconds() {
    return glfwGetTime();
}
