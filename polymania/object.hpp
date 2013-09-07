#pragma once

// Forwards
class Object;
class Class;
class Scene;

struct MetaField {
    enum ETypes {
        TYPE_Integer,
        TYPE_Integer64,
        TYPE_Boolean,
        TYPE_Floating,
        TYPE_String
    };

    UInt32 type;
    union {
        Int32 integer;
        Int64 integer64;
        Int32 boolean;
        float floating;
    };
    std::string string;
};

struct Event {    
    // Handler function
    typedef bool (Object::*Handler)(Event &ev);

    // Events must have a type and priority
    Event(std::string inType, Int32 inPriority = 0)
        : type(inType), priority(inPriority) {};

    // Event info
    const std::string type;
    const std::unordered_map<std::string, MetaField> data;
    const Int32 priority;
};

class Class {
public:
    // Constructors
    typedef void (Object::*StaticConstructor)();
    typedef void (*Constructor)(void* object);

    // Initialize a class
    Class(const std::string inName, Int64 inSize, Constructor inCtor, StaticConstructor inCtorStatic)
        : name(inName), size(inSize), constructor(inCtor), constructorStatic(inCtorStatic) {};

    // Class info
    const std::string name;
    const Int64 size;
    Constructor constructor;
    StaticConstructor constructorStatic;
    std::unordered_map<std::string, Event::Handler> handlers;

};

class Object
{
    Class* _class;
    std::unordered_map<std::string, MetaField> _meta;
    
    static void StaticRegisterClasses();

protected:
    static std::unordered_map<std::string, Class> globalClasses;
    
public:
    static bool StaticInit();
    static Class* StaticFindClass(const std::string name);
    static Object* StaticConstructObject(Class* cls);
    void StaticConstructor();

    Object(){};
    virtual ~Object(){};    
    virtual void Update(Scene &scene, std::shared_ptr<Controller> k)=0;
    virtual void Draw(Scene &scene)=0;
    Class* GetClass() {return _class;}

    void Send(Event &ev) {
        auto it = GetClass()->handlers.find(ev.type);
        // If the event couldn't be found
        if(it == GetClass()->handlers.end()) {
            std::cerr << "Warning: Class " << GetClass()->name << " has not event " << ev.type << "!" << std::endl;
        }
        // Otherwise fire off the event
        else (this->*(it->second))(ev);
    }
};

template<class Klass>
class DeclaredClass : public Object {
private:
    void* operator new(size_t size, void* mem){return mem;}   
    static void InternalConstructor(void* object){new(object) Klass();}
    static void StaticRegisterClass(const char* klassName){
        globalClasses.insert(std::make_pair<std::string, Class>(klassName,
            Class(klassName, sizeof(Klass), (Class::Constructor) &Klass::InternalConstructor, (Class::StaticConstructor) &Klass::StaticConstructor)));
    }

protected:
    template<typename HandlerType>
    void RegisterHandler(HandlerType handlerFunction, const char* handlerName){
        GetClass()->handlers.insert(
            std::pair<const std::string, Event::Handler>(handlerName, (Event::Handler) handlerFunction));
    }

    typedef Klass Klass;
    friend Object;
};

#define CLASS_BEGIN_REGISTRATION void Object::StaticRegisterClasses() {
#define CLASS_REGISTER(Klass) Klass::StaticRegisterClass(#Klass);
#define CLASS_END_REGISTRATION }

#define HANDLER_BEGIN_REGISTRATION void StaticConstructor() {
#define HANDLER_REGISTER(Handler) RegisterHandler(&Klass::On##Handler,#Handler);
#define HANDLER_END_REGISTRATION }
