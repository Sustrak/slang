#include "RPC.h"

#include "Log.h"
#include <fmt/format.h>
#include <iostream>
#include <string>

using namespace slang::rpc;

#define GET_VALUE(_type, _name, _json) getValue<_type>(_name, _json, #_name);
#define GET_BITMASK(_type, _name, _json, _func) getBitmask<_type>(_name, _json, #_name, _func);
#define GET_ENUM(_type, _name, _json, _func) getEnum<_type>(_name, _json, #_name, _func);
#define GET_VECTOR(_name, _json) getVector(_name, _json, #_name);

#define TO_JSON(_name, _json) _name.toJSON(_json[#_name]);
#define TO_JSON_OPT(_name, _json) _name->toJSON(_json[#_name]);

template<typename T>
inline void getValue(auto& var, const json& j, const std::string& name) {
    if constexpr (std::is_same_v<T, bool>) {
        if (j.contains(name))
            if (const auto& v = j[name]; v.is_boolean())
                var = v;
    }
    else if constexpr (std::is_same_v<T, string>) {
        if (j.contains(name)) {
            if (const auto& v = j[name]; v.is_string())
                var = v.template get<string>();
            else if (v.is_number_integer())
                var = fmt::format("{}", v.template get<integer>());
            else if (v.is_number_unsigned())
                var = fmt::format("{}", v.template get<uinteger>());
        }
    }
    else if constexpr (std::is_same_v<T, uinteger> || std::is_same_v<T, integer>) {
        if (j.contains(name))
            if (const auto& v = j[name]; v.is_number())
                var = v.template get<T>();
    }
}

template<typename T>
inline void getEnum(auto& var, const json& j, const std::string& name, auto toString) {
    if constexpr (std::is_same_v<T, string>) {
        if (j.contains(name))
            if (const auto& v = j[name]; v.is_string())
                var = toString(v.template get<string>());
    }
    else if constexpr (std::is_same_v<T, uinteger>) {
        if (j.contains(name))
            if (const auto& v = j[name]; v.is_number())
                var = toString(v.template get<uinteger>());
    }
}

template<typename T>
inline void getBitmask(auto& var, const json& j, const std::string& name, auto toString) {
    if (j.contains(name))
        if (const auto& v = j[name]; v.is_array())
            for (const auto& op : v)
                var |= toString(op.template get<T>());
}

inline void getVector(auto& vec, const json& j, const std::string& name) {
    if (j.contains(name))
        if (const auto& v = j[name]; v.is_array())
            for (const auto& val : v)
                vec.emplace_back(val.template get<string>());
}

inline void getEnumVector(auto& vec, const json& j, const std::string& name, auto toString) {
    if (j.contains(name))
        if (const auto& v = j[name]; v.is_array())
            for (const auto& val : v)
                vec.emplace_back(toString(val.template get<string>()));
}

