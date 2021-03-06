// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#if defined(__APPLE__)
#  include <sys/sysctl.h>
#elif !defined(_WIN32)
#  include <unistd.h>
#endif

#include "Thread.hpp"
#include "utils/Errors.hpp"

#if defined(_WIN32)
static const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
#endif

#if defined(_WIN32)
static DWORD WINAPI threadFunction(LPVOID parameter)
#else
static void* threadFunction(void* parameter)
#endif
{
    ouzel::Thread::State* state = static_cast<ouzel::Thread::State*>(parameter);

    if (!state->name.empty()) ouzel::Thread::setCurrentThreadName(state->name);

    state->function();

#if defined(_WIN32)
    return 0;
#else
    return NULL;
#endif
}

namespace ouzel
{
    uint32_t getCPUCount()
    {
#if defined(_WIN32)
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
        int mib[2];
        mib[0] = CTL_HW;
#  ifdef HW_AVAILCPU
        mib[1] = HW_AVAILCPU;
#  else
        mib[1] = HW_NCPU;
#  endif
        int count;
        size_t size = sizeof(count);
        if (sysctl(mib, 2, &count, &size, NULL, 0) != 0)
            throw ThreadError("Failed to get CPU count");

        return (count > 0) ? static_cast<uint32_t>(count) : 0;
#elif defined(__linux__) || defined(__ANDROID__)
        int count = sysconf(_SC_NPROCESSORS_ONLN);
        if (count == -1)
            throw ThreadError("Failed to get CPU count");
        return (count > 0) ? static_cast<uint32_t>(count) : 0;
#else
        return 1;
#endif
    }

    Thread::Thread(const std::function<void()>& function, const std::string& name):
        state(new State())
    {
        state->function = function;
        state->name = name;

#if defined(_WIN32)
        handle = CreateThread(nullptr, 0, threadFunction, state.get(), 0, &threadId);
        if (handle == nullptr)
            throw ThreadError("Failed to initialize thread");
#else
        if (pthread_create(&thread, NULL, threadFunction, state.get()) != 0)
            throw ThreadError("Failed to initialize thread");

        initialized = true;
#endif
    }

    Thread::~Thread()
    {
#if defined(_WIN32)
        if (handle)
        {
            WaitForSingleObject(handle, INFINITE);
            CloseHandle(handle);
        }
#else
        if (initialized) pthread_join(thread, nullptr);
#endif
    }

    Thread::Thread(Thread&& other)
    {
#if defined(_WIN32)
        handle = other.handle;
        threadId = other.threadId;
        other.handle = nullptr;
        other.threadId = 0;
#else
        thread = other.thread;
        initialized = other.initialized;
        other.initialized = false;

#endif
        state = std::move(other.state);
    }

    Thread& Thread::operator=(Thread&& other)
    {
        if (&other != this)
        {
#if defined(_WIN32)
            if (handle)
            {
                WaitForSingleObject(handle, INFINITE);
                CloseHandle(handle);
            }
            handle = other.handle;
            threadId = other.threadId;
            other.handle = nullptr;
            other.threadId = 0;
#else
            if (initialized) pthread_join(thread, nullptr);
            thread = other.thread;
            initialized = other.initialized;
            other.initialized = false;
#endif
            state = std::move(other.state);
        }

        return *this;
    }

    void Thread::join()
    {
#if defined(_WIN32)
        if (WaitForSingleObject(handle, INFINITE) == WAIT_FAILED)
            throw ThreadError("Failed to join thread");
#else
        if (pthread_join(thread, nullptr) != 0)
            throw ThreadError("Failed to join thread");

        initialized = false;
#endif
    }

    int32_t Thread::getPriority() const
    {
#if defined(_WIN32)
        return GetThreadPriority(handle);
#else
        int policy;
        sched_param param;
        if (pthread_getschedparam(thread, &policy, &param) != 0)
            throw ThreadError("Failed to get thread priority");

        return param.sched_priority;
#endif
    }

    void Thread::setPriority(int32_t priority)
    {
#if defined(_WIN32)
        if (!SetThreadPriority(handle, priority))
            throw ThreadError("Failed to set thread priority");
#else
        sched_param param;
        param.sched_priority = priority;
        if (pthread_setschedparam(thread, SCHED_RR, &param) != 0)
            throw ThreadError("Failed to set thread priority");
#endif
    }

    void Thread::setCurrentThreadName(const std::string& name)
    {
#if defined(_MSC_VER)
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name.c_str();
        info.dwThreadID = static_cast<DWORD>(-1);
        info.dwFlags = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
#else
#ifdef __APPLE__
        if (pthread_setname_np(name.c_str()) != 0)
            throw ThreadError("Failed to set thread name");
#elif defined(__linux__) || defined(__ANDROID__)
        if (pthread_setname_np(pthread_self(), name.c_str()) != 0)
            throw ThreadError("Failed to set thread name");
#endif
#endif
    }
}
