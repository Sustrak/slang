#include "Server.h"

#include "Log.h"
#include "RPC.h"
#include <csignal>

using namespace slang::rpc;

void Server::startServer() {
    Log::low("Starting the LSP Server\n");
    if (status != ServerStatus::STOPPED)
        throw ServerAlreadyStartedException();

    status = ServerStatus::INITIALIZING;
    initializeServer();

    while (status == ServerStatus::RUNNING) {
    }
}

void Server::initializeServer() {
    Log::debug("Initializing server\n");
    while (status == ServerStatus::INITIALIZING) {
        // Get the header of the RPC to know how many bytes we need to consume from the input,
        // read as json the number of bytes specified by the header and construct the requestMessage

        auto request = RequestMessage(readJSON(LSPHeader::fromStdin().contentLength));

        // TODO: If the request is not of method initialize send an error to the client, for now
        // crash the server :(
        assert(request.method == "initialize");

        // Interpret the params as InitializeParams, this is safe since the RequestMessage
        // constructor will create an InitializeParams object from the json request
        const auto& initializeParams = request.params[0]->as<InitializeParams>();

        // Print the client info if it has been provided
        if (initializeParams.clientInfo) {
            Log::low("Client {} {}\n"sv, initializeParams.clientInfo->name,
                     initializeParams.clientInfo->version);
        }

        // If the process id that we have been provided is not alive, exit the server
        if (initializeParams.processId != -1) {
            if (kill(initializeParams.processId, 0) != 0) {
                // TODO: parent process do not exists exit the server
            }
        }
        // TODO: Use the locale

        // FIXME: This should be moved
        clientCapabilities = initializeParams.capabilities;
        workspaceFolders = initializeParams.workspaceFolders;

        if (initializeParams.traceValue)
            traceValue = initializeParams.traceValue.value();

        if (initializeParams.rootPath) {
            Log::warning("initializeParams has been deprecated in favor of workspaceFolders\n");
            workspaceFolders.emplace_back(initializeParams.rootPath.value(), "");
        }

        if (initializeParams.rootUri) {
            Log::warning("rootUri has been deprecated in favor of workspaceFolders\n");
            workspaceFolders.emplace_back(initializeParams.rootUri.value(), "");
        }

        // Generate the result for the client
        auto result = InitializeResult();

        // Setup how we want the client to notify us when a file has been modified
        result.capabilities.textDocumentSync.openClose = true;
        result.capabilities.textDocumentSync.change = slang::rpc::TextDocumentSyncKind::Full;

        // Setup characters that triggers autocompletion on the client
        result.capabilities.completionProvider.triggerCharacters = {"."};
        result.capabilities.completionProvider.resolveProvider = false;
        result.capabilities.completionProvider.completionItem = {false};

        auto response = ResponseMessage(1, std::make_unique<InitializeResult>(result));
        auto JSONResponse = response.toJSON().dump();

        LSPHeader header(JSONResponse.size());

        Log::debug("{}{}\n", header.toString(), JSONResponse);
        fmt::print("{}{}", header.toString(), JSONResponse);

        status = ServerStatus::RUNNING;
        Log::low("Served initialized :D\n");
    }
}

json Server::readJSON(size_t size) {
    // Read size bytes from stdin and create a json
    std::vector<uint8_t> buff(size);
    for (int i = 0; i < size; i++)
        buff[i] = getchar_unlocked();

    auto j = json::parse(std::move(buff));

    Log::debug("{}\n", j.dump());

    return std::move(j);
}
