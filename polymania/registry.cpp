#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"

class Test : public DeclaredClass<Test> {
public:
    HANDLER_BEGIN_REGISTRATION
        HANDLER_REGISTER(TestEvent)
    HANDLER_END_REGISTRATION

    bool OnTestEvent(Event &ev){
        std::cout << "Test::OnTestEvent" << std::endl;
        return true;
    }

    void Update(Scene &scene, std::shared_ptr<Controller> k){}
    void Draw(Scene &scene){}
};

CLASS_BEGIN_REGISTRATION
    CLASS_REGISTER(Test)
CLASS_END_REGISTRATION
