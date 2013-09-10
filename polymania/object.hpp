#pragma once

// Forwards
class Object;
class Class;
class Controller;
class GameSystem;

struct MetaField {
    static const MetaField nullField;

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
    typedef bool (*Handler)(Object*, const Event &ev);
    typedef std::unordered_map<std::string, MetaField> Data;

private:
    struct DataAssign{ 
        DataAssign(const std::string &name, MetaField field) { d[name] = field; }
        DataAssign &operator()(const std::string &name, MetaField field) {
            d[name] = field;
            return *this;
        }
        operator const Data&() const { return d; }
        private: Data d;
    };

public:
    static Data MakeEventData();
    static DataAssign MakeEventData(const std::string &name, MetaField field);

    // Events must have a type and priority
    Event(const std::string& inType, const Data& inData, Int32 inPriority = 0)
        : type(inType), priority(inPriority), data(inData) {};

    Event(const std::string& inType, Int32 inPriority = 0)
        : type(inType), priority(inPriority), data(nullData) {};

    const MetaField &Get(const std::string& inName) const;

    const MetaField &operator[](const std::string& inName) const {
        return Get(inName);
    }

    // Event info
    const std::string& type;
    const Int32 priority;

private:
    static const Data nullData;
    const Data& data;
};

class Class {
public:
    // Constructors
    typedef void (*StaticConstructor)(Class* klass);
    typedef void (*Constructor)(void* object, const Event& ev);
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
    static Object* StaticConstructObject(Class* cls, const Event::Data& data);
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
    static void InternalConstructor(void* object, const Event& ev){new(object) Klass(ev);}
    static void InternalDestructor(void* object){((Klass*)object)->~Klass();}
    static void StaticRegisterClass(const char* klassName){
        globalClasses.insert(std::make_pair<std::string, Class>(klassName,
            Class(klassName, sizeof(Klass), &Klass::InternalConstructor, &Klass::InternalDestructor, &Klass::StaticConstructor)));
    }

protected:
    static void RegisterHandler(Class* klass, Event::Handler handlerFunction, const char* handlerName){
        klass->handlers.insert(
            std::pair<const std::string, Event::Handler>(handlerName, handlerFunction));
    }
    template<bool(TKlass::*Handler)(const Event&)>
    static bool MakeStaticHandler(Object *self, const Event& ev) { return (((TKlass*)self)->*Handler)(ev); }

    typedef TKlass Klass;
    friend Object;
};

#define CLASS_BEGIN_REGISTRATION void Object::StaticRegisterClasses() {
#define CLASS_REGISTER(Klass) Klass::StaticRegisterClass(#Klass);
#define CLASS_END_REGISTRATION }

#define HANDLER_BEGIN_REGISTRATION static void StaticConstructor(Class* klass) {
#define HANDLER_REGISTER(Handler) RegisterHandler(klass, &MakeStaticHandler<&Klass::On##Handler>, #Handler);
#define HANDLER_END_REGISTRATION }
