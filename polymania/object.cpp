#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"

// Globals
std::unordered_map<std::string, Class> Object::globalClasses;

bool Object::StaticInit() {
    StaticRegisterClasses();
    for (auto iter = globalClasses.begin();
         iter != globalClasses.end();
         ++iter)
    {
        Object* obj = Object::StaticConstructObject(&iter->second);
        (obj->*iter->second.constructorStatic)();
        delete(obj);
    } return true;
}

Class* Object::StaticFindClass(const std::string name) {
    auto iterClass = globalClasses.find(name);
    if (iterClass == globalClasses.end()) return NULL;
    return &iterClass->second;
}

Object* Object::StaticConstructObject(Class* cls) {
    // Null check
    if(!cls) return NULL;

    // Allocate the object
    Object *O = (Object*) new char[(Int32) cls->size];

    // Setup the object's properties
    O->_class = cls;

    // Construct the object and return it
    cls->constructor(O);
    return O;
}

void Object::StaticConstructor() {}
