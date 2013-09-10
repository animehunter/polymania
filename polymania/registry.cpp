#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"
#include "game.hpp"

class Test : public DeclaredClass<Test> {
public:
    HANDLER_BEGIN_REGISTRATION
        HANDLER_REGISTER(TestEvent)
    HANDLER_END_REGISTRATION

    bool OnTestEvent(Event &ev){
        std::cout << "Test::OnTestEvent" << std::endl;
        return true;
    }

    Test(const Event::Data &ev){}
    void Update(Game &scene, const std::shared_ptr<Controller> &k){}
    void Draw(Game &scene){}
};

CLASS_BEGIN_REGISTRATION
    CLASS_REGISTER(Test)
    CLASS_REGISTER(GameSystem)
CLASS_END_REGISTRATION