namespace slang::rpc {
RPCMethod RPCMethodFromString(std::string_view str) {
    if (str == "initialize")
        return RPCMethod::Initialize;
    if (str == "initialized")
        return RPCMethod::Initialized;
    if (str == "shutdown")
        return RPCMethod::Shutdown;
    if (str == "exit")
        return RPCMethod::Exit;
    throw UnknownEnumVariant(fmt::format("{} is not a valid RPCMethod variant", str));
}

ResourceOperationKind ResourceOperationKindFromString(std::string_view str) {
    if (str == "create")
        return ResourceOperationKind::Create;
    if (str == "rename")
        return ResourceOperationKind::Rename;
    if (str == "delete")
        return ResourceOperationKind::Delete;
    throw UnknownEnumVariant(fmt::format("{} is not a valid ResourceOperationKind variant", str));
}

FailureHandlingKind FailureHandlingKindFromString(std::string_view str) {
    if (str == "abort")
        return FailureHandlingKind::Abort;
    if (str == "transactional")
        return FailureHandlingKind::Transactional;
    if (str == "textOnlyTransactional")
        return FailureHandlingKind::TextOnlyTransactional;
    if (str == "undo")
        return FailureHandlingKind::Undo;
    throw UnknownEnumVariant(fmt::format("{} is not a valid FailureHandlingKind variant", str));
}

SymbolKindEnum SymbolKindEnumFromUInteger(uinteger v) {
    // clang-format off
    switch (v) {
        case 1: return SymbolKindEnum::Field;
        case 2: return SymbolKindEnum::Module;
        case 3: return SymbolKindEnum::Namespace;
        case 4: return SymbolKindEnum::Package;
        case 5: return SymbolKindEnum::Class;
        case 6: return SymbolKindEnum::Method;
        case 7: return SymbolKindEnum::Property;
        case 8: return SymbolKindEnum::Field;
        case 9: return SymbolKindEnum::Constructor;
        case 10: return SymbolKindEnum::Enum;
        case 11: return SymbolKindEnum::Interface;
        case 12: return SymbolKindEnum::Function;
        case 13: return SymbolKindEnum::Variable;
        case 14: return SymbolKindEnum::Constant;
        case 15: return SymbolKindEnum::String;
        case 16: return SymbolKindEnum::Number;
        case 17: return SymbolKindEnum::Boolean;
        case 18: return SymbolKindEnum::Array;
        case 19: return SymbolKindEnum::Object;
        case 20: return SymbolKindEnum::Key;
        case 21: return SymbolKindEnum::Null;
        case 22: return SymbolKindEnum::EnumMember;
        case 23: return SymbolKindEnum::Struct;
        case 24: return SymbolKindEnum::Event;
        case 25: return SymbolKindEnum::Operator;
        case 26: return SymbolKindEnum::TypeParameter;
        default:
            throw UnknownEnumVariant(fmt::format("{} is not a valid SymbolKind variant", v));
    }
    // clang-format on
}

SymbolTag SymbolTagFromUInteger(uinteger v) {
    if (v == 1) {
        return SymbolTag::Deprecated;
    }
    else {
        throw UnknownEnumVariant(fmt::format("{} is not a valid SymbolTag variant", v));
    }
}

CompletionItemKindEnum CompletionItemKindEnumFromUInteger(uinteger v) {
    // clang-format off
    switch (v) {
        case 1: return CompletionItemKindEnum::Text;
        case 2: return CompletionItemKindEnum::Method;
        case 3: return CompletionItemKindEnum::Function;
        case 4: return CompletionItemKindEnum::Constructor;
        case 5: return CompletionItemKindEnum::Field;
        case 6: return CompletionItemKindEnum::Variable;
        case 7: return CompletionItemKindEnum::Class;
        case 8: return CompletionItemKindEnum::Interface;
        case 9: return CompletionItemKindEnum::Module;
        case 10: return CompletionItemKindEnum::Property;
        case 11: return CompletionItemKindEnum::Unit;
        case 12: return CompletionItemKindEnum::Value;
        case 13: return CompletionItemKindEnum::Enum;
        case 14: return CompletionItemKindEnum::Keyword;
        case 15: return CompletionItemKindEnum::Snippet;
        case 16: return CompletionItemKindEnum::Color;
        case 17: return CompletionItemKindEnum::File;
        case 18: return CompletionItemKindEnum::Reference;
        case 19: return CompletionItemKindEnum::Folder;
        case 20: return CompletionItemKindEnum::EnumMember;
        case 21: return CompletionItemKindEnum::Constant;
        case 22: return CompletionItemKindEnum::Struct;
        case 23: return CompletionItemKindEnum::Event;
        case 24: return CompletionItemKindEnum::Operator;
        case 25: return CompletionItemKindEnum::TypeParameter;
        default:
            throw UnknownEnumVariant(fmt::format("{} is not a valid CompletionItemKind variant", v));
    }
    // clang-format on
}

MarkupKind MarkupKindFromString(std::string_view str) {
    if (str == "plaintext")
        return MarkupKind::PlainText;
    if (str == "markdown")
        return MarkupKind::Markdown;
    throw UnknownEnumVariant(fmt::format("{} is not a valid MarkupKind variant", str));
}

CompletionItemTag CompletionItemTagFromUInteger(uinteger v) {
    if (v == 1)
        return CompletionItemTag::Deprecated;
    throw UnknownEnumVariant(fmt::format("{} is not a valid CompletionItemTag variant", v));
}

InsertTextMode InsertTextModeFromUInteger(uinteger v) {
    if (v == 1)
        return InsertTextMode::AsIs;
    if (v == 2)
        return InsertTextMode::AdjustIndentation;
    throw UnknownEnumVariant(fmt::format("{} is not a valid InsertTextMode variant", v));
}

CodeActionKind CodeActionKindFromString(std::string_view str) {
    if (str == "")
        return CodeActionKind::Empty;
    if (str == "quickfix")
        return CodeActionKind::QuickFix;
    if (str == "refactor")
        return CodeActionKind::Refactor;
    if (str == "refactor.extract")
        return CodeActionKind::RefactorExtract;
    if (str == "refactor.inline")
        return CodeActionKind::RefactorInline;
    if (str == "refactor.rewrite")
        return CodeActionKind::RefactorRewrite;
    if (str == "source")
        return CodeActionKind::Source;
    if (str == "source.organizeImports")
        return CodeActionKind::SourceOrganizeImports;
    if (str == "source.fixAll")
        return CodeActionKind::SourceFixAll;
    throw UnknownEnumVariant(fmt::format("{} is not a valid CodeActionKind variant", str));
}

PrepareSupportDefaultBehavior PrepareSupportDefaultBehaviorFromUInteger(uinteger v) {
    if (v == 1)
        return PrepareSupportDefaultBehavior::Identifier;
    throw UnknownEnumVariant(
        fmt::format("{} is not a valid PrepareSupportDefaultBehavior  variant", v));
}

DiagnosticTag DiagnosticTagFromUInteger(uinteger v) {
    if (v == 1)
        return DiagnosticTag::Unnecessary;
    if (v == 2)
        return DiagnosticTag::Deprecated;
    throw UnknownEnumVariant(fmt::format("{} is not a valid DiagnosticTag variant", v));
}

FoldingRangeKind FoldingRangeKindFromString(std::string_view str) {
    if (str == "comment")
        return FoldingRangeKind::Comment;
    if (str == "imports")
        return FoldingRangeKind::Imports;
    if (str == "region")
        return FoldingRangeKind::Region;
    throw UnknownEnumVariant(fmt::format("{} is not a valid FoldingRangeKind variant", str));
}

TokenFormat TokenFormatFromString(std::string_view str) {
    if (str == "relative")
        return TokenFormat::Relative;
    throw UnknownEnumVariant(fmt::format("{} is not a valid TokenFormat variant", str));
}

TraceValue TraceValueFromString(std::string_view str) {
    if (str == "off")
        return TraceValue::OFF;
    if (str == "messages")
        return TraceValue::MESSAGES;
    if (str == "verbose")
        return TraceValue::VERBOSE;
    throw UnknownEnumVariant(fmt::format("{} is not a valid TraceValue variant", str));
}

integer ErrorCodeToInteger(ErrorCode err) {
    switch (err) {
        case ErrorCode::ParseError:
            return -32700;
        case ErrorCode::InvalidRequest:
            return -32600;
        case ErrorCode::MethodNotFound:
            return -32601;
        case ErrorCode::InvalidParams:
            return -32602;
        case ErrorCode::InternalError:
            return -32603;
        case ErrorCode::jsonrpcReservedErrorRangeStart:
            return -32099;
        case ErrorCode::serverErrorStart:
            return -32099;
        case ErrorCode::ServerNotInitialized:
            return -32002;
        case ErrorCode::UnknownErrorCode:
            return -32001;
        case ErrorCode::jsonrpcReservedErrorRangeEnd:
            return -32000;
        case ErrorCode::serverErrorEnd:
            return -32000;
        case ErrorCode::lspReservedErrorRangeStart:
            return -32899;
        case ErrorCode::RequestFailed:
            return -32803;
        case ErrorCode::ServerCancelled:
            return -32802;
        case ErrorCode::ContentModified:
            return -32801;
        case ErrorCode::RequestCancelled:
            return -32800;
        case ErrorCode::lspReservedErrorRangeEnd:
            return -32800;
    }
}

std::string PositionEncodingKindToString(PositionEncodingKind kind) {
    switch (kind) {
        case PositionEncodingKind::UTF8:
            return "utf-8";
        case PositionEncodingKind::UTF16:
            return "utf-16";
        case PositionEncodingKind::UTF32:
            return "utf-32";
    }
}

PositionEncodingKind PositionEncodingKindFromString(std::string_view str) {
    if (str == "utf-8")
        return PositionEncodingKind::UTF8;
    if (str == "utf-16")
        return PositionEncodingKind::UTF16;
    if (str == "utf-32")
        return PositionEncodingKind::UTF32;
    throw UnknownEnumVariant(fmt::format("{} is not a valid PositionEncodingKind variant", str));
}

uinteger TextDocumentSyncKindToUInteger(TextDocumentSyncKind kind) {
    switch (kind) {
        case TextDocumentSyncKind::None:
            return 0;
        case TextDocumentSyncKind::Full:
            return 1;
        case TextDocumentSyncKind::Incremental:
            return 2;
    }
}

std::string FileOperationPatternKindToString(FileOperationPatternKind kind) {
    switch (kind) {
        case FileOperationPatternKind::File:
            return "file";
        case FileOperationPatternKind::Folder:
            return "folder";
    }
}

LSPHeader LSPHeader::fromStdin() {
    // LSP Headers look like:
    //    <header_name>: <header_value>\r\n
    //    <header_name>: <header_value>\r\n
    //    \r\n
    // Where the Content-Length field is mandatory

    bool contentLengthParsed = false;
    auto header = LSPHeader();
    std::string line;
    std::getline(std::cin, line);
    while (line != "\r") {
        if (line.substr(0, CONTENT_TYPE.length()) == CONTENT_TYPE) {
            header.contentType = line.substr(CONTENT_TYPE.length(),
                                             line.length() - END_LINE.length());
        }
        else if (line.substr(0, CONTENT_LENGTH.length()) == CONTENT_LENGTH) {
            contentLengthParsed = true;
            header.contentLength = std::stoul(
                line.substr(CONTENT_LENGTH.length(), line.length() - END_LINE.length()));
        }
        std::getline(std::cin, line);
    }

    if (!contentLengthParsed)
        throw NoContentLengthException();

    if (header.contentType != DEFAULT_CONTENT_TYPE)
        throw NoDefaultContentType();

    Log::debug("Header parsed: Content-Length: {} Content-Type: {}\n", header.contentLength,
               header.contentType);

    return header;
}

std::string LSPHeader::toString() const {
    // LSP Headers look like:
    //    <header_name>: <header_value>\r\n
    //    <header_name>: <header_value>\r\n
    return fmt::format("{}{}{}{}{}{}{}", CONTENT_LENGTH, contentLength, END_LINE_CRLF, CONTENT_TYPE,
                       contentType, END_LINE_CRLF, END_LINE_CRLF);
}

WorkspaceEditClientCapabilities::WorkspaceEditClientCapabilities(const json& j) {
    GET_VALUE(bool, documentChanges, j)
    GET_BITMASK(string, resourceOperations, j, ResourceOperationKindFromString)
    GET_ENUM(string, failureHandlingKind, j, FailureHandlingKindFromString)
    GET_VALUE(bool, normalizesLineEndings, j)
    if (j.contains("changeAnnotationSupport"sv)) {
        if (const auto& v = j["changeAnnotationSupport"]; v.is_object())
            if (v.contains("groupsOnLabel"sv))
                if (const auto& vv = v["groupsOnLabel"]; vv.is_boolean())
                    changeAnnotationSupport = ChangeAnnotationSupport{v};
    }
}

DidChangeWatchedFilesClientCapabilities::DidChangeWatchedFilesClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    if (j.contains("relativePatternSupport"sv))
        if (const auto& v = j["relativePatternSupport"]; v.is_boolean())
            relativePatternSupport = v;
}

