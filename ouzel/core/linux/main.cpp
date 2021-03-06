// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include <cstdlib>
#include "EngineLinux.hpp"
#include "utils/Log.hpp"

int main(int argc, char* argv[])
{
    try
    {
        ouzel::EngineLinux engine(argc, argv);
        engine.run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        ouzel::Log(ouzel::Log::Level::ERR) << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        ouzel::Log(ouzel::Log::Level::ERR) << "Unknown error occurred";
        return EXIT_FAILURE;
    }
}
