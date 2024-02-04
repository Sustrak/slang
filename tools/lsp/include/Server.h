#pragma once

#include "RPC.h"

using json = nlohmann::json;

class ServerAlreadyStartedException : public std::exception {};

class Server {
public:
    void startServer();

private:
    enum class ServerStatus { STOPPED, INITIALIZING, RUNNING, EXITED };
    ServerStatus status;
    std::vector<slang::rpc::WorkspaceFolder> workspaceFolders;
    slang::rpc::ClientCapabilities clientCapabilities;
    slang::rpc::TraceValue traceValue = slang::rpc::TraceValue::OFF;

    void initializeServer();

    /// Reads from stdin the number of characters `size` indicate. Then parses the read characters
    /// as JSON
    static json readJSON(size_t size);

    /// Sends the response through the channel server and client communicate
    static void sendResponse(const slang::rpc::ResponseMessage& response);
};