WorkspaceSymbolClientCapabilities::WorkspaceSymbolClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    if (j.contains("symbolKind"sv)) {
        if (const auto& v = j["symbolKind"]; v.is_object()) {
            bitmask<SymbolKindEnum> valueSet;
            GET_BITMASK(uinteger, valueSet, v, SymbolKindEnumFromUInteger)
            symbolKind = {valueSet};
        }
    }
    if (j.contains("tagSupport"sv)) {
        if (const auto& v = j["tagSupport"]; v.is_object()) {
            bitmask<SymbolTag> valueSet;
            GET_BITMASK(uinteger, valueSet, v, SymbolTagFromUInteger)
            tagSupport = {valueSet};
        }
    }
    if (j.contains("resolveSupport"sv)) {
        if (const auto& v = j["resolveSupport"]; v.is_object()) {
            std::vector<string> properties;
            GET_VECTOR(properties, v["properties"])
            resolveSupport = {std::move(properties)};
        }
    }
}

TextDocumentSyncClientCapabilities::TextDocumentSyncClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    GET_VALUE(bool, willSave, j);
    GET_VALUE(bool, willSaveWaitUntil, j)
    GET_VALUE(bool, didSave, j)
}

CompletionClientCapabilities::CompletionClientCapabilities(const json& j) : DynamicRegistration(j) {
    if (j.contains("completionItem"sv)) {
        if (const auto& v = j["completionItem"]; v.is_object()) {
            auto compItem = CompletionClientCapabilities::CompletionItem();
            getValue<bool>(compItem.snippetSupport, v, "snippetSupport");
            getValue<bool>(compItem.commitCharactersSupport, v, "commitCharactersSupport");
            getBitmask<string>(compItem.documentationFormat, v, "documentationFormat",
                               MarkupKindFromString);
            if (v.contains("documentationFormat"))
                compItem.preferredDocumentationFormat = MarkupKindFromString(
                    v["documentationFormat"][0].get<string>());
            getValue<bool>(compItem.deprecatedSupport, v, "deprecatedSupport");
            getValue<bool>(compItem.preselectSupport, v, "preselectSupport");
            if (v.contains("tagSupport"sv)) {
                if (const auto& t = v["tagSupport"]; t.is_object()) {
                    bitmask<CompletionItemTag> valueSet;
                    GET_BITMASK(uinteger, valueSet, t, CompletionItemTagFromUInteger)
                    compItem.tagSupport = {valueSet};
                }
            }
            getValue<bool>(compItem.insertReplaceSupport, v, "insertReplaceSupport");
            if (v.contains("resolveSupport"sv)) {
                if (const auto& r = v["resolveSupport"]; r.is_object()) {
                    std::vector<string> properties;
                    GET_VECTOR(properties, r)
                    compItem.resolveSupport = {std::move(properties)};
                }
            }
            if (v.contains("insertModeSupport"sv)) {
                if (const auto& i = v["insertModeSupport"]; i.is_object()) {
                    bitmask<InsertTextMode> valueSet;
                    GET_BITMASK(uinteger, valueSet, i, InsertTextModeFromUInteger)
                    compItem.insertModeSupport = {valueSet};
                }
            }
            getValue<bool>(compItem.labelDetailsSupport, v, "labelDetailsSupport");
            completionItem = std::move(compItem);
        }
    }
    if (j.contains("completionItemKind"sv)) {
        if (const auto& v = j["completionItemKind"]; v.is_object()) {
            bitmask<CompletionItemKindEnum> valueSet;
            GET_BITMASK(uinteger, valueSet, v, CompletionItemKindEnumFromUInteger)
            completionItemKind = {valueSet};
        }
    }
    GET_VALUE(bool, contextSupport, j)
    GET_ENUM(uinteger, insertTextMode, j, InsertTextModeFromUInteger)
    if (j.contains("completionList"sv)) {
        if (const auto& v = j["completionList"]; v.is_object()) {
            std::vector<string> itemDefaults;
            GET_VECTOR(itemDefaults, v)
            completionList = {std::move(itemDefaults)};
        }
    }
}

