// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include <functional>
#include <memory>
#include <string>
#if defined(_WIN32)
#  include <Windows.h>
#  define ThreadLocal __declspec(thread)
#else
#  include <pthread.h>
#  define ThreadLocal __thread
#endif

namespace ouzel
{
    extern uint32_t getCPUCount();

    class Thread final
    {
    public:
        class ID
        {
            friend Thread;
        public:
            bool operator==(const ID& other)
            {
#if defined(_WIN32)
                return threadId == other.threadId;
#else
                return pthread_equal(thread, other.thread) != 0;
#endif
            }

            bool operator!=(const ID& other)
            {
#if defined(_WIN32)
                return threadId != other.threadId;
#else
                return pthread_equal(thread, other.thread) == 0;
#endif
            }

        protected:
#if defined(_WIN32)
            ID(DWORD id):
                threadId(id)
#else
            ID(pthread_t t):
                thread(t)
#endif
            {
            }
        private:
#if defined(_WIN32)
            DWORD threadId = 0;
#else
            pthread_t thread = 0;
#endif
        };

        Thread() {}
        explicit Thread(const std::function<void()>& function, const std::string& name = "");

        ~Thread();

        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

        Thread(Thread&& other);
        Thread& operator=(Thread&& other);

        void join();

        int32_t getPriority() const;
        void setPriority(int32_t priority);

        inline bool isJoinable() const
        {
#if defined(_WIN32)
            return handle != nullptr;
#else
            return initialized;
#endif
        }

        ID getId() const
        {
#if defined(_WIN32)
            return threadId;
#else
            return thread;
#endif
        }

        static ID getCurrentThreadId()
        {
#if defined(_WIN32)
            return GetCurrentThreadId();
#else
            return pthread_self();
#endif
        }

        static void setCurrentThreadName(const std::string& name);

        struct State
        {
            std::function<void()> function;
            std::string name;
        };

    private:
        std::unique_ptr<State> state;

#if defined(_WIN32)
        HANDLE handle = nullptr;
        DWORD threadId = 0;
#else
        pthread_t thread;
        bool initialized = false;
#endif
    };
}
