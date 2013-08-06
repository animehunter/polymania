#pragma once

/*
 * Raspberry Pi implementation of the controller
 */
class RaspberryPiController : public Controller {
public:
    void Poll(Context *ctx);
};
