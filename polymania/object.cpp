#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>

#include "types.hpp"
#include "controller.hpp"
#include "object.hpp"

// Globals
std::unordered_map<std::string, Class> Object::globalClasses;
std::vector<Object::ObjectLink> Object::objectLinks;

const Event::Data Event::nullData = Event::Data();

std::string MetaField::typeNames[TYPE_Max];
const MetaField MetaField::nullField = MetaField();

void MetaField::StaticInitMetaTypeNames() {
    static_assert(sizeof(MetaField::typeNames)/sizeof(std::string) == 7, "MetaField has changed, don't forget to add the new type name below");

#define METAFIELD_MAKE_TYPENAME(ty, e) typeNames[e] = #e; { MetaField f = ty(); ty val = f; (void)val; }
    METAFIELD_MAKE_TYPENAME(NullField, TYPE_Null)
    METAFIELD_MAKE_TYPENAME(Int32, TYPE_Integer)
    METAFIELD_MAKE_TYPENAME(Int64, TYPE_Integer64)
    METAFIELD_MAKE_TYPENAME(Int32, TYPE_Boolean)
    METAFIELD_MAKE_TYPENAME(float, TYPE_Floating)
    METAFIELD_MAKE_TYPENAME(double, TYPE_Floating64)
    METAFIELD_MAKE_TYPENAME(std::string, TYPE_String)
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

Event::Data Event::MakeEventData() {
    return Event::Data();
}

Event::DataAssign Event::MakeEventData( const std::string &name, MetaField field ) {
    return Event::DataAssign(name, field);
}

const MetaField &Event::Get( const std::string& inName ) const {
    auto it = data.find(inName);
    if(it == data.end()) { 
        std::cerr << "COULD NOT find field:" << inName << " event:" << type << std::endl;
        return MetaField::nullField;
    } else return it->second;
}

void Object::StaticLinkClasses() {
    for (auto it = objectLinks.begin(); it != objectLinks.end();++it) {
        auto clsIt = globalClasses.find(it->currentClass);
        if(clsIt == globalClasses.end()) {
            std::cerr << "ERROR Could not find " << it->currentClass << " in RegisterClassesEnd" << std::endl;
        } else {
            auto baseIt = globalClasses.find(it->baseClass);
            if(baseIt == globalClasses.end()) {
                std::cerr << "WARNING Could not find base " << it->baseClass << " in RegisterClassesEnd" << std::endl;
            } else {
                clsIt->second.base = &baseIt->second;
            }
            clsIt->second.registerVar = it->registerVar;
        }
    }
    for (auto it = globalClasses.begin(); it != globalClasses.end();++it) {
        if(it->second.base == 0 && it->first != "Object") {
            std::cerr << "WARNING " << it->second.name << " has unresolved base" << std::endl;
        } else {
            // go all the way up to the topmost parent (Object)
            // while going down the hierarchy, repeatedly call registerVar on the current class
            std::vector<Class*> parents;
            Class *cur = &it->second;
            while(cur->name != "Object") {
                parents.push_back(cur);
                cur = cur->base;
            }
            for (auto parentIt = parents.begin();parentIt != parents.end();++parentIt) {
                for(auto phandlerIt = (*parentIt)->handlers.begin(); 
                    phandlerIt != (*parentIt)->handlers.end();
                    ++phandlerIt) {
                        if(it->second.handlers.find(phandlerIt->first) == it->second.handlers.end())
                            it->second.handlers[phandlerIt->first] = phandlerIt->second;
                }
            }
            for (auto parentIt = parents.rbegin();parentIt != parents.rend();++parentIt) {
                (*parentIt)->registerVar(it->second);
            }
        }
    }
}

bool Object::StaticInit() {
    MetaField::StaticInitMetaTypeNames();
    StaticRegisterClasses();
    for (auto iter = globalClasses.begin();
         iter != globalClasses.end();
         ++iter) {
        iter->second.constructorStatic(&iter->second);
    } 
    Object::StaticLinkClasses();
    return true;
}

Class* Object::StaticFindClass(const std::string name) {
    auto iterClass = globalClasses.find(name);
    if (iterClass == globalClasses.end()) return NULL;
    return &iterClass->second;
}

Object* Object::StaticConstructObject(Class* cls, const Event::Data& data) {
    // Null check
    if(!cls || !cls->constructor) return NULL;

    if(cls->size > 1024*1024) std::cerr << "WARNING Allocating object above 1MB, actual size: " << cls->size << std::endl;

    // Allocate the object
    Object *O = (Object*) malloc((UInt) cls->size);

    // Setup the object's properties
    O->_class = cls;

    // Construct the object and return it
    cls->constructor(O, Event(cls->name, data));
    return O;
}

Object* Object::StaticConstructObject(Class* cls) {
    return StaticConstructObject(cls, Event::nullData);
}

void Object::StaticDestroyObject(Object* obj) {
    // Null check
    if(!obj) return;

    obj->_class->destructor(obj);

    free(obj);
}

void Object::StaticConstructor(Class* cls) {}

void Object::Send( const Event &ev ) {
    auto handler = FindEventHandler(ev.type);
    // If the event couldn't be found
    if(handler) {
        handler(this, ev);
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
