#pragma once

// Forwards
class Object;
class Class;
class Controller;
class GameSystem;

struct MetaField {
    enum ETypes {
        TYPE_Null,
        TYPE_Integer,
        TYPE_Integer64,
        TYPE_Boolean,
        TYPE_Floating,
        TYPE_Floating64,
        TYPE_String,
        TYPE_Max
    };

    UInt32 type;
    union {
        Int32 integer;
        Int64 integer64;
        Int32 boolean;
        float floating;
        double floating64;
    };
    std::string string;

    MetaField() : type(TYPE_Null) {}

#define METAFIELD_MAKE_CONVERTOR(t, e, v, vin, vout) MetaField(t val) : type(e), v(vin) {} \
                                                     operator t () const { ValidateType(e); return vout; }
    METAFIELD_MAKE_CONVERTOR(Int32,  TYPE_Integer,              integer,    val,         integer)
    METAFIELD_MAKE_CONVERTOR(Int64,  TYPE_Integer64,            integer64,  val,         integer64)
    METAFIELD_MAKE_CONVERTOR(bool,   TYPE_Boolean,              boolean,    val ? 1 : 0, boolean ? true : false)
    METAFIELD_MAKE_CONVERTOR(float,  TYPE_Floating,             floating,   val,         floating)
    METAFIELD_MAKE_CONVERTOR(double, TYPE_Floating64,           floating64, val,         floating64)
    METAFIELD_MAKE_CONVERTOR(const   std::string&, TYPE_String, string,     val,         string)
#undef METAFIELD_MAKE_CONVERTOR

private:
    static void StaticInitMetaTypeNames();
    bool ValidateType(ETypes t) const;

private:
    static std::string typeNames[TYPE_Max];
    friend class Object;
};

struct Event {    
    // Handler function
    typedef bool (Object::*Handler)(const Event &ev);
    typedef std::unordered_map<std::string, MetaField> Data;

    // Events must have a type and priority
    Event(const std::string& inType, Data& data, Int32 inPriority = 0)
        : type(inType), data(data), priority(inPriority) {};

    // Event info
    const std::string& type;
    Data& data;
    const Int32 priority;
};

class Class {
public:
    // Constructors
    typedef void (*StaticConstructor)(Class* klass);
    typedef void (*Constructor)(void* object, Event::Data& ev);
    typedef void (*Destructor)(void* object);

    // Initialize a class
    Class(const std::string inName, Int64 inSize, Constructor inCtor, Destructor inDtor, StaticConstructor inCtorStatic)
        : name(inName), size(inSize), constructor(inCtor), destructor(inDtor), constructorStatic(inCtorStatic) {};

    // Class info
    const std::string name;
    const Int64 size;
    Constructor constructor;
    Destructor destructor;
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
    static Object* StaticConstructObject(Class* cls, Event::Data& ev);
    static Object* StaticConstructObject(Class* cls);
    static void StaticDestroyObject(Object* obj);
    void StaticConstructor();

    Object() {}
    virtual ~Object(){};    
    Class* GetClass() {return _class;}
    Event::Handler FindEventHandler(const std::string& name);
    void Send(const Event &ev);

protected:
    virtual void Update(GameSystem &game, const std::shared_ptr<Controller> &k)=0;
    virtual void Draw(GameSystem &game)=0;
};

template<class TKlass>
class DeclaredClass : public Object {
private:
    void* operator new(size_t size, void* mem){return mem;}   
    static void InternalConstructor(void* object, Event::Data& ev){new(object) Klass(ev);}
    static void InternalDestructor(void* object){((Klass*)object)->~Klass();}
    static void StaticRegisterClass(const char* klassName){
        globalClasses.insert(std::make_pair<std::string, Class>(klassName,
            Class(klassName, sizeof(Klass), &Klass::InternalConstructor, &Klass::InternalDestructor, &Klass::StaticConstructor)));
    }

protected:
    template<typename HandlerType>
    static void RegisterHandler(Class* klass, HandlerType handlerFunction, const char* handlerName){
        klass->handlers.insert(
            std::pair<const std::string, Event::Handler>(handlerName, (Event::Handler) handlerFunction));
    }

    typedef TKlass Klass;
    friend Object;
};

#define CLASS_BEGIN_REGISTRATION void Object::StaticRegisterClasses() {
#define CLASS_REGISTER(Klass) Klass::StaticRegisterClass(#Klass);
#define CLASS_END_REGISTRATION }

#define HANDLER_BEGIN_REGISTRATION static void StaticConstructor(Class* klass) {
#define HANDLER_REGISTER(Handler) RegisterHandler(klass, &Klass::On##Handler,#Handler);
#define HANDLER_END_REGISTRATION }
