#pragma once

#include <stack>
#include <vector>

struct RecordedTime
{
    unsigned last_;
    unsigned average_;
    unsigned lowest_;
    unsigned highest_;
};

/// Simple per-thread hierarchical profiler.
struct Profiler
{
    /// 128 character name for the profiling stage.
    const char title_[128];
    /// Timing data.
    RecordedTime time_;
    RecordedTime calls_;
    /// Child passes in the profiler
    std::vector<Profiler*> children_;

    void Start();
    void Stop();
};

/// Per-thread global storage, calling thread is identified and then
struct ProfilingSystem
{
    /// Indexed by thread, 0 is the main thread.
    std::vector<Profiler*> roots_;
    /// Current profiler stack
    std::vector< std::stack<Profiler*> > currents_;

    /// Starts profiling, if there is a current then 
    void BeginProfile(const char* name);
    /// Stops profiling.
    void EndProfile(const char* name);

    /// Global shared instance
    static ProfilingSystem* instance_;
};

/// Start profiling
#define PROFILE_BEGIN(PROFILE_NAME) ProfilingSystem::instance_->BeginProfile(#PROFILE_NAME)

/// Stop profiling
#define PROFILE_END(PROFILE_NAME) ProfilingSystem::instance_->EndProfile(#PROFILE_NAME)