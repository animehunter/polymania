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

    bool OnTestEvent(const Event &ev){
        std::cout << "Test::OnTestEvent" << std::endl;
        return true;
    }

    Test(const Event &ev){}
    void Update(GameSystem &game, const std::shared_ptr<Controller> &k){}
    void Draw(GameSystem &game){}
};

CLASS_BEGIN_REGISTRATION
    CLASS_REGISTER(Test)
    CLASS_REGISTER(GameSystem)
CLASS_END_REGISTRATION
