#include <cstdlib> 
#include <cstdio>
#include <memory>
#include <unordered_map>
#include <string>

#include "types.hpp"
#include "resource.hpp"

using std::FILE;

class ResourceMemoryAllocatorDefault : public ResourceMemoryAllocator {
public:
    void *Allocate(UInt inSize, UInt32 inAlignment) {
        return std::malloc(inSize);
    }
    void *Reallocate(void *inPtr, UInt inSize, UInt32 inAlignment) {
        return std::realloc(inPtr, inSize);
    }
    void Free(void *inPtr) {
        return std::free(inPtr);
    }
};

ResourceMemoryAllocator *GetDefaultAllocatorInstance() {
    static ResourceMemoryAllocatorDefault inst;
    return &inst;
}

ResourceMemoryAllocator *ResourceMemoryAllocator::instance = GetDefaultAllocatorInstance();


//////////////////////////////////////////////////////////////////////////

struct CloseResourceOnDestroy {
    ResourceDirectory *owner;
    void (ResourceDirectory::*close)(ResourceIo *res);
    CloseResourceOnDestroy(ResourceDirectory *owner, 
                           void (ResourceDirectory::*close)(ResourceIo *res)) : owner(owner), close(close) {
    }
    inline void operator()(ResourceIo *res) const {
        (owner->*close)(res);
    }
};


class ResourceIoDisk : public ResourceIo {
    friend class ResourceDirectoryDisk;

public:
    ResourceIoDisk(FILE *fp, bool writable) : fp(fp), writable(writable) {

    }

    Int Read(void *outBuffer, UInt inSizeBytes) {
        return fread(outBuffer, 1, inSizeBytes, fp);
    }
    Int Write(const void *inBuffer, UInt inSizeBytes) {
        return fwrite(inBuffer, 1, inSizeBytes, fp);
    }
    bool Seek(UInt inOffset, Int32 inOrigin) {
        Int succ;
        switch(inOrigin) {
            case ORIGIN_Set:
                succ = fseek(fp, inOffset, SEEK_SET);
                break;
            case ORIGIN_Cur:
                succ = fseek(fp, inOffset, SEEK_SET);
                break;
            case ORIGIN_End:
                succ = fseek(fp, inOffset, SEEK_SET);
                break;
            default:
                return false;
        }

        return succ ? false : true;
    }
    virtual UInt Tell() const {
        return ftell(fp);
    }
    bool IsWritable() const {
        return writable;
    }

private:
    FILE *fp;
    bool writable;
};

class ResourceDirectoryDisk : public ResourceDirectory {
public:
    bool IsWritable() const {
        return true;
    }

private:
    ResourceIo *InternalOpen(const std::string &inLocation, Int32 inPermission) {
        char *mode;
        bool readonly;
        switch(inPermission) {
            case PERMISSION_ReadOnly:
                mode = "rb";
                readonly = true;
                break;

            case PERMISSION_ReadWriteUpdate:
                mode = "rb+";
                readonly = false;
                break;

            case PERMISSION_ReadWriteTruncate:
                mode = "wb";
                readonly = false;
                break;

            default:
                mode = "rb";
                readonly = true;
                break;
        }

        FILE *fp = std::fopen(inLocation.c_str(), mode);
        if(fp) {
            void *resourceIoDiskRaw = ResourceMemoryAllocator::instance->Allocate(sizeof(ResourceIoDisk));
            return new(resourceIoDiskRaw)ResourceIoDisk(fp, readonly);
        } else {
            return 0;
        }
    }
    void InternalClose(ResourceIo *res) {
        ResourceIoDisk *io = static_cast<ResourceIoDisk*>(res);
        fclose(io->fp);
        io->~ResourceIoDisk();
        ResourceMemoryAllocator::instance->Free(io);
    }
};

std::shared_ptr<ResourceIo> ResourceDirectory::Open(const std::string &inLocation, Int32 inPermission)  {
    return std::shared_ptr<ResourceIo>(InternalOpen(inLocation, inPermission), 
                                       CloseResourceOnDestroy(this, &ResourceDirectory::InternalClose));
}

ResourceDirectory *GetDefaultResourceLocatorInstance() {
    static ResourceDirectoryDisk inst;
    return &inst;
}

ResourceDirectory *ResourceDirectory::instance = GetDefaultResourceLocatorInstance();

//////////////////////////////////////////////////////////////////////////


struct DecRefOnDestroy {
    ResourceCache *owner;
    DecRefOnDestroy(ResourceCache *owner) : owner(owner) {
    }
    inline void operator()(Resource *res) const {
        owner->DecRef(res);
    }
};

void ResourceCache::DecRef( Resource *res ) {
    res->refCount--;
    if(res->refCount == 0) {
        auto it = linkedResources.find(res->location);
        if(it != linkedResources.end()) {
            unlinkedResources[it->first] = it->second;
            linkedResources.erase(it);
        }
    }
}

ResourceHandle ResourceCache::Load(const std::string &inLocation, bool hintForceReload) {
    // first check linkedResources
    auto itLinked = linkedResources.find(inLocation);
    if(itLinked != linkedResources.end()) {
        itLinked->second->refCount++;
        return ResourceHandle(itLinked->second, DecRefOnDestroy(this));
    }

    // if not found, check unlinkedResources and then link it
    auto itUnlinked = unlinkedResources.find(inLocation);
    if(itUnlinked != unlinkedResources.end()) {
        Resource *resource = itUnlinked->second;
        itUnlinked->second->refCount++;
        linkedResources[itUnlinked->first] = itUnlinked->second;
        unlinkedResources.erase(itUnlinked);
        return ResourceHandle(resource, DecRefOnDestroy(this));
    }

    // finally if not found, call Load() and then link it
    Resource *newRes = (Resource*)ResourceMemoryAllocator::instance->Allocate(typeSize);
    constructor(newRes);
    newRes->refCount++;
    newRes->location = inLocation;
    linkedResources[inLocation] = newRes;
    return ResourceHandle(newRes, DecRefOnDestroy(this));
}

void ResourceCache::Purge() {
    for(auto it=unlinkedResources.begin();it!=unlinkedResources.end();++it) {
        it->second->Unload();
    }
    for(auto it=unlinkedResources.begin();it!=unlinkedResources.end();++it) {
        it->second->~Resource();
        ResourceMemoryAllocator::instance->Free(it->second);
    }
    unlinkedResources.clear();
}


void ResourceManager::AddResourceLoader(const std::string &in3CharExtName, UInt32 inTypeSize, void(*inConstructor)(Resource*)) {
    caches[in3CharExtName] = std::make_shared<ResourceCache>(inTypeSize, inConstructor);
}
