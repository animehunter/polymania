#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"

// Globals
std::unordered_map<std::string, Class> Object::globalClasses;

std::string MetaField::typeNames[TYPE_Max];
void MetaField::StaticInitMetaTypeNames() {
    static_assert(sizeof(MetaField::typeNames)/sizeof(std::string) == 7, "MetaField has changed, don't forget to add the new type name below");
#define METAFIELD_MAKE_TYPENAME(t) typeNames[t] = #t;
   METAFIELD_MAKE_TYPENAME(TYPE_Null)
   METAFIELD_MAKE_TYPENAME(TYPE_Integer)
   METAFIELD_MAKE_TYPENAME(TYPE_Integer64)
   METAFIELD_MAKE_TYPENAME(TYPE_Boolean)
   METAFIELD_MAKE_TYPENAME(TYPE_Floating)
   METAFIELD_MAKE_TYPENAME(TYPE_Floating64)
   METAFIELD_MAKE_TYPENAME(TYPE_String)
#undef METAFIELD_MAKE_TYPENAME
}

bool MetaField::ValidateType( ETypes to ) const{
    if(to != type) {
        std::cout << "CANNOT convert type from " << typeNames[type] << " to " << typeNames[to] << std::endl;
        return false;
    } else {
        return true;
    }
}

bool Object::StaticInit() {
    MetaField::StaticInitMetaTypeNames();
    StaticRegisterClasses();
    for (auto iter = globalClasses.begin();
         iter != globalClasses.end();
         ++iter)
    {
        iter->second.constructorStatic(&iter->second);
    } return true;
}

Class* Object::StaticFindClass(const std::string name) {
    auto iterClass = globalClasses.find(name);
    if (iterClass == globalClasses.end()) return NULL;
    return &iterClass->second;
}

Object* Object::StaticConstructObject(Class* cls, Event::Data& ev) {
    // Null check
    if(!cls) return NULL;

    // Allocate the object
    Object *O = (Object*) malloc((UInt) cls->size);

    // Setup the object's properties
    O->_class = cls;

    // Construct the object and return it
    cls->constructor(O, ev);
    return O;
}

Object* Object::StaticConstructObject(Class* cls) {
    Event::Data ev;
    return StaticConstructObject(cls, ev);
}

void Object::StaticDestroyObject(Object* obj) {
    // Null check
    if(!obj) return;

    obj->_class->destructor(obj);

    free(obj);
}

void Object::StaticConstructor() {}

void Object::Send( Event &ev ) {
    auto handler = FindEventHandler(ev.type);
    // If the event couldn't be found
    if(handler) {
        (this->*handler)(ev);
    }
}

Event::Handler Object::FindEventHandler( const std::string& name ) {
    auto it = GetClass()->handlers.find(name);
    // If the event couldn't be found
    if(it == GetClass()->handlers.end()) {
        std::cerr << "Warning: Class " << GetClass()->name << " has no event " << name << "!" << std::endl;
        return 0;
    }
    // Otherwise fire off the event
    else return it->second;
}

