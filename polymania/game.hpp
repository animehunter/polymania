#pragma once

class Controller;
class GameSystemImplementation;

class GameSystem : public DeclaredClass<GameSystem> {
public:
    // variables
    double interp; //an interpolation value between the previous and the current frame for the purpose of drawing
    bool quitRequested;

public:
    //////////////////////////////////////////////////////////////////////////
    // Messages
    HANDLER_BEGIN_REGISTRATION
        HANDLER_REGISTER(CloseWindow)
        HANDLER_REGISTER(ResizedWindow)
    HANDLER_END_REGISTRATION

    // Implementations
    bool OnCloseWindow(Event &ev);
    bool OnResizedWindow(Event &ev);

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    GameSystem(Event::Data &ev);
    void Update(const std::shared_ptr<Controller> &inController) { Update(*this, inController); }
    void Draw() { Draw(*this); }

    Int32 GetWidth() const;
    Int32 GetHeight() const;

private:
    void Update(GameSystem &game, const std::shared_ptr<Controller> &inController);
    void Draw(GameSystem &game);

private:
    std::unique_ptr<GameSystemImplementation> impl;
};

