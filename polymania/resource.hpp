#pragma once

class ResourceMemoryAllocator {
public:
    static ResourceMemoryAllocator *instance;

    ResourceMemoryAllocator() {}
    virtual ~ResourceMemoryAllocator() {}

    inline void *Allocate(UInt inSizeBytes) {
        return Allocate(inSizeBytes, sizeof(UInt));
    }
    inline void *Reallocate(void *inPtr, UInt inSizeBytes) {
        return Reallocate(inPtr, inSizeBytes, sizeof(UInt));
    }

    virtual void *Allocate(UInt inSizeBytes, UInt32 inAlignment)=0;
    virtual void *Reallocate(void *inPtr, UInt inSize, UInt32 inAlignment)=0;
    virtual void Free(void *inPtr)=0;
};

class ResourceIo {
public:
    enum EOrigin {
        ORIGIN_Set,
        ORIGIN_Cur,
        ORIGIN_End
    };

public:
    ResourceIo() {}
    virtual ~ResourceIo() {}

    virtual AsyncResult<Int> Read(void *outBuffer, UInt inSizeBytes)=0;
    virtual AsyncResult<Int> Write(const void *inBuffer, UInt inSizeBytes)=0;
    virtual bool Seek(UInt inOffset, Int32 inOrigin)=0;

    // return -1 if not seekable
    virtual Int Tell() const=0;
    virtual bool IsWritable() const=0;
    virtual bool IsSeekable() const=0;

    template<typename T>
    inline T Read() {
        T val = T();
        Read(&val, sizeof(T));
        return val;
    }
    template<typename T>
    inline void Write(T val) {
        Write(&val, sizeof(T));
    }
};

class ResourceDirectory {
public:
    static ResourceDirectory *instance;

    enum EPermission {
        PERMISSION_ReadOnly,
        PERMISSION_ReadWriteUpdate,
        PERMISSION_ReadWriteTruncate
    };

public:
    ResourceDirectory() {}
    virtual ~ResourceDirectory() {}

    AsyncResult<std::shared_ptr<ResourceIo>> Open(const std::string &inLocation, Int32 inPermission);
    virtual bool IsWritable() const=0;

protected:
    virtual ResourceIo *InternalOpen(const std::string &inLocation, Int32 inPermission)=0;
    virtual void InternalClose(ResourceIo *res)=0;
};

class Resource {
public:
    Int32 refCount;
    std::string location;

    Resource() : refCount(0) {}
    virtual ~Resource() {}

    // all Resource types must return a default resource if not found
    virtual bool Load(ResourceMemoryAllocator &inAllocator, ResourceIo &inIo)=0;
    virtual bool Unload()=0;
};

typedef std::shared_ptr<Resource> ResourceHandle;

class ResourceCache {
public:
    ResourceCache(UInt32 inTypeSize, void(*inConstructor)(Resource*)) : typeSize(inTypeSize), constructor(inConstructor) {}

    ResourceHandle Load(const std::string &inLocation, bool hintForceReload=false);
    void Purge(); // Unload all unlinked resources
    void DecRef(Resource *inResource);

private:
    // maps resource location -> resource
    std::unordered_map<std::string, Resource*> linkedResources; // resources that are currently in use
    std::unordered_map<std::string, Resource*> unlinkedResources; // resources that are currently not in use
    UInt32 typeSize;
    void(*constructor)(Resource*);
};

class ResourceManager {
private:
    template<typename T>
    struct CtorToFunc {
        static void Ctor(Resource *val) {
            (void*)new(val)T();
        }
    };

public:
    template<typename T>
    void AddResourceLoader(const std::string &in3CharExtName) {
        AddResourceLoader(in3CharExtName, sizeof(T), &CtorToFunc<T>::Ctor);
    }
    void AddResourceLoader(const std::string &in3CharExtName, UInt32 inTypeSize, void(*inConstructor)(Resource*));

    template<typename T>
    std::shared_ptr<T> Load(const std::string &location) {
        return std::static_pointer_cast<T>(Load(location));
    }
    std::shared_ptr<Resource> Load(const std::string &location);

    // resource type (3 char extension name) -> cache
    std::unordered_map<std::string, std::shared_ptr<ResourceCache>> caches;
};

