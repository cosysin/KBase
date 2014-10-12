
#include "kbase\path_service.h"

#include <forward_list>
#include <mutex>
#include <unordered_map>

#include "kbase\base_path_provider.h"
#include "kbase\error_exception_util.h"

using kbase::FilePath;
using kbase::PathKey;

namespace kbase {

FilePath BasePathProvider(PathKey);

}   // namespace kbase

namespace {

typedef kbase::PathService::ProviderFunc ProviderFunc;
typedef std::unordered_map<PathKey, FilePath> PathMap;

// Both |start| and |end| are used to prevent path keys claimed by different 
// providers being overlapped.
struct PathProvider {
    ProviderFunc fn;
    PathKey start;
    PathKey end;
};

// We keep providers in a linked list, and ensure that our provider is always placed
// in tail.
typedef std::forward_list<PathProvider> ProviderChain;

struct PathData {
    ProviderChain providers;
    std::mutex lock;
    PathMap cached_path_table;
    bool cache_disabled;

    PathData()
        : providers({ PathProvider { kbase::BasePathProvider,
                                     kbase::BASE_PATH_START,
                                     kbase::BASE_PATH_END }}),
          cache_disabled(false)
    {}
};

// TODO: replace with LazyInstance object
PathData g_path_data;

PathData& GetPathData()
{
    return g_path_data;
}

// Returns the path corresponding to the key, or an empty path if no cache was found.
// The caller takes responsibility for thread safety.
FilePath GetPathFromCache(PathKey key, const PathData& path_data)
{
    if (path_data.cache_disabled || key == kbase::DIR_CURRENT) {
        return FilePath();
    }

    auto it = path_data.cached_path_table.find(key);
    if (it != path_data.cached_path_table.end()) {
        return it->second;
    }

    return FilePath();
}

// The caller takes responsibility for thread safety.
void EnsureNoPathKeyOverlapped(PathKey start, PathKey end, const PathData& path_data)
{
    for (const PathProvider& provider : path_data.providers) {
        ENSURE(start >= provider.end || end <= provider.start)
            (start)(end)(provider.start)(provider.end).raise();
    }
}

}   // namespace

namespace kbase {

// static
FilePath PathService::Get(PathKey key)
{
    PathData& path_data = GetPathData();
    ENSURE(key >= BASE_PATH_START)(key).raise();
    ENSURE(path_data.providers.empty() == false).raise();

    ProviderChain::const_iterator provider;
    {
        std::lock_guard<std::mutex> scoped_lock(path_data.lock);
        FilePath&& path = GetPathFromCache(key, path_data);
        if (!path.empty()) {
            return path;
        }

        // To prevent head being modified by accidently registering a new provider 
        // from other threads.
        provider = path_data.providers.begin();
    }

    FilePath path;
    for (; provider != path_data.providers.end(); ++provider) {
        path = provider->fn(key);
        if (!path.empty()) {
            break;
        }
    }

    // No corresponding path to the path key.
    if (path.empty()) {
        return path;
    }

    // Ensure that the returned path never contains '..'.
    if (path.ReferenceParent()) {
        // TODO: MakeAbsolutePath(path)
    }

    // Special case for current direcotry: We never cache it.
    std::lock_guard<std::mutex> scoped_lock(path_data.lock);
    if (!path_data.cache_disabled && key != DIR_CURRENT) {
        path_data.cached_path_table[key] = path;
    }
    
    return path;
}

// static
void PathService::RegisterPathProvider(ProviderFunc provider,
                                       PathKey start, PathKey end)
{
    PathData& path_data = GetPathData();
    ENSURE(path_data.providers.empty() == false).raise();
#ifdef _DEBUG
    ENSURE(start < end)(start)(end).raise();
#endif

    std::lock_guard<std::mutex> scoped_lock(path_data.lock);

#ifdef _DEBUG
    EnsureNoPathKeyOverlapped(start, end, path_data);
#endif

    path_data.providers.emplace_front(PathProvider { provider, start, end });
}

// static
void PathService::DisableCache()
{
    PathData& path_data = GetPathData();

    std::lock_guard<std::mutex> scoped_lock(path_data.lock);

    if (path_data.cache_disabled) {
        return;
    }

    path_data.cached_path_table.clear();
    path_data.cache_disabled = true;
}

// static
void PathService::EnableCache()
{
    PathData& path_data = GetPathData();

    std::lock_guard<std::mutex> scoped_lock(path_data.lock);

    if (!path_data.cache_disabled) {
        return;
    }

    path_data.cache_disabled = false;
}

}   // namespace kbase