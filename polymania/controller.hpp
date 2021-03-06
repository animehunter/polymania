#pragma once

class Context;

struct ControllerPosition {
    double x, y;
};

/*
 * Abstract controller
 */
class Controller {
public:
    bool up, down, left, right;
    bool a,b,x,y;

    ControllerPosition touches[10];
    int nTouches;
    
    Controller() : up(false), down(false), left(false), right(false), a(false), b(false), x(false), y(false), nTouches(0) {
    }

    void Reset() {
        up = down = left = right = false;
        a = b = x = y = false;
        nTouches = 0;
    }

    virtual void Poll(Context *ctx) = 0;
};

