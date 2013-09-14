#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"
#include "game.hpp"

class Test : public Object {
public:
    HANDLER_BEGIN_REGISTRATION(Test, Object)
        HANDLER_REGISTER(TestEvent)
    HANDLER_END_REGISTRATION

    bool OnTestEvent(const Event &ev){
        std::cout << "Test::OnTestEvent variables: " << std::endl;
        auto &vars = this->GetClass()->vars;
        for(auto it = vars.begin(); it != vars.end(); ++it) {
            std::cout << it->second->type << " " << it->second->name 
                      << " pointer: " << it->second->GetPointer(this) 
                      << " value: " << *(int*)it->second->GetPointer(this) 
                      << std::endl;
        }
        return true;
    }
    PROPERTY(Int32, var)
    PROPERTY(Test*, selftest)
    PROPERTY(std::string, something)

    Test(const Event &ev) : Object(ev), selftest(this) {}
    void Update(GameSystem &game, const std::shared_ptr<Controller> &k){}
    void Draw(GameSystem &game){}
};

class TestChild : public Test {
public:
    HANDLER_BEGIN_REGISTRATION(TestChild, Test)
    HANDLER_END_REGISTRATION

    PROPERTY(Int32, childVar)

    TestChild(const Event &ev) : Test(ev), childVar(123) { var = 20; }
};

class TestGrandChild : public TestChild {
public:
    HANDLER_BEGIN_REGISTRATION(TestGrandChild, TestChild)
        HANDLER_REGISTER(TestEvent)
    HANDLER_END_REGISTRATION

    bool OnTestEvent(const Event &ev) {
        std::cout << "TestGrandChild::OnTestEvent" << std::endl;
        return Base::OnTestEvent(ev);
    }

    PROPERTY(Int64, childVar)

    TestGrandChild(const Event &ev) : TestChild(ev), childVar(456) { var = 30; }
};

CLASS_BEGIN_REGISTRATION
    globalClasses.insert(std::make_pair<std::string, Class>("Object", Class("Object", sizeof(Object), 0, 0, &Object::StaticConstructor))); 
    CLASS_REGISTER(Test)
    CLASS_REGISTER(TestGrandChild)
    CLASS_REGISTER(TestChild)
    CLASS_REGISTER(GameSystem)
CLASS_END_REGISTRATION
