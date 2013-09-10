#pragma once

class Controller;
class Game;
class GameSystemImplementation;

class GameSystem : public DeclaredClass<GameSystem> {
public:
    //////////////////////////////////////////////////////////////////////////
    // Messages
    HANDLER_BEGIN_REGISTRATION
        HANDLER_REGISTER(CloseWindow)
        HANDLER_REGISTER(ResizeWindow)
    HANDLER_END_REGISTRATION

    // Implementations
    bool OnCloseWindow(Event &ev);
    bool OnResizeWindow(Event &ev);
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    GameSystem(Event::Data &ev);
    void Update(Game &inScene, const std::shared_ptr<Controller> &inController);
    void Draw(Game &inScene);

public:
    std::unique_ptr<GameSystemImplementation> impl;
};

class Game {
public:
    Game(Int32 inWidth, Int32 inHeight);
    void Update(const std::shared_ptr<Controller> &inController);
    void Draw();
    void SetInterpolation(double inInterp);

private:
    std::unique_ptr<GameSystem> impl;
};
