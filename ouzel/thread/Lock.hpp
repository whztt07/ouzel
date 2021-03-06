// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include "thread/Mutex.hpp"

namespace ouzel
{
    class Lock final
    {
        friend Condition;
    public:
        explicit Lock(Mutex& initMutex):
            mutex(&initMutex)
        {
            mutex->lock();
        }

        ~Lock()
        {
            if (mutex) mutex->unlock();
        }

        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;

        Lock(Lock&& other)
        {
            mutex = other.mutex;
            other.mutex = nullptr;
        }

        Lock& operator=(Lock&& other)
        {
            if (mutex) mutex->unlock();
            mutex = other.mutex;
            other.mutex = nullptr;

            return *this;
        }

    private:
        Mutex* mutex = nullptr;
    };
}
