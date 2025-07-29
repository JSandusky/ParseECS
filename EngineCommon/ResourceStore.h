#pragma once

#include <cstdint>
#include <unordered_map>

class Resource;
class ResourceLoader;
class ResourcePathMapper;

typedef uint32_t ResourceTypeID;

class ResourceStore final
{
public:
    ResourceStore();
    ~ResourceStore();

    struct ResourceUsage {
        /// Number of objects.
        uint32_t count_;
        /// Number of bytes used.
        uint32_t size_;
    };

    template<typename T>
    T* GetOrLoadResource(const char* resourcePath) {
        return (T*)GetOrLoadResource(T::GetResourceTypeID(), resourcePath);
    }
    Resource* GetOrLoadResource(uint32_t resourceType, const char* resourcePath);

    /// Call whenever a resource handle is invalidated to decrement reference counts
    void UnloadResource(Resource* resource);

private:
    std::unordered_map<ResourceTypeID, ResourceUsage> usageTracking_;
    std::unordered_map<uint32_t, Resource*> resources_;
    std::unordered_map<uint32_t, std::vector<ResourceLoader*> > loaders_;
    std::vector<ResourcePathMapper*> pathMappers_;
};