#include "Log.h"
#include "RPC.h"
#include "Server.h"
#include <iostream>

#define DEBUG_GDB                                                                \
    if (auto* env = std::getenv("DEBUG_GDB"); env && std::string(env) == "ON") { \
        volatile int _done = 0;                                                  \
        while (!_done)                                                           \
            sleep(1);                                                            \
    }

int main(int argc, char** argv) {
    DEBUG_GDB

    Log::setVerbosity(Log::High);
    Log::useColors(false);

    auto server = Server();
    try {
        server.startServer();
    }
    catch (const slang::rpc::NoContentLengthException&) {
        std::cerr << "NoContentLengthException" << std::endl;
    }
    catch (const slang::rpc::UnknownLSPMethod& e) {
        std::cerr << "UnknownLSPMethod: " << e.what() << std::endl;
    }

    return 0;
}