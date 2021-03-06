// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#ifdef _WIN32
#include <winsock.h>
#endif

namespace ouzel
{
    namespace network
    {
        class Client final
        {
        public:
            ~Client();

            Client(const Client&) = delete;
            Client& operator=(const Client&) = delete;

            Client(Client&& other);
            Client& operator=(Client&& other);

            void disconnect();

        private:
#ifdef _WIN32
            SOCKET endpoint = INVALID_SOCKET;
#else
            int endpoint = -1;
#endif
        };
    } // namespace network
} // namespace ouzel
