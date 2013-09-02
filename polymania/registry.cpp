#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"

class Test : public Object {
	DECLARE_CLASS(Test)

public:
	void StaticConstructor() {
		REGISTER_HANDLER(Test, TestEvent)
	}

	bool OnTestEvent(Event &ev){
		std::cout << "Test::OnTestEvent" << std::endl;
		return true;
	}

	void Update(Scene &scene, std::shared_ptr<Controller> k){}
	void Draw(Scene &scene){}
};

BEGIN_REGISTRATION
	REGISTER_CLASS(Test)
END_REGISTRATION