HoverClientCapabilities::HoverClientCapabilities(const json& j) : DynamicRegistration(j) {
    if (j.contains("contentFormat"sv))
        if (const auto& v = j["contentFormat"]; v.is_array()) {
            GET_BITMASK(string, contentFormat, j, MarkupKindFromString)
            preferredContentFormat = MarkupKindFromString(j["contentFormat"][0].get<string>());
        }
}

SignatureHelpClientCapabilities::SignatureHelpClientCapabilities(const slang::rpc::json& j) :
    DynamicRegistration(j) {
    if (j.contains("signatureInformation"sv)) {
        if (const auto& v = j["signatureInformation"]; v.is_object()) {
            auto signature = SignatureHelpClientCapabilities::SignatureInformation();
            getBitmask<string>(signature.documentationFormat, v, "documentationFormat",
                               MarkupKindFromString);
            if (v.contains("documentationFormat"sv))
                if (const auto& d = v["documentationFormat"]; d.is_array())
                    signature.preferredDocumentationFormat = MarkupKindFromString(
                        d[0].get<string>());
            if (v.contains("parameterInformation"sv)) {
                if (const auto& p = v["parameterInformation"]; p.is_object()) {
                    bool b;
                    GET_VALUE(bool, b, p)
                    signature.parameterInformation = {b};
                }
            }
            getValue<bool>(signature.activeParameterSupport, v, "activeParameterSupport");
            signatureInformation = signature;
        }
    }
    GET_VALUE(bool, contextSupport, j)
}

DeclarationClientCapabilities::DeclarationClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    GET_VALUE(bool, linkSupport, j);
}

DefinitionClientCapabilities::DefinitionClientCapabilities(const json& j) : DynamicRegistration(j) {
    GET_VALUE(bool, linkSupport, j);
}

TypeDefinitionClientCapabilities::TypeDefinitionClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    GET_VALUE(bool, linkSupport, j);
}

ImplementationClientCapabilities::ImplementationClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    GET_VALUE(bool, linkSupport, j);
}

DocumentSymbolClientCapabilities::DocumentSymbolClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    if (j.contains("symbolKind"sv)) {
        if (const auto& v = j["symbolKind"]; v.is_object()) {
            auto symbol = DocumentSymbolClientCapabilities::SymbolKind();
            getBitmask<uinteger>(symbol.valueSet, v, "valueSet", SymbolKindEnumFromUInteger);
            symbolKind = symbol;
        }
    }
    GET_VALUE(bool, hierarchicalDocumentSymbolSupport, j)
    if (j.contains("tagSupport"sv)) {
        if (const auto& v = j["tagSupport"]; v.is_object()) {
            auto tag = DocumentSymbolClientCapabilities::TagSupport();
            getBitmask<uinteger>(tag.valueSet, v, "valueSet", SymbolTagFromUInteger);
            tagSupport = tag;
        }
    }
    GET_VALUE(bool, labelSupport, j)
}

CodeActionClientCapabilities::CodeActionClientCapabilities(const json& j) : DynamicRegistration(j) {
    if (j.contains("codeActionLiteralSupport"sv)) {
        if (const auto& v = j["codeActionLiteralSupport"]; v.is_object()) {
            auto code = CodeActionClientCapabilities::CodeActionLiteralSupport();
            if (v.contains("codeActionKind"sv))
                if (const auto& c = v["codeActionKind"]; c.is_object())
                    getBitmask<string>(code.codeActionKind, c, "codeActionKind",
                                       CodeActionKindFromString);
            codeActionLiteralSupport = code;
        }
    }
    GET_VALUE(bool, isPreferredSupport, j)
    GET_VALUE(bool, disabledSupport, j)
    GET_VALUE(bool, dataSupport, j)
    if (j.contains("resolveSupport"sv)) {
        if (const auto& v = j["resolveSupport"]; v.is_object()) {
            auto resolve = CodeActionClientCapabilities::ResolveSupport();
            getVector(resolve.properties, v, "properties");
            resolveSupport = resolve;
        }
    }
    GET_VALUE(bool, honorsChangeAnnotations, j)
}

DocumentLinkClientCapabilities::DocumentLinkClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    GET_VALUE(bool, tooltipSupport, j);
}

RenameClientCapabilities::RenameClientCapabilities(const json& j) : DynamicRegistration(j) {
    GET_VALUE(bool, prepareSupport, j)
    if (j.contains("prepareSupportDefaultBehavior"sv))
        if (const auto& v = j["prepareSupportDefaultBehavior"]; v.is_number())
            prepareSupportDefaultBehavior = PrepareSupportDefaultBehaviorFromUInteger(
                v.get<uinteger>());
    GET_VALUE(bool, honorsChangeAnnotations, j)
}

PublishDiagnosticsClientCapabilities::PublishDiagnosticsClientCapabilities(const json& j) {
    GET_VALUE(bool, relatedInformation, j);
    if (j.contains("tagSupport"sv))
        getBitmask<uinteger>(tagSupport, j["tagSupport"], "valueSet", DiagnosticTagFromUInteger);
    GET_VALUE(bool, versionSupport, j)
    GET_VALUE(bool, codeDescriptionSupport, j)
    GET_VALUE(bool, dataSupport, j)
}

FoldingRangeClientCapabilities::FoldingRangeClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    GET_VALUE(uinteger, rangeLimit, j)
    GET_VALUE(bool, lineFoldingOnly, j)
    GET_BITMASK(string, foldingRangeKind, j, FoldingRangeKindFromString)
    if (j.contains("foldingRange"sv))
        if (const auto& v = j["foldingRange"]; v.is_object()) {
            auto folding = FoldingRangeClientCapabilities::FoldingRange();
            getValue<bool>(folding.collapsedText, v, "collapsedText");
            foldingRange = folding;
        }
}

SemanticTokensClientCapabilities::SemanticTokensClientCapabilities(const json& j) :
    DynamicRegistration(j) {
    if (j.contains("requests"sv)) {
        if (const auto& v = j["requests"]; v.is_object()) {
            auto req = SemanticTokensClientCapabilities::Requests();
            getValue<bool>(req.range, v, "range");
            if (v.contains("full"sv)) {
                if (const auto& f = v["full"]; f.is_boolean())
                    req.full = f;
                else if (f.is_object())
                    getValue<bool>(req.fullDelta, f, "fullDelta");
            }
        }
    }
    GET_VECTOR(tokenTypes, j)
    GET_VECTOR(tokenModifiers, j)
    GET_BITMASK(string, formats, j, TokenFormatFromString)
    GET_VALUE(bool, overlappingTokenSupport, j)
    GET_VALUE(bool, multilineTokenSupport, j)
    GET_VALUE(bool, serverCancelSupport, j)
    if (j.contains("augmentsSyntaxTokens"sv))
        if (const auto& v = j["augmentsSyntaxTokens"]; v.is_boolean())
            augmentsSyntaxTokens = v;
}

InlayHintClientCapabilities::InlayHintClientCapabilities(const json& j) : DynamicRegistration(j) {
    if (j.contains("resolveSupport")) {
        if (const auto& v = j["resolveSupport"]; v.is_object()) {
            auto resolve = InlayHintClientCapabilities::ResolveSupport();
            getVector(resolve.properties, v, "properties");
            resolveSupport = std::move(resolve);
        }
    }
}

DiagnosticClientCapabilities::DiagnosticClientCapabilities(const json& j) : DynamicRegistration(j) {
    GET_VALUE(bool, relatedDocumentSupport, j);
}

TextDocumentClientCapabilities::TextDocumentClientCapabilities(const json& j) {
#define GET(_name, _class)                            \
    if (j.contains(#_name##sv))                       \
        if (const auto& v = j[#_name]; v.is_object()) \
            _name = _class(v);

    GET(synchronization, TextDocumentSyncClientCapabilities)
    GET(completion, CompletionClientCapabilities)
    GET(hover, HoverClientCapabilities)
    GET(signatureHelp, SignatureHelpClientCapabilities)
    GET(declaration, DeclarationClientCapabilities)
    GET(definition, DefinitionClientCapabilities)
    GET(typeDefinition, TypeDefinitionClientCapabilities)
    GET(implementation, ImplementationClientCapabilities)
    GET(references, ReferenceClientCapabilities)
    GET(documentHighlight, DocumentHighlightClientCapabilities)
    GET(documentSymbol, DocumentSymbolClientCapabilities)
    GET(codeAction, CodeActionClientCapabilities)
    GET(codeLens, CodeLensClientCapabilities)
    GET(documentLink, DocumentLinkClientCapabilities)
    GET(colorProvider, DocumentColorClientCapabilities)
    GET(formatting, DocumentFormattingClientCapabilities)
    GET(rangeFormatting, DocumentRangeFormattingClientCapabilities)
    GET(onTypeFormatting, DocumentOnTypeFormattingClientCapabilities)
    GET(rename, RenameClientCapabilities)
    GET(publishDiagnostics, PublishDiagnosticsClientCapabilities)
    GET(foldingRange, FoldingRangeClientCapabilities)
    GET(selectionRange, SelectionRangeClientCapabilities)
    GET(linkedEditingRange, LinkedEditingRangeClientCapabilities)
    GET(callHierarchy, CallHierarchyClientCapabilities)
    GET(semanticTokens, SemanticTokensClientCapabilities)
    GET(moniker, MonikerClientCapabilities)
    GET(typeHierarchy, TypeHierarchyClientCapabilities)
    GET(inlineValue, InlineValueClientCapabilities)
    GET(inlayHint, InlayHintClientCapabilities)
    GET(diagnostic, DiagnosticClientCapabilities)
#undef GET
}

ClientCapabilities::ClientCapabilities(const json& j) {
#define GET(_name, _json, _class)                           \
    if (_json.contains(#_name##sv))                         \
        if (const auto& _v = _json[#_name]; _v.is_object()) \
            _name = _class(_v);

#define GET_NAME(_var, _name, _json, _class)                \
    if (_json.contains(#_name##sv))                         \
        if (const auto& _v = _json[#_name]; _v.is_object()) \
            _var = _class(_v);

    if (j.contains("workspace"sv)) {
        if (const auto& v = j["workspace"]; v.is_object()) {
            auto w = ClientCapabilities::Workspace();
            getValue<bool>(w.applyEdit, v, "applyEdit");
            GET_NAME(w.workspaceEditCapabilities, workspaceEditCapabilities, v,
                     WorkspaceEditClientCapabilities)
            GET_NAME(w.didChangeConfiguration, didChangeConfiguration, v,
                     DidChangeConfigurationClientCapabilities)
            GET_NAME(w.didChangeWatchedFiles, didChangeWatchedFiles, v,
                     DidChangeWatchedFilesClientCapabilities)
            GET_NAME(w.symbol, symbol, v, WorkspaceSymbolClientCapabilities)
            GET_NAME(w.executeCommand, executeCommand, v, ExecuteCommandClientCapabilities)
            getValue<bool>(w.workspaceFolders, v, "workspaceFolders");
            getValue<bool>(w.configuration, v, "configuration");
            GET_NAME(w.semanticTokens, semanticTokens, v, SemanticTokensWorkspaceClientCapabilities)
            if (v.contains("fileOperations"sv)) {
                if (const auto& f = v["fileOperations"]; f.is_object()) {
                    auto file = ClientCapabilities::Workspace::FileOperations();
                    getValue<bool>(file.dynamicRegistration, f, "dynamicRegistration");
                    getValue<bool>(file.didCreate, f, "didCreate");
                    getValue<bool>(file.willCreate, f, "willCreate");
                    getValue<bool>(file.didRename, f, "didRename");
                    getValue<bool>(file.willRename, f, "willRename");
                    getValue<bool>(file.didDelete, f, "didDelete");
                    getValue<bool>(file.willDelete, f, "willDelete");
                    w.fileOperations = file;
                }
            }
            GET_NAME(w.inlineValue, inlineValue, v, InlineValueWorkspaceClientCapabilities)
            GET_NAME(w.inlayHint, inlayHint, v, InlayHintWorkspaceClientCapabilities)
            GET_NAME(w.diagnostics, diagnostics, v, DiagnosticWorkspaceClientCapabilities)
            workspace = std::move(w);
        }
    }
    GET(textDocument, j, TextDocumentClientCapabilities)
    GET(notebookDocument, j, NotebookDocumentClientCapabilities)
    if (j.contains("window")) {
        if (const auto& v = j["window"]; v.is_object()) {
            auto w = ClientCapabilities::Window();
            getValue<bool>(w.workDoneProgress, v, "workDoneProgress");
            GET_NAME(w.showMessage, showMessage, v, ShowMessageRequestClientCapabilities)
            GET_NAME(w.showDocument, showDocument, v, ShowDocumentClientCapabilities)
            window = w;
        }
    }
    if (j.contains("general")) {
        if (const auto& v = j["general"]; v.is_object()) {
            auto g = ClientCapabilities::General();
            if (v.contains("staleRequestSupport")) {
                if (const auto& s = v["staleRequestSupport"]; s.is_object()) {
                    auto srs = ClientCapabilities::General::StaleRequestSupport();
                    getValue<bool>(srs.cancel, s, "cancel");
                    getVector(srs.retryOnContentModified, s, "retryOnContentModified");
                    g.staleRequestSupport = srs;
                }
                GET_NAME(g.regularExpressions, regularExpressions, v,
                         RegularExpressionsClientCapabilities)
                GET_NAME(g.markdown, markdown, v, MarkdownClientCapabilities)
                getEnumVector(g.positionEncodings, v, "positionEncodings",
                              PositionEncodingKindFromString);
            }
            general = g;
        }
    }

    if (j.contains("experimental"))
        experimental = j["experimental"];
#undef GET
}

WorkspaceFolder::WorkspaceFolder(const json& j) {
    GET_VALUE(string, uri, j);
    GET_VALUE(string, name, j)
}

InitializeParams::InitializeParams(const json& j) :
    WorkDoneProgressParams(j, ParamKind::Initialize) {
    if (j.contains("processId")) {
        if (const auto& p = j["processId"]; p.is_null())
            processId = -1;
        else
            processId = p.get<integer>();
    }
    else {
        processId = -1;
    }

    GET_VALUE(integer, processId, j)
    if (j.contains("clientInfo"sv)) {
        const auto& v = j["clientInfo"];
        auto client = InitializeParams::ClientInfo();
        getValue<string>(client.name, v, "name");
        getValue<string>(client.version, v, "version");
        clientInfo = client;
    }
    GET_VALUE(string, locale, j)
    GET_VALUE(string, rootPath, j)
    GET_VALUE(string, rootUri, j)
    if (j.contains("initializationOptions"sv))
        initializationOptions = j["initializationOptions"];
    capabilities = ClientCapabilities(j["capabilities"]);
    GET_ENUM(string, traceValue, j, TraceValueFromString)
    if (j.contains("workspaceFolders"sv)) {
        if (const auto& v = j["workspaceFolders"]; v.is_array())
            for (const auto& w : v)
                workspaceFolders.emplace_back(w);
    }
}

Message::Message(const json& j) {
    GET_VALUE(string, jsonrpc, j);
}

RequestMessage::RequestMessage(const json& j) : Message(j) {
    GET_VALUE(string, id, j)
    GET_ENUM(string, method, j, RPCMethodFromString)

    // Construct the correspondent param(s) for the method received
    if (method == RPCMethod::Initialize)
        params.emplace_back(std::make_unique<InitializeParams>(InitializeParams(j["params"])));
    else if (method == RPCMethod::Initialized)
        params.emplace_back(std::make_unique<InitializedParams>(InitializedParams(j["params"])));
}

json ResponseMessage::toJSON() const {
    json j;
    Message::toJSON(j);
    j["id"] = id;
    if (result)
        result.value()->toJSON(j["result"]);
    if (error)
        TO_JSON_OPT(error, j)
    return j;
}

void Result::toJSON(json& j) const {
    switch (kind) {
        case ResultKind::Initialize:
            this->as<InitializeResult>().toJSON(j["InitializeResutl"]);
            break;
    }
}

void InitializeResult::toJSON(json& j) const {
    TO_JSON(capabilities, j)
    j["serverInfo"] = serverInfo;
}

void ServerCapabilities::toJSON(json& j) const {
    j["positionEncoding"] = PositionEncodingKindToString(positionEncoding);
    TO_JSON(textDocumentSync, j)
    TO_JSON(completionProvider, j)
    TO_JSON(hoverProvider, j)
    TO_JSON(signatureHelpProvider, j)
    TO_JSON(declarationProvider, j)
    TO_JSON(typeDefinitionProvider, j)
    TO_JSON(implementationProvider, j)
    TO_JSON(referencesProvider, j)
    TO_JSON(documentHighlightProvider, j)
    TO_JSON(documentSymbolProvider, j)
    TO_JSON(codeActionProvider, j)
    TO_JSON(codeLensProvider, j)
    TO_JSON(documentLinkProvider, j)
    TO_JSON(colorProvider, j)
    TO_JSON(documentFormattingProvider, j)
    TO_JSON(documentRangeFormattingProvider, j)
    TO_JSON(documentOnTypeFormattingProvider, j)
    TO_JSON(renameProvider, j)
    TO_JSON(foldingRangeProvider, j)
    TO_JSON(executeCommandProvider, j)
    TO_JSON(selectionRangeProvider, j)
    TO_JSON(linkedEditingRangeProvider, j)
    TO_JSON(callHierarchyProvider, j)
    TO_JSON(semanticTokensProvider, j)
    TO_JSON(monikerPr, j)
    TO_JSON(typeHierarchyProvider, j)
    TO_JSON(inlineValueProvider, j)
    TO_JSON(inlayHintProv, j)
    TO_JSON(diagnosticProvider, j)
    TO_JSON(workspaceSymbolProvider, j)
    TO_JSON(workspace, j)
    if (experimental)
        j["experimental"] = experimental.value();
}

inline void TextDocumentSyncOptions::toJSON(json& j) const {
    j["openClose"] = openClose;
    j["change"] = TextDocumentSyncKindToUInteger(change);
}

inline json DocumentFilter::toJSON() const {
    json j;
    j["language"] = language;
    j["scheme"] = scheme;
    j["pattern"] = pattern;
    return j;
}

inline void TextDocumentRegistrationOptions::toJSON(json& j) const {
    if (documentSelector) {
        auto array = json::array();
        for (const auto& v : documentSelector.value())
            array.push_back(v.toJSON());
        j["documentSelector"] = json::array();
    }
}

inline void CompletionOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["triggerCharacters"] = triggerCharacters;
    j["allCommitCharacters"] = allCommitCharacters;
    j["resolveProvider"] = resolveProvider;
    j["completionItem"] = completionItem;
}

inline void DocumentSymbolOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["label"] = label;
}

inline void CodeLensOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["resolveProvider"] = resolveProvider;
}
inline void DocumentLinkOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["resolveProvider"] = resolveProvider;
}

inline void DocumentOnTypeFormattingOptions::toJSON(json& j) const {
    j["firstTriggerCharacter"] = firstTriggerCharacter;
    j["moreTriggerCharacter"] = moreTriggerCharacter;
}

inline void ExecuteCommandOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["commands"] = commands;
}

inline void InlayHintOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["resolveProvider"] = resolveProvider;
}

inline void DiagnosticOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["identifier"] = identifier;
    j["interFileDependencies"] = interFileDependencies;
    j["workspaceDiagnostics"] = workspaceDiagnostics;
}

inline void WorkspaceSymbolOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["resolveProvider"] = resolveProvider;
}

inline void SemanticTokensOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    legend.toJSON(j["legend"]);
    j["range"] = range;
    j["full"] = full;
}

inline void SemanticTokensLegend::toJSON(json& j) const {
    j["tokenTypes"] = tokenTypes;
    j["tokenModifiers"] = tokenModifiers;
}

inline void RenameOptions::toJSON(json& j) const {
    WorkDoneProgressOptions::toJSON(j);
    j["prepareProvider"] = prepareProvider;
}

inline void WorkspaceFoldersServerCapabilities::toJSON(json& j) const {
    j["supported"] = supported;
    j["changeNotifications"] = changeNotifications;
}

inline void FileOperationRegistrationOptions::toJSON(json& j) const {
    auto array = json::array();
    for (const auto& filter : filters) {
        json tmp;
        filter.toJSON(tmp);
        array.push_back(tmp);
    }
    j["filters"] = array;
}

inline void FileOperationFilter::toJSON(json& j) const {
    j["scheme"] = scheme;
    pattern.toJSON(j["pattern"]);
}

inline void FileOperationPattern::toJSON(json& j) const {
    j["glob"] = glob;
    j["matches"] = FileOperationPatternKindToString(matches);
    options.toJSON(j["options"]);
}

inline void FileOperationPatternOptions::toJSON(json& j) const {
    j["ignoreCase"] = ignoreCase;
}

inline void ServerCapabilities::Workspace::FileOperations::toJSON(json& j) const {
    didCreate.toJSON(j["didCreate"]);
    willCreate.toJSON(j["willCreate"]);
    didRename.toJSON(j["didRename"]);
    willRename.toJSON(j["willRename"]);
    didDelete.toJSON(j["didDelete"]);
    willDelete.toJSON(j["willDelete"]);
}

inline void ServerCapabilities::Workspace::toJSON(json& j) const {
    fileOperations.toJSON(j["fileOperations"]);
}

void ResponseError::toJSON(json& j) const {
    j["code"] = ErrorCodeToInteger(code);
    j["message"] = message;
    if (data)
        j["data"] = data.value();
}

NotebookDocumentSyncClientCapabilities::NotebookDocumentSyncClientCapabilities(const json& j) {
    GET_VALUE(bool, dynamicRegistration, j);
    GET_VALUE(bool, executionSummarySupport, j)
}

ShowMessageRequestClientCapabilities::ShowMessageRequestClientCapabilities(const json& j) {
    if (j.contains("messageActionItem")) {
        auto messageAction = MessageActionItem();
        getValue<bool>(messageAction.additionalPropertiesSupport, j["messageActionItem"],
                       "additionalPropertiesSupport");
    }
}

ShowDocumentClientCapabilities::ShowDocumentClientCapabilities(const json& j) {
    GET_VALUE(bool, support, j);
}

RegularExpressionsClientCapabilities::RegularExpressionsClientCapabilities(const json& j) {
    GET_VALUE(string, engine, j);
    GET_VALUE(string, version, j)
}

MarkdownClientCapabilities::MarkdownClientCapabilities(const json& j) {
    GET_VALUE(string, parser, j)
    GET_VALUE(string, version, j)
    GET_VECTOR(allowedTags, j)
}
} // namespace slang::rpc
