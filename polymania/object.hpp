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
    static std::unordered_map<std::string, Class> globalClasses;

    Class* _class;
    std::unordered_map<std::string, MetaField> _meta;

    static void RegisterClasses();
    
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
};

class Scene
{
public:
    double interp; //an interpolation value between the previous and the current frame for the purpose of drawing
    RenderBatcher batch;
    Shader shader;

    Scene();
    ~Scene();
    void Update(std::shared_ptr<Controller> k);
    void Draw();
};

#define DECLARE_CLASS(TClass)                                                  \
    public:                                                                   \
                                                                              \
        TClass(){}                                                            \
                                                                              \
        static void InternalConstructor(void* object){new(object) TClass();}  \
                                                                              \
        void* operator new(size_t size, void* mem){return mem;}               \
                                                                              \
    private:

                            
#define BEGIN_REGISTRATION void Object::RegisterClasses() {
#define REGISTER_CLASS(TClass) \
    Object::globalClasses.insert(std::make_pair<std::string, Class>(#TClass, Class(#TClass, sizeof(TClass), (Class::Constructor) &TClass::InternalConstructor, (Class::StaticConstructor) &TClass::StaticConstructor)));
#define END_REGISTRATION };

#define REGISTER_HANDLER(TClass, THandler)\
    GetClass()->handlers.insert(std::pair<const std::string, Event::Handler>(#THandler, (Event::Handler) &TClass::On##THandler));
