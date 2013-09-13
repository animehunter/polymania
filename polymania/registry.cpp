#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"
#include "game.hpp"

class Test : public Object {
public:
    HANDLER_BEGIN_REGISTRATION(Test)
        HANDLER_REGISTER(TestEvent)
    HANDLER_END_REGISTRATION

    bool OnTestEvent(const Event &ev){
        std::cout << "Test::OnTestEvent variables: " << std::endl;
        auto &vars = this->GetClass()->vars;
        var = 10;
        selftest = this;
        for(auto it = vars.begin(); it != vars.end(); ++it) {
            std::cout << (*it)->type << " " << (*it)->name << " pointer: " << (*it)->GetPointer(this) << " value: " << *(int*)(*it)->GetPointer(this) << std::endl;
        }
        return true;
    }
    PROPERTY(Int32, var);
    PROPERTY(Test*, selftest);
    PROPERTY(std::string, something);

    Test(const Event &ev) {}
    void Update(GameSystem &game, const std::shared_ptr<Controller> &k){}
    void Draw(GameSystem &game){}
};

CLASS_BEGIN_REGISTRATION
    CLASS_REGISTER(Test)
    CLASS_REGISTER(GameSystem)
CLASS_END_REGISTRATION
