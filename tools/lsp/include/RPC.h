#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <utility>

#include "slang/util/Enum.h"

#define DECLARE_RUNTIME_EXC(_name)                                    \
    class _name : public std::runtime_error {                         \
    public:                                                           \
        explicit _name(const std::string& err) : runtime_error(err) { \
        }                                                             \
    };

namespace slang::rpc {

DECLARE_RUNTIME_EXC(UnknownEnumVariant)
DECLARE_RUNTIME_EXC(UnknownLSPMethod)

using json = nlohmann::json;

/// Defines an integer number in the range of -2^31 to 2^31 - 1.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#integer
using integer = int32_t;
/// Defines an unsigned integer number in the range of 0 to 2^31 - 1.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#uinteger
using uinteger = uint32_t;
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#decimal
using decimal = float;
/// The LSP any type.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#lspAny
using LSPAny = json;
using string = std::string;

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#progress
using ProgressToken = string;
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentUri
using DocumentUri = string;
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#uri
using URI = string;

class NoContentLengthException : public std::exception {};
class NoDefaultContentType : public std::exception {};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#headerPart
class LSPHeader {
    // Actually END_LINE should be "\r\n" but the std::getline function that we use already consumes
    // the '\n' character
    static constexpr std::string_view END_LINE = "\r";
    static constexpr std::string_view END_LINE_CRLF = "\r\n";
    static constexpr std::string_view CONTENT_TYPE = "Content-Type: ";
    static constexpr std::string_view DEFAULT_CONTENT_TYPE =
        "application/vscode-jsonrpc; charset=utf-8";
    static constexpr std::string_view CONTENT_LENGTH = "Content-Length: ";

public:
    explicit LSPHeader() : contentType(DEFAULT_CONTENT_TYPE) {}
    explicit LSPHeader(size_t size) : LSPHeader() { contentLength = size; }
    size_t contentLength;
    std::string contentType;

    std::string toString() const;
    static LSPHeader fromStdin();
};

#define KIND(x) x(WorkDoneProgress) x(Initialize)
SLANG_ENUM(ParamKind, KIND)
#undef KIND

class Param {
public:
    explicit Param(ParamKind kind) : kind(kind) {}
    ParamKind kind;

    template<typename T>
    decltype(auto) as() {
        SLANG_ASSERT(T::isKind(kind));
        return *static_cast<T*>(this);
    }

    template<typename T>
    const T& as() const {
        return const_cast<Param*>(this)->as<T>();
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workDoneProgressParams
class WorkDoneProgressParam : public Param {
public:
    explicit WorkDoneProgressParam(const json& j, ParamKind p = ParamKind::WorkDoneProgress) :
        Param(p) {
        if (j.contains("workDoneToken")) {
            workDoneToken = j["workDoneToken"];
        }
    }

    std::optional<ProgressToken> workDoneToken;
};

/// The kind of resource operations supported by the client.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#resourceOperationKind
enum class ResourceOperationKind : uint8_t {
    /// Supports creating new files and folders.
    Create,
    /// Supports renaming existing files and folders.
    Rename,
    /// Supports deleting existing files and folders.
    Delete
};
SLANG_BITMASK(ResourceOperationKind, Delete)
/// Converts the string representation of ResourceOperationKind to the correct enum variant
ResourceOperationKind ResourceOperationKindFromString(std::string_view str);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#failureHandlingKind
enum class FailureHandlingKind {
    /// Applying the workspace change is simply aborted if one of the changes provided fails.
    /// All operations executed before the failing operation stay executed.
    Abort,
    /// All operations are executed transactional. That means they either all
    //	succeed or no changes at all are applied to the workspace.
    Transactional,
    /// If the workspace edit contains only textual file changes they are executed transactional.
    /// If resource changes (create, rename or delete file) are part of the change the
    /// failure handling strategy is abort.
    TextOnlyTransactional,
    /// The client tries to undo the operations already executed. But there is no
    /// guarantee that this is succeeding.
    Undo,
    /// Added by implementation: No failure handling has been specified
    None
};
FailureHandlingKind FailureHandlingKindFromString(std::string_view str);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workspaceEditClientCapabilities
class WorkspaceEditClientCapabilities {
public:
    explicit WorkspaceEditClientCapabilities(const json& j);

    /// The client supports versioned document changes in `WorkspaceEdit`s.
    bool documentChanges;

    /// The resource operations the client supports. Clients should at least
    ///	support 'create', 'rename' and 'delete' files and folders.
    bitmask<ResourceOperationKind> resourceOperations;

    /// The failure handling strategy of a client if applying the workspace edit fails.
    FailureHandlingKind failureHandlingKind = FailureHandlingKind::None;

    /// Whether the client normalizes line endings to the client specific setting.
    /// If set to `true` the client will normalize line ending characters
    /// in a workspace edit to the client specific new line character(s).
    bool normalizesLineEndings;

    struct ChangeAnnotationSupport {
        /// Whether the client groups edits with equal labels into tree nodes,
        ///	for instance all edits labelled with "Changes in Strings" would be a tree node.
        bool groupsOnLabel;
    };
    /// Whether the client in general supports change annotations on text edits,
    ///	create file, rename file and delete file changes.
    std::optional<ChangeAnnotationSupport> changeAnnotationSupport;
};

class DynamicRegistration {
public:
    explicit DynamicRegistration(const json& j) {
        if (j.contains("dynamicRegistration"sv))
            if (const auto& v = j["dynamicRegistration"]; v.is_boolean())
                dynamicRegistration = v;
    }
    bool dynamicRegistration;
};

class RefreshSupport {
public:
    explicit RefreshSupport(const json& j) {
        if (j.contains("refreshSupport"sv))
            if (const auto& v = j["refreshSupport"]; v.is_boolean())
                refreshSupport = v;
    }
    bool refreshSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#didChangeConfigurationClientCapabilities
class DidChangeConfigurationClientCapabilities : public DynamicRegistration {
public:
    /// Did change configuration notification supports dynamic registration.
    explicit DidChangeConfigurationClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#didChangeWatchedFilesClientCapabilities
class DidChangeWatchedFilesClientCapabilities : public DynamicRegistration {
public:
    /// Did change watched files notification supports dynamic registration.
    /// Please note that the current protocol doesn't support static
    /// configuration for file changes from the server side.
    explicit DidChangeWatchedFilesClientCapabilities(const json& j);

    /// Whether the client has support for relative patterns or not.
    bool relativePatternSupport;
};

/// A symbol kind
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#symbolKind
enum class SymbolKindEnum {
    File,
    Module,
    Namespace,
    Package,
    Class,
    Method,
    Property,
    Field,
    Constructor,
    Enum,
    Interface,
    Function,
    Variable,
    Constant,
    String,
    Number,
    Boolean,
    Array,
    Object,
    Key,
    Null,
    EnumMember,
    Struct,
    Event,
    Operator,
    TypeParameter,
};
SLANG_BITMASK(SymbolKindEnum, TypeParameter)
SymbolKindEnum SymbolKindEnumFromUInteger(uinteger v);

/// Symbol tags are extra annotations that tweak the rendering of a symbol.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#symbolTag
enum class SymbolTag {
    /// Render a symbol as obsolete, usually using a strike-out.
    Deprecated
};
SLANG_BITMASK(SymbolTag, Deprecated)
SymbolTag SymbolTagFromUInteger(uinteger v);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workspace_symbol
class WorkspaceSymbolClientCapabilities : public DynamicRegistration {
public:
    /// Symbol request supports dynamic registration.
    explicit WorkspaceSymbolClientCapabilities(const json& j);

    struct SymbolKind {
        /// The symbol kind values the client supports. When this property exists the client also
        /// guarantees that it will handle values outside its set gracefully and falls back
        /// to a default value when unknown.
        ///
        /// If this property is not present the client only supports the symbol kinds
        /// from `File` to `Array` as defined in the initial version of the protocol.
        bitmask<SymbolKindEnum> valueSet;
    };
    /// Specific capabilities for the `SymbolKind` in the `workspace/symbol` request.
    std::optional<SymbolKind> symbolKind;

    struct TagSupport {
        /// The tags supported by the client
        bitmask<SymbolTag> valueSet;
    };
    /// The client supports tags on `SymbolInformation` and `WorkspaceSymbol`.
    /// Clients supporting tags have to handle unknown tags gracefully.
    std::optional<TagSupport> tagSupport;

    struct ResolveSupport {
        /// The properties that a client can resolve lazily. Usually `location.range`.
        // TODO:: Maybe this can be a pointer
        std::vector<string> properties;
    };
    /// The client support partial workspace symbols. The client will send the
    //	request `workspaceSymbol/resolve` to the server to resolve additional properties.
    std::optional<ResolveSupport> resolveSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#executeCommandClientCapabilities
class ExecuteCommandClientCapabilities : public DynamicRegistration {
public:
    /// Execute command supports dynamic registration.
    explicit ExecuteCommandClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensWorkspaceClientCapabilities
class SemanticTokensWorkspaceClientCapabilities : public RefreshSupport {
public:
    /// Whether the client implementation supports a refresh request sent from
    /// the server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// semantic tokens currently shown. It should be used with absolute care
    /// and is useful for situation where a server for example detect a project
    /// wide change that requires such a calculation.
    explicit SemanticTokensWorkspaceClientCapabilities(const json& j) : RefreshSupport(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#codeLensWorkspaceClientCapabilities
class CodeLensWorkspaceClientCapabilities : public RefreshSupport {
public:
    /// Whether the client implementation supports a refresh request sent from the
    /// server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// code lenses currently shown. It should be used with absolute care and is
    /// useful for situation where a server for example detect a project wide
    /// change that requires such a calculation.
    explicit CodeLensWorkspaceClientCapabilities(const json& j) : RefreshSupport(j) {}
};

/// Client workspace capabilities specific to inline values.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlineValueWorkspaceClientCapabilities
class InlineValueWorkspaceClientCapabilities : public RefreshSupport {
public:
    /// Whether the client implementation supports a refresh request sent from
    /// the server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// inline values currently shown. It should be used with absolute care and
    /// is useful for situation where a server for example detect a project wide
    /// change that requires such a calculation.
    explicit InlineValueWorkspaceClientCapabilities(const json& j) : RefreshSupport(j) {}
};

/// Client workspace capabilities specific to inlay hints.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlayHintWorkspaceClientCapabilities
class InlayHintWorkspaceClientCapabilities : public RefreshSupport {
public:
    ///  Whether the client implementation supports a refresh request sent from
    /// the server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// inlay hints currently shown. It should be used with absolute care and
    /// is useful for situation where a server for example detects a project wide
    /// change that requires such a calculation.
    explicit InlayHintWorkspaceClientCapabilities(const json& j) : RefreshSupport(j) {}
};

/// Workspace client capabilities specific to diagnostic pull requests.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#diagnosticWorkspaceClientCapabilities
class DiagnosticWorkspaceClientCapabilities : public RefreshSupport {
public:
    ///  Whether the client implementation supports a refresh request sent from
    ///	 the server to the client.
    ///
    ///	 Note that this event is global and will force the client to refresh all
    ///	 pulled diagnostics currently shown. It should be used with absolute care
    ///	 and is useful for situation where a server for example detects a project
    ///	 wide change that requires such a calculation.
    explicit DiagnosticWorkspaceClientCapabilities(const json& j) : RefreshSupport(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentSyncClientCapabilities
class TextDocumentSyncClientCapabilities : DynamicRegistration {
public:
    /// Whether text document synchronization supports dynamic registration.
    explicit TextDocumentSyncClientCapabilities(const json& j);

    /// The client supports sending will save notifications.
    bool willSave;

    /// The client supports sending a will save request and
    /// waits for a response providing text edits which will
    /// be applied to the document before it is saved.
    bool willSaveWaitUntil;

    /// The client supports did save notifications.
    bool didSave;
};

/// Describes the content type that a client supports in various
/// result literals like `Hover`, `ParameterInfo` or `CompletionItem`.
///
/// Please note that `MarkupKinds` must not start with a `$`. This kinds
/// are reserved for internal usage.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#markupContent
enum class MarkupKind : uint8_t {
    /// Plain text is supported as a content format
    PlainText,
    /// Markdown is supported as a content format
    Markdown
};
SLANG_BITMASK(MarkupKind, Markdown)
MarkupKind MarkupKindFromString(std::string_view str);

/// Completion item tags are extra annotations that tweak the rendering of a completion item.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#completionItemTag
enum class CompletionItemTag {
    /// Render a completion as obsolete, usually using a strike-out.
    Deprecated
};
SLANG_BITMASK(CompletionItemTag, Deprecated)
CompletionItemTag CompletionItemTagFromUInteger(uinteger v);

/// How whitespace and indentation is handled during completion item insertion.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#insertTextMode
enum class InsertTextMode {
    /// The insertion or replace strings is taken as it is. If the
    /// value is multi line the lines below the cursor will be
    /// inserted using the indentation defined in the string value.
    /// The client will not apply any kind of adjustments to the
    /// string.
    AsIs,

    /// The editor adjusts leading whitespace of new lines so that
    /// they match the indentation up to the cursor of the line for
    /// which the item is accepted.
    ///
    /// Consider a line like this: <2tabs><cursor><3tabs>foo. Accepting a
    /// multi line completion item is indented using 2 tabs and all
    /// following lines inserted will be indented using 2 tabs as well.
    AdjustIndentation
};
SLANG_BITMASK(InsertTextMode, AdjustIndentation)
InsertTextMode InsertTextModeFromUInteger(uinteger v);

/// The kind of a completion entry.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#completionItemKind
enum class CompletionItemKindEnum {
    Text,
    Method,
    Function,
    Constructor,
    Field,
    Variable,
    Class,
    Interface,
    Module,
    Property,
    Unit,
    Value,
    Enum,
    Keyword,
    Snippet,
    Color,
    File,
    Reference,
    Folder,
    EnumMember,
    Constant,
    Struct,
    Event,
    Operator,
    TypeParameter,
};
SLANG_BITMASK(CompletionItemKindEnum, TypeParameter)
CompletionItemKindEnum CompletionItemKindEnumFromUInteger(uinteger v);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#completionClientCapabilities
class CompletionClientCapabilities : public DynamicRegistration {
public:
    /// Whether completion supports dynamic registration.
    explicit CompletionClientCapabilities(const json& j);

    struct CompletionItem {
        /// Client supports snippets as insert text.
        ///
        /// A snippet can define tab stops and placeholders with `$1`, `$2`
        /// and `${3:foo}`. `$0` defines the final tab stop, it defaults to
        /// the end of the snippet. Placeholders with equal identifiers are
        /// linked, that is typing in one will update others too.
        bool snippetSupport;

        /// Client supports commit characters on a completion item.
        bool commitCharactersSupport;

        /// Client supports the follow content formats for the documentation
        /// property. The order describes the preferred format of the client.
        bitmask<MarkupKind> documentationFormat;
        /// Added by implementation: Preferred format of the client
        MarkupKind preferredDocumentationFormat;

        /// Client supports the deprecated property on a completion item.
        bool deprecatedSupport;

        /// Client supports the preselect property on a completion item.
        bool preselectSupport;

        struct TagSupport {
            /// The tags supported by the client.
            bitmask<CompletionItemTag> valueSet;
        };
        /// Client supports the tag property on a completion item. Clients
        /// supporting tags have to handle unknown tags gracefully. Clients
        /// especially need to preserve unknown tags when sending a completion
        /// item back to the server in a resolve call.
        std::optional<TagSupport> tagSupport;

        /// Client supports insert replace edit to control different behavior if
        /// a completion item is inserted in the text or should replace text.
        bool insertReplaceSupport;

        struct ResolveSupport {
            /// The properties that a client can resolve lazily.
            // TODO: Maybe this can be a pointer
            std::vector<string> properties;
        };
        /// Indicates which properties a client can resolve lazily on a
        /// completion item. Before version 3.16.0 only the predefined properties
        /// `documentation` and `detail` could be resolved lazily.
        std::optional<ResolveSupport> resolveSupport;

        struct InsertTextModeSupport {
            bitmask<InsertTextMode> valueSet;
        };
        /// The client supports the `insertTextMode` property on
        /// a completion item to override the whitespace handling mode
        /// as defined by the client (see `insertTextMode`).
        std::optional<InsertTextModeSupport> insertModeSupport;

        /// The client has support for completion item label details
        /// (see also `CompletionItemLabelDetails`).
        bool labelDetailsSupport;
    };
    /// The client supports the following `CompletionItem` specific
    /// capabilities.
    std::optional<CompletionItem> completionItem;

    struct CompletionItemKind {
        /// The completion item kind values the client supports. When this
        /// property exists the client also guarantees that it will
        /// handle values outside its set gracefully and falls back
        /// to a default value when unknown.
        ///
        /// If this property is not present the client only supports
        /// the completion items kinds from `Text` to `Reference` as defined in
        /// the initial version of the protocol.
        bitmask<CompletionItemKindEnum> valueSet;
    };
    std::optional<CompletionItemKind> completionItemKind;

    /// The client supports to send additional context information for a
    ///  `textDocument/completion` request.
    bool contextSupport;

    /// The client's default when the completion item doesn't provide a `insertTextMode` property.
    std::optional<InsertTextMode> insertTextMode;

    struct CompletionList {
        /// The client supports the following itemDefaults on
        /// a completion list.
        ///
        /// The value lists the supported property names of the
        /// `CompletionList.itemDefaults` object. If omitted
        /// no properties are supported.
        // TODO: Maybe this can be a pointer
        std::vector<string> itemDefaults;
    };
    /// The client supports the following `CompletionList` specific capabilities.
    std::optional<CompletionList> completionList;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#hoverClientCapabilities
class HoverClientCapabilities : public DynamicRegistration {
public:
    /// Whether hover supports dynamic registration.
    explicit HoverClientCapabilities(const json& j);

    /// Client supports the follow content formats if the content property refers to a
    /// `literal of type MarkupContent`. The order describes the preferred format of the client.
    bitmask<MarkupKind> contentFormat;
    /// Added by implementation: Preferred format of the client
    std::optional<MarkupKind> preferredContentFormat;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#signatureHelpClientCapabilities
class SignatureHelpClientCapabilities : public DynamicRegistration {
public:
    /// Whether signature help supports dynamic registration.
    explicit SignatureHelpClientCapabilities(const json& j);

    /// The client supports the following `SignatureInformation` specific properties.
    struct SignatureInformation {
        /// Client supports the follow content formats for the documentation
        /// property. The order describes the preferred format of the client.
        bitmask<MarkupKind> documentationFormat;
        /// Added by implementation: Preferred format of the client
        std::optional<MarkupKind> preferredDocumentationFormat;

        struct ParameterInformation {
            /// The client supports processing label offsets instead of a simple label string.
            bool labelOffsetSupport;
        };
        /// Client capabilities specific to parameter information.
        std::optional<ParameterInformation> parameterInformation;

        /// The client supports the `activeParameter` property on `SignatureInformation` literal.
        bool activeParameterSupport;
    };
    std::optional<SignatureInformation> signatureInformation;

    /// The client supports to send additional context information for a
    /// `textDocument/signatureHelp` request. A client that opts into
    /// contextSupport will also support the `retriggerCharacters` on
    /// `SignatureHelpOptions`.
    bool contextSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#declarationClientCapabilities
class DeclarationClientCapabilities : public DynamicRegistration {
public:
    /// Whether declaration supports dynamic registration. If this is set to
    /// `true` the client supports the new `DeclarationRegistrationOptions`
    /// return value for the corresponding server capability as well.
    explicit DeclarationClientCapabilities(const json& j);

    /// The client supports additional metadata in the form of declaration links.
    bool linkSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#definitionClientCapabilities
class DefinitionClientCapabilities : public DynamicRegistration {
public:
    /// Whether definition supports dynamic registration.
    explicit DefinitionClientCapabilities(const json& j);

    /// The client supports additional metadata in the form of definition links.
    bool linkSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#typeDefinitionClientCapabilities
class TypeDefinitionClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration. If this is set to
    /// `true` the client supports the new `TypeDefinitionRegistrationOptions`
    /// return value for the corresponding server capability as well.
    explicit TypeDefinitionClientCapabilities(const json& j);

    /// The client supports additional metadata in the form of definition links.
    bool linkSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#implementationClientCapabilities
class ImplementationClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration. If this is set to
    /// `true` the client supports the new `ImplementationRegistrationOptions`
    /// return value for the corresponding server capability as well.
    explicit ImplementationClientCapabilities(const json& j);

    /// The client supports additional metadata in the form of definition links.
    bool linkSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#referenceClientCapabilities
class ReferenceClientCapabilities : public DynamicRegistration {
public:
    /// Whether references supports dynamic registration.
    explicit ReferenceClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentHighlightClientCapabilities
class DocumentHighlightClientCapabilities : public DynamicRegistration {
public:
    /// Whether document highlight supports dynamic registration.
    explicit DocumentHighlightClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentSymbolClientCapabilities
class DocumentSymbolClientCapabilities : public DynamicRegistration {
public:
    /// Whether document symbol supports dynamic registration.
    explicit DocumentSymbolClientCapabilities(const json& j);

    struct SymbolKind {
        /// The symbol kind values the client supports. When this
        /// property exists the client also guarantees that it will
        /// handle values outside its set gracefully and falls back
        /// to a default value when unknown.
        ///
        /// If this property is not present the client only supports
        /// the symbol kinds from `File` to `Array` as defined in
        /// the initial version of the protocol.
        bitmask<SymbolKindEnum> valueSet;
    };
    /// Specific capabilities for the `SymbolKind` in the `textDocument/documentSymbol` request.
    std::optional<SymbolKind> symbolKind;

    /// The client supports hierarchical document symbols.
    bool hierarchicalDocumentSymbolSupport;

    struct TagSupport {
        /// The tags supported by the client.
        bitmask<SymbolTag> valueSet;
    };
    /// The client supports tags on `SymbolInformation`. Tags are supported on
    /// `DocumentSymbol` if `hierarchicalDocumentSymbolSupport` is set to true.
    /// Clients supporting tags have to handle unknown tags gracefully.
    std::optional<TagSupport> tagSupport;

    /// The client supports an additional label presented in the UI when
    /// registering a document symbol provider.
    bool labelSupport;
};

///  The kind of a code action.
///
/// Kinds are a hierarchical list of identifiers separated by `.`, e.g.
/// `"refactor.extract.function"`.
///
/// The set of kinds is open and client needs to announce the kinds it supports
/// to the server during initialization.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#codeActionKind
enum class CodeActionKind : uint8_t {
    /// Empty kind.
    Empty,
    /// Base kind for quickfix actions: 'quickfix'.
    QuickFix,
    /// Base kind for refactoring actions: 'refactor'.
    Refactor,
    /// Base kind for refactoring extraction actions: 'refactor.extract'.
    ///
    /// Example extract actions:
    ///
    /// - Extract method
    /// - Extract function
    /// - Extract variable
    /// - Extract interface from class
    /// - ...
    RefactorExtract,
    /// Base kind for refactoring inline actions: 'refactor.inline'.
    ///
    /// Example inline actions:
    ///
    /// - Inline function
    /// - Inline variable
    /// - Inline constant
    /// - ...
    RefactorInline,
    /// Base kind for refactoring rewrite actions: 'refactor.rewrite'.
    ///
    /// Example rewrite actions:
    ///
    /// - Convert JavaScript function to class
    /// - Add or remove parameter
    /// - Encapsulate field
    /// - Make method static
    /// - Move method to base class
    /// - ...
    RefactorRewrite,
    /// Base kind for source actions: `source`.
    ///
    /// Source code actions apply to the entire file.
    Source,
    // Base kind for an organize imports source action:
    // `source.organizeImports`.
    SourceOrganizeImports,
    /// Base kind for a 'fix all' source action: `source.fixAll`.
    ///
    /// 'Fix all' actions automatically fix errors that have a clear fix that
    /// do not require user input. They should not suppress errors or perform
    /// unsafe fixes such as generating new types or classes.
    SourceFixAll,
};
SLANG_BITMASK(CodeActionKind, SourceFixAll)
CodeActionKind CodeActionKindFromString(std::string_view str);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#codeActionClientCapabilities
class CodeActionClientCapabilities : public DynamicRegistration {
public:
    /// Whether code action supports dynamic registration.
    explicit CodeActionClientCapabilities(const json& j);

    /// The client supports code action literals as a valid
    /// response of the `textDocument/codeAction` request.
    struct CodeActionLiteralSupport {
        /// The code action kind values the client supports. When this property exists the
        /// client also guarantees that it will handle values outside its set gracefully
        /// and falls back to a default value when unknown.
        bitmask<CodeActionKind> codeActionKind;
    };
    std::optional<CodeActionLiteralSupport> codeActionLiteralSupport;

    /// Whether code action supports the `isPreferred` property.
    bool isPreferredSupport;

    /// Whether code action supports the `disabled` property.
    bool disabledSupport;

    /// Whether code action supports the `data` property which is preserved between
    /// a `textDocument/codeAction` and a `codeAction/resolve` request.
    bool dataSupport;

    struct ResolveSupport {
        /// The properties that a client can resolve lazily.
        // TODO: Maybe this can be a pointer
        std::vector<string> properties;
    };
    /// Whether the client supports resolving additional code action
    /// properties via a separate `codeAction/resolve` request.
    std::optional<ResolveSupport> resolveSupport;

    /// Whether the client honors the change annotations in text edits and resource
    /// operations returned via the `CodeAction#edit` property by for example presenting
    /// the workspace edit in the user interface and asking for confirmation.
    bool honorsChangeAnnotations;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#codeLensClientCapabilities
class CodeLensClientCapabilities : DynamicRegistration {
public:
    /// Whether code lens supports dynamic registration.
    explicit CodeLensClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentLinkClientCapabilities
class DocumentLinkClientCapabilities : public DynamicRegistration {
public:
    /// Whether document link supports dynamic registration.
    explicit DocumentLinkClientCapabilities(const json& j);

    /// Whether the client supports the `tooltip` property on `DocumentLink`.
    bool tooltipSupport;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentColorClientCapabilities
class DocumentColorClientCapabilities : public DynamicRegistration {
public:
    /// Whether document color supports dynamic registration.
    explicit DocumentColorClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentFormattingClientCapabilities
class DocumentFormattingClientCapabilities : public DynamicRegistration {
public:
    /// Whether formatting supports dynamic registration.
    explicit DocumentFormattingClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentRangeFormattingClientCapabilities
class DocumentRangeFormattingClientCapabilities : public DynamicRegistration {
public:
    /// Whether formatting supports dynamic registration.
    explicit DocumentRangeFormattingClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentOnTypeFormattingClientCapabilities
class DocumentOnTypeFormattingClientCapabilities : public DynamicRegistration {
public:
    explicit DocumentOnTypeFormattingClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#prepareSupportDefaultBehavior
enum class PrepareSupportDefaultBehavior {
    /// The client's default behavior is to select the identifier
    /// according to the language's syntax rule.
    Identifier
};
PrepareSupportDefaultBehavior PrepareSupportDefaultBehaviorFromUInteger(uinteger v);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#renameClientCapabilities
class RenameClientCapabilities : public DynamicRegistration {
public:
    /// Whether rename supports dynamic registration.
    explicit RenameClientCapabilities(const json& j);

    /// Client supports testing for validity of rename operations
    /// before execution.
    bool prepareSupport;

    /// Client supports the default behavior result (`{ defaultBehavior: boolean }`).
    ///
    /// The value indicates the default behavior used by the client.
    std::optional<PrepareSupportDefaultBehavior> prepareSupportDefaultBehavior;

    /// Whether the client honors the change annotations in text edits and resource operations
    /// returned via the rename request's workspace edit by for example presenting the workspace
    /// edit in the user interface and asking for confirmation.
    bool honorsChangeAnnotations;
};

/// The diagnostic tags.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#diagnosticTag
enum class DiagnosticTag {
    /// Unused or unnecessary code.
    ///
    /// Clients are allowed to render diagnostics with this tag faded out
    /// instead of having an error squiggle.
    Unnecessary,
    /// Deprecated or obsolete code.
    ///
    /// Clients are allowed to rendered diagnostics with this tag strike through.
    Deprecated
};
SLANG_BITMASK(DiagnosticTag, Deprecated)
DiagnosticTag DiagnosticTagFromUInteger(uinteger v);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#publishDiagnosticsClientCapabilities
class PublishDiagnosticsClientCapabilities {
public:
    explicit PublishDiagnosticsClientCapabilities(const json& j);

    /// Whether the clients accepts diagnostics with related information.
    bool relatedInformation;

    /// Client supports the tag property to provide meta data about a diagnostic.
    /// Clients supporting tags have to handle unknown tags gracefully.
    bitmask<DiagnosticTag> tagSupport;

    /// Whether the client interprets the version property of the
    /// `textDocument/publishDiagnostics` notification's parameter.
    bool versionSupport;

    /// Client supports a codeDescription property
    bool codeDescriptionSupport;

    /// Whether code action supports the `data` property which is
    /// preserved between a `textDocument/publishDiagnostics` and
    /// `textDocument/codeAction` request.
    bool dataSupport;
};

/// A set of predefined range kinds.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#foldingRangeKind
enum class FoldingRangeKind {
    /// Folding range for a comment
    Comment,
    /// Folding range for imports or includes
    Imports,
    /// Folding range for a region (e.g. `#region`)
    Region
};
SLANG_BITMASK(FoldingRangeKind, Region)
FoldingRangeKind FoldingRangeKindFromString(std::string_view str);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#foldingRangeClientCapabilities
class FoldingRangeClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration for folding range providers.
    /// If this is set to `true` the client supports the new `FoldingRangeRegistrationOptions`
    /// return value for the corresponding server capability as well.
    explicit FoldingRangeClientCapabilities(const json& j);

    /// The maximum number of folding ranges that the client prefers to receive
    /// per document. The value serves as a hint, servers are free to follow the limit.
    uinteger rangeLimit;

    /// If set, the client signals that it only supports folding complete lines. If set, client
    /// will ignore specified `startCharacter` and `endCharacter` properties in a FoldingRange.
    bool lineFoldingOnly;

    /// The folding range kind values the client supports. When this property exists the client
    /// also guarantees that it will handle values outside its set gracefully and falls back
    /// to a default value when unknown.
    bitmask<FoldingRangeKind> foldingRangeKind;

    struct FoldingRange {
        /// If set, the client signals that it supports setting collapsedText on
        /// folding ranges to display custom labels instead of the default text.
        bool collapsedText;
    };
    /// Specific options for the folding range.
    std::optional<FoldingRange> foldingRange;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#selectionRangeClientCapabilities
class SelectionRangeClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration for selection range providers.
    /// If this is set to `true` the client supports the new `SelectionRangeRegistrationOptions`
    /// return value for the corresponding server capability as well.
    explicit SelectionRangeClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#linkedEditingRangeClientCapabilities
class LinkedEditingRangeClientCapabilities : public DynamicRegistration {
public:
    /// Whether the implementation supports dynamic registration. If this is set to `true` the
    /// client supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    explicit LinkedEditingRangeClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#callHierarchyClientCapabilities
class CallHierarchyClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration. If this is set to
    /// `true` the client supports the new `(TextDocumentRegistrationOptions &
    /// StaticRegistrationOptions)` return value for the corresponding server
    /// capability as well.
    explicit CallHierarchyClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#tokenFormat
enum class TokenFormat : uint8_t { Relative };
SLANG_BITMASK(TokenFormat, Relative)
TokenFormat TokenFormatFromString(std::string_view str);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensClientCapabilities
class SemanticTokensClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration. If this is set to `true` the client
    /// supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    explicit SemanticTokensClientCapabilities(const json& j);

    // TODO: This structure may no be correctly represented from the spec

    /// Which requests the client supports and might send to the server
    /// depending on the server's capability. Please note that clients might not
    /// show semantic tokens or degrade some of the user experience if a range
    /// or full request is advertised by the client but not provided by the
    /// server. If for example the client capability `requests.full` and
    /// `request.range` are both set to true but the server only provides a
    /// range provider the client might not render a minimap correctly or might
    /// even decide to not show any semantic tokens at all.
    struct Requests {
        /// The client will send the `textDocument/semanticTokens/range` request
        /// if the server provides a corresponding handler.
        bool range;
        /// The client will send the `textDocument/semanticTokens/full` request
        /// if the server provides a corresponding handler.
        bool full;
        /// The client will send the `textDocument/semanticTokens/full/delta`
        /// request if the server provides a corresponding handler.
        bool fullDelta;
    } requests;

    /// The token types that the client supports.
    /// TODO: Maybe this can be a pointer
    std::vector<string> tokenTypes;

    /// The token modifiers that the client supports.
    /// TODO: Maybe this can be a pointer
    std::vector<string> tokenModifiers;

    /// The formats the clients supports.
    bitmask<TokenFormat> formats;

    /// Whether the client supports tokens that can overlap each other.
    bool overlappingTokenSupport;

    /// Whether the client supports tokens that can span multiple lines.
    bool multilineTokenSupport;

    /// Whether the client allows the server to actively cancel a semantic token request,
    /// e.g. supports returning ErrorCodes.ServerCancelled. If a server does the client
    /// needs to retrigger the request.
    bool serverCancelSupport;

    /// Whether the client uses semantic tokens to augment existing syntax tokens.
    /// If set to `true` client side created syntax tokens and semantic tokens are both used
    /// for colorization. If set to `false` the client only uses the returned semantic tokens
    /// for colorization.
    ///
    /// If the value is `undefined` then the client behavior is not specified.
    std::optional<bool> augmentsSyntaxTokens;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#monikerClientCapabilities
class MonikerClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration. If this is set to
    /// `true` the client supports the new `(TextDocumentRegistrationOptions &
    /// StaticRegistrationOptions)` return value for the corresponding server
    /// capability as well.
    explicit MonikerClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#typeHierarchyClientCapabilities
class TypeHierarchyClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration. If this is set to
    /// `true` the client supports the new `(TextDocumentRegistrationOptions &
    /// StaticRegistrationOptions)` return value for the corresponding server
    /// capability as well.
    explicit TypeHierarchyClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// Client capabilities specific to inline values.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlineValueClientCapabilities
class InlineValueClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration for inline
    /// value providers.
    explicit InlineValueClientCapabilities(const json& j) : DynamicRegistration(j) {}
};

/// Inlay hint client capabilities.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlayHintClientCapabilities
class InlayHintClientCapabilities : public DynamicRegistration {
public:
    /// Whether inlay hints support dynamic registration.
    explicit InlayHintClientCapabilities(const json& j);

    struct ResolveSupport {
        /// The properties that a client can resolve lazily.
        // TODO: Maybe this can be a pointer
        std::vector<string> properties;
    };
    /// Indicates which properties a client can resolve lazily on an inlay hint.
    std::optional<ResolveSupport> resolveSupport;
};

/// Client capabilities specific to diagnostic pull requests.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#diagnosticClientCapabilities
class DiagnosticClientCapabilities : public DynamicRegistration {
public:
    /// Whether implementation supports dynamic registration. If this is set to `true` the client
    /// supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    explicit DiagnosticClientCapabilities(const json& j);

    /// Whether the clients supports related documents for document diagnostic pulls.
    bool relatedDocumentSupport;
};

/// Text document specific client capabilities.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentClientCapabilities
class TextDocumentClientCapabilities {
public:
    explicit TextDocumentClientCapabilities(const json& j);

    std::optional<TextDocumentSyncClientCapabilities> synchronization;

    /// Capabilities specific to the `textDocument/completion` request.
    std::optional<CompletionClientCapabilities> completion;

    /// Capabilities specific to the `textDocument/hover` request.
    std::optional<HoverClientCapabilities> hover;

    /// Capabilities specific to the `textDocument/signatureHelp` request.
    std::optional<SignatureHelpClientCapabilities> signatureHelp;

    /// Capabilities specific to the `textDocument/declaration` request.
    std::optional<DeclarationClientCapabilities> declaration;

    /// Capabilities specific to the `textDocument/definition` request.
    std::optional<DefinitionClientCapabilities> definition;

    /// Capabilities specific to the `textDocument/typeDefinition` request.
    std::optional<TypeDefinitionClientCapabilities> typeDefinition;

    /// Capabilities specific to the `textDocument/implementation` request.
    std::optional<ImplementationClientCapabilities> implementation;

    /// Capabilities specific to the `textDocument/references` request.
    std::optional<ReferenceClientCapabilities> references;

    /// Capabilities specific to the `textDocument/documentHighlight` request.
    std::optional<DocumentHighlightClientCapabilities> documentHighlight;

    /// Capabilities specific to the `textDocument/documentSymbol` request.
    std::optional<DocumentSymbolClientCapabilities> documentSymbol;

    /// Capabilities specific to the `textDocument/codeAction` request.
    std::optional<CodeActionClientCapabilities> codeAction;

    /// Capabilities specific to the `textDocument/codeLens` request.
    std::optional<CodeLensClientCapabilities> codeLens;

    /// Capabilities specific to the `textDocument/documentLink` request.
    std::optional<DocumentLinkClientCapabilities> documentLink;

    /// Capabilities specific to the `textDocument/documentColor` and the
    /// `textDocument/colorPresentation` request.
    std::optional<DocumentColorClientCapabilities> colorProvider;

    /// Capabilities specific to the `textDocument/formatting` request.
    std::optional<DocumentFormattingClientCapabilities> formatting;

    /// Capabilities specific to the `textDocument/rangeFormatting` request.
    std::optional<DocumentRangeFormattingClientCapabilities> rangeFormatting;

    /// Capabilities specific to the `textDocument/onTypeFormatting` request.
    std::optional<DocumentOnTypeFormattingClientCapabilities> onTypeFormatting;

    /// Capabilities specific to the `textDocument/rename` request.
    std::optional<RenameClientCapabilities> rename;

    /// Capabilities specific to the `textDocument/publishDiagnostics` notification.
    std::optional<PublishDiagnosticsClientCapabilities> publishDiagnostics;

    /// Capabilities specific to the `textDocument/foldingRange` request.
    std::optional<FoldingRangeClientCapabilities> foldingRange;

    /// Capabilities specific to the `textDocument/selectionRange` request.
    std::optional<SelectionRangeClientCapabilities> selectionRange;

    /// Capabilities specific to the `textDocument/linkedEditingRange` request.
    std::optional<LinkedEditingRangeClientCapabilities> linkedEditingRange;

    /// Capabilities specific to the various call hierarchy requests.
    std::optional<CallHierarchyClientCapabilities> callHierarchy;

    /// Capabilities specific to the various semantic token requests.
    std::optional<SemanticTokensClientCapabilities> semanticTokens;

    /// Capabilities specific to the `textDocument/moniker` request.
    std::optional<MonikerClientCapabilities> moniker;

    /// Capabilities specific to the various type hierarchy requests.
    std::optional<TypeHierarchyClientCapabilities> typeHierarchy;

    /// Capabilities specific to the `textDocument/inlineValue` request.
    std::optional<InlineValueClientCapabilities> inlineValue;

    /// Capabilities specific to the `textDocument/inlayHint` request.
    std::optional<InlayHintClientCapabilities> inlayHint;

    /// Capabilities specific to the diagnostic pull model.
    std::optional<DiagnosticClientCapabilities> diagnostic;
};

/// Notebook specific client capabilities.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#notebookDocumentSyncClientCapabilities
class NotebookDocumentSyncClientCapabilities {
public:
    explicit NotebookDocumentSyncClientCapabilities(const json& j);

    /// Whether implementation supports dynamic registration. If this is
    /// set to `true` the client supports the new
    /// `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    bool dynamicRegistration;

    /// The client supports sending execution summary data per cell.
    bool executionSummarySupport;
};

/// Capabilities specific to the notebook document support.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#notebookDocumentClientCapabilities
class NotebookDocumentClientCapabilities {
public:
    explicit NotebookDocumentClientCapabilities(const json& j) :
        synchronization(NotebookDocumentSyncClientCapabilities(j["synchronization"])) {}

    /// Capabilities specific to notebook document synchronization
    NotebookDocumentSyncClientCapabilities synchronization;
};

/// Show message request client capabilities
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#window_showMessageRequest
class ShowMessageRequestClientCapabilities {
public:
    explicit ShowMessageRequestClientCapabilities(const json& j);

    struct MessageActionItem {
        /// Whether the client supports additional attributes which are preserved and sent
        /// back to the server in the request's response.
        bool additionalPropertiesSupport;
    };
    /// Capabilities specific to the `MessageActionItem` type.
    std::optional<MessageActionItem> messageActionItem;
};

/// Client capabilities for the show document request.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#window_showDocument
class ShowDocumentClientCapabilities {
public:
    explicit ShowDocumentClientCapabilities(const json& j);

    /// The client has support for the show document request.
    bool support;
};

/// Client capabilities specific to regular expressions.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#regExp
class RegularExpressionsClientCapabilities {
public:
    RegularExpressionsClientCapabilities() = default;
    explicit RegularExpressionsClientCapabilities(const json& j);

    /// The engine's name.
    string engine;

    /// The engine's version.
    std::optional<string> version;
};

/// Client capabilities specific to the used markdown parser.
class MarkdownClientCapabilities {
public:
    MarkdownClientCapabilities() = default;
    explicit MarkdownClientCapabilities(const json& j);

    /// The name of the parser.
    string parser;

    /// The version of the parser.
    std::optional<string> version;

    /// A list of HTML tags that the client allows / supports in Markdown.
    std::vector<string> allowedTags;
};

/// A set of predefined position encoding kinds.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#positionEncodingKind
enum class PositionEncodingKind {
    /// Character offsets count UTF-8 code units (e.g bytes).
    UTF8,
    /// Character offsets count UTF-16 code units.
    /// This is the default and must always be supported by servers
    UTF16,
    /// Character offsets count UTF-32 code units.
    ///
    /// Implementation note: these are the same as Unicode code points, so this
    /// `PositionEncodingKind` may also be used for an encoding-agnostic representation of
    /// character offsets.
    UTF32
};
std::string PositionEncodingKindToString(PositionEncodingKind kind);
PositionEncodingKind PositionEncodingKindFromString(std::string_view kind);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#clientCapabilities
class ClientCapabilities {
public:
    ClientCapabilities() = default;
    explicit ClientCapabilities(const json& j);

    struct Workspace {
        /// The client supports applying batch edits to the workspace by supporting the request
        ///	'workspace/applyEdit'.
        bool applyEdit;

        /// Capabilities specific to `WorkspaceEdit`s.
        std::optional<WorkspaceEditClientCapabilities> workspaceEditCapabilities;

        /// Capabilities specific to the `workspace/didChangeConfiguration` notification.
        std::optional<DidChangeConfigurationClientCapabilities> didChangeConfiguration;

        /// Capabilities specific to the `workspace/didChangeWatchedFiles` notification.
        std::optional<DidChangeWatchedFilesClientCapabilities> didChangeWatchedFiles;

        /// Capabilities specific to the `workspace/symbol` request.
        std::optional<WorkspaceSymbolClientCapabilities> symbol;

        /// Capabilities specific to the `workspace/executeCommand` request.
        std::optional<ExecuteCommandClientCapabilities> executeCommand;

        /// The client has support for workspace folders.
        bool workspaceFolders;

        /// The client supports `workspace/configuration` requests.
        bool configuration;

        /// Capabilities specific to the semantic token requests scoped to the workspace.
        std::optional<SemanticTokensWorkspaceClientCapabilities> semanticTokens;

        /// Capabilities specific to the code lens requests scoped to the workspace.
        std::optional<CodeLensWorkspaceClientCapabilities> codeLens;

        struct FileOperations {
            /// Whether the client supports dynamic registration for file requests/notifications.
            bool dynamicRegistration;

            /// The client has support for sending didCreateFiles notifications.
            bool didCreate;

            /// The client has support for sending willCreateFiles requests.
            bool willCreate;

            /// The client has support for sending didRenameFiles notifications.
            bool didRename;

            /// The client has support for sending willRenameFiles requests.
            bool willRename;

            /// The client has support for sending didDeleteFiles notifications.
            bool didDelete;

            /// The client has support for sending willDeleteFiles requests.
            bool willDelete;
        };
        /// The client has support for file requests/notifications.
        std::optional<FileOperations> fileOperations;

        /// Client workspace capabilities specific to inline values.
        std::optional<InlineValueWorkspaceClientCapabilities> inlineValue;

        /// Client workspace capabilities specific to inlay hints.
        std::optional<InlayHintWorkspaceClientCapabilities> inlayHint;

        /// Client workspace capabilities specific to diagnostics.
        std::optional<DiagnosticWorkspaceClientCapabilities> diagnostics;
    };
    /// Workspace specific client capabilities.
    std::optional<Workspace> workspace;

    /// Text document specific client capabilities.
    std::optional<TextDocumentClientCapabilities> textDocument;

    /// Capabilities specific to the notebook document support.
    std::optional<NotebookDocumentClientCapabilities> notebookDocument;

    struct Window {
        /// It indicates whether the client supports server initiated progress using the
        /// `window/workDoneProgress/create` request.
        ///
        /// The capability also controls Whether client supports handling of progress
        /// notifications. If set servers are allowed to report a `workDoneProgress` property
        /// in the request specific server capabilities.
        bool workDoneProgress;

        /// Capabilities specific to the showMessage request
        std::optional<ShowMessageRequestClientCapabilities> showMessage;

        /// Client capabilities for the show document request.
        std::optional<ShowDocumentClientCapabilities> showDocument;
    };
    /// Window specific client capabilities.
    std::optional<Window> window;

    struct General {
        /// Client capability that signals how the client handles stale requests (e.g. a request
        /// for which the client will not process the response anymore since the information
        /// is outdated).
        struct StaleRequestSupport {
            /// The client will actively cancel the request.
            bool cancel;

            /// The list of requests for which the client will retry the request if it receives a
            /// response with error code `ContentModified``
            std::vector<string> retryOnContentModified;
        };
        std::optional<StaleRequestSupport> staleRequestSupport;

        /// Client capabilities specific to regular expressions.
        RegularExpressionsClientCapabilities regularExpressions;

        /// Client capabilities specific to the client's markdown parser.
        MarkdownClientCapabilities markdown;

        /// The position encodings supported by the client. Client and server have to agree on
        /// the same position encoding to ensure that offsets (e.g. character position in a line)
        /// are interpreted the same on both side.
        ///
        /// To keep the protocol backwards compatible the following applies: if the value 'utf-16'
        /// is missing from the array of position encodings servers can assume that the client
        /// supports UTF-16. UTF-16 is therefore a mandatory encoding.
        ///
        /// If omitted it defaults to ['utf-16'].
        ///
        /// Implementation considerations: since the conversion from one encoding
        /// into another requires the content of the file / line the conversion
        /// is best done where the file is read which is usually on the server side.
        std::vector<PositionEncodingKind> positionEncodings;
    };
    /// General client capabilities.
    std::optional<General> general;

    /// Experimental client capabilities.
    std::optional<LSPAny> experimental;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#traceValue
enum class TraceValue { OFF, MESSAGES, VERBOSE };
TraceValue TraceValueFromString(std::string_view str);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workspaceFolder
class WorkspaceFolder {
public:
    explicit WorkspaceFolder(const json& j);
    explicit WorkspaceFolder(URI u, string n) : uri(std::move(u)), name(std::move(n)) {}

    /// The associated URI for this workspace folder.
    URI uri;

    /// The name of the workspace folder. Used to refer to this workspace folder in the user
    /// interface.
    string name;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initializeParams
class InitializeParams : public WorkDoneProgressParam {
public:
    explicit InitializeParams(const json& j);

    static bool isKind(ParamKind kind) { return kind == ParamKind::Initialize; }

    /// The process Id of the parent process that started the server. Is null if
    /// the process has not been started by another process. If the parent
    /// process is not alive then the server should exit (see exit notification).
    ///
    /// Implementation note: the null value is represented as -1 since process ids
    /// are always positive
    integer processId;

    struct ClientInfo {
        /// The name of the client as defined by the client.
        string name;
        /// The client's version as defined by the client.
        string version;
    };
    /// Information about the client.
    std::optional<ClientInfo> clientInfo;

    /// The locale the client is currently showing the user interface
    /// in. This must not necessarily be the locale of the operating system.
    ///
    /// Uses IETF language tags as the value's syntax. See
    /// https://en.wikipedia.org/wiki/IETF_language_tag
    std::optional<string> locale;

    /// The rootPath of the workspace. Is null if no folder is open.
    ///
    /// @deprecated in favour of `rootUri`.
    std::optional<string> rootPath;

    /// The rootUri of the workspace. Is null if no folder is open.
    /// If both `rootPath` and `rootUri` are set `rootUri` wins.
    ///
    ///@deprecated in favour of `workspaceFolders`
    std::optional<DocumentUri> rootUri;

    /// User provided initialization options.
    std::optional<LSPAny> initializationOptions;

    /// The capabilities provided by the client (editor or tool)
    ClientCapabilities capabilities;

    /// The initial trace setting. If omitted trace is disabled ('off').
    std::optional<TraceValue> traceValue;

    /// The workspace folders configured in the client when the server starts. This property is
    /// only available if the client supports workspace folders. It can be `null` if the client
    /// supports workspace folders but none are configured.
    std::vector<WorkspaceFolder> workspaceFolders;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#abstractMessage
class Message {
    static constexpr std::string_view DEFAULT_JSONRPC = "2.0";

public:
    Message() : jsonrpc(DEFAULT_JSONRPC) {}
    explicit Message(const json& j);

    void toJSON(json& j) const { j["jsonrpc"] = jsonrpc; }
    string jsonrpc;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#requestMessage
class RequestMessage : public Message {
public:
    explicit RequestMessage(const json& j);
    string id;
    string method;
    // Since Param is polymorphic a pointer needs to be stored in order to be able to downcast the
    // type
    std::vector<std::unique_ptr<Param>> params;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#errorCodes
enum class ErrorCode {
    /// Defined by JSON-RPC
    ParseError,
    InvalidRequest,
    MethodNotFound,
    InvalidParams,
    InternalError,
    /// This is the start range of JSON-RPC reserved error codes. It doesn't denote a real error
    /// code.
    /// No LSP error codes should be defined between the start and end range. For backwards
    /// compatibility the `ServerNotInitialized` and the `UnknownErrorCode` are left in the range.
    jsonrpcReservedErrorRangeStart,
    /// @deprecated use jsonrpcReservedErrorRangeStart
    serverErrorStart,
    /// Error code indicating that a server received a notification or
    /// request before the server has received the `initialize` request.
    ServerNotInitialized,
    UnknownErrorCode,
    /// This is the end range of JSON-RPC reserved error codes. It doesn't denote a real error code.
    jsonrpcReservedErrorRangeEnd,
    /// @deprecated use jsonrpcReservedErrorRangeEnd
    serverErrorEnd,
    /// This is the start range of LSP reserved error codes. It doesn't denote a real error code.
    lspReservedErrorRangeStart,
    /// A request failed but it was syntactically correct, e.g the method name was known and the
    /// parameters were valid. The error message should contain human readable information about why
    /// the request failed.
    RequestFailed,
    /// The server cancelled the request. This error code should only be used for requests that
    /// explicitly support being server cancellable.
    ServerCancelled,
    /// The server detected that the content of a document got modified outside normal conditions.
    /// A server should NOT send this error code if it detects a content change in it unprocessed
    /// messages. The result even computed on an older state might still be useful for the client.
    ///
    /// If a client decides that a result is not of any use anymore the client should cancel the
    /// request.
    ContentModified,
    /// The client has canceled a request and a server as detected the cancel.
    RequestCancelled,
    /// This is the end range of LSP reserved error codes. It doesn't denote a real error code.
    lspReservedErrorRangeEnd
};
integer ErrorCodeToInteger(ErrorCode err);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#responseError
class ResponseError {
public:
    void toJSON(json& j) const;

    /// A number indicating the error type that occurred.
    ErrorCode code;
    /// A string providing a short description of the error.
    string message;
    /// A primitive or structured value that contains additional information about the error.
    /// Can be omitted.
    std::optional<json> data;
};

#define KIND(x) x(Initialize)
SLANG_ENUM(ResultKind, KIND)
#undef KIND

class Result {
public:
    explicit Result(ResultKind kind) : kind(kind) {}
    ResultKind kind;

    virtual void toJSON(json& j) const;

    template<typename T>
    decltype(auto) as() {
        SLANG_ASSERT(T::isKind(kind));
        return *static_cast<T*>(this);
    }

    template<typename T>
    const T& as() const {
        return const_cast<Result*>(this)->as<T>();
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#responseMessage
class ResponseMessage : public Message {
public:
    explicit ResponseMessage(int id, std::unique_ptr<Result> result) :
        id(std::move(id)), result(std::move(result)) {}

    explicit ResponseMessage(int id, ResponseError error) :
        id(std::move(id)), error(std::move(error)) {}

    json toJSON() const;

    //string id;
    int id;
    // Since result is polymorphic a pointer needs to be stored in order to be able to downcast the
    // type
    std::optional<std::unique_ptr<Result>> result;
    std::optional<ResponseError> error;
};

/// Defines how the host (editor) should sync document changes to the language server.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentSyncKind
enum class TextDocumentSyncKind {
    /// Documents should not be synced at all.
    None,
    /// Documents are synced by always sending the full content of the document.
    Full,
    /// Documents are synced by sending the full content on open. After that only incremental
    /// updates to the document are sent.
    Incremental
};
uinteger TextDocumentSyncKindToUInteger(TextDocumentSyncKind kind);

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentSyncOptions
class TextDocumentSyncOptions {
public:
    void toJSON(json& j) const;

    /// Open and close notifications are sent to the server. If omitted open
    /// close notifications should not be sent.
    bool openClose;

    /// Change notifications are sent to the server. See TextDocumentSyncKind.None,
    /// TextDocumentSyncKind.Full and TextDocumentSyncKind.Incremental. If omitted it defaults to
    /// TextDocumentSyncKind.None.
    TextDocumentSyncKind change;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workDoneProgressOptions
class WorkDoneProgressOptions {
public:
    virtual void toJSON(json& j) const { j["workDoneProgress"] = workDoneProgress; }
    bool workDoneProgress;
};

/// Completion options.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#completionOptions
class CompletionOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// The additional characters, beyond the defaults provided by the client (typically
    /// [a-zA-Z]), that should automatically trigger a completion request. For example
    /// `.` in JavaScript represents the beginning of an object property or method and is
    /// thus a good candidate for triggering a completion request.
    ///
    /// Most tools trigger a completion request automatically without explicitly
    /// requesting it using a keyboard shortcut (e.g. Ctrl+Space). Typically they
    /// do so when the user starts to type an identifier. For example if the user
    /// types `c` in a JavaScript file code complete will automatically pop up
    /// present `console` besides others as a completion item. Characters that
    /// make up identifiers don't need to be listed here.
    std::vector<string> triggerCharacters;

    /// The list of all possible characters that commit a completion. This field
    /// can be used if clients don't support individual commit characters per
    /// completion item. See client capability
    /// `completion.completionItem.commitCharactersSupport`.
    ///
    /// If a server provides both `allCommitCharacters` and commit characters on
    /// an individual completion item the ones on the completion item win.
    std::vector<string> allCommitCharacters;

    /// The server provides support to resolve additional information for a completion item.
    bool resolveProvider;

    /// The server supports the following `CompletionItem` specific capabilities.
    struct CompletionItem {
        /// The server has support for completion item label
        /// details (see also `CompletionItemLabelDetails`) when receiving
        /// a completion item in a resolve call.
        bool labelDetailsSupport;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(CompletionItem, labelDetailsSupport)
    } completionItem;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#hoverOptions
class HoverOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#signatureHelpOptions
class SignatureHelpOptions : public WorkDoneProgressOptions {
public:
    /// The characters that trigger signature help automatically.
    std::vector<string> triggerCharacters;

    /// List of characters that re-trigger signature help.
    ///
    /// These trigger characters are only active when signature help is already
    /// showing. All trigger characters are also counted as re-trigger characters.
    std::vector<string> retriggerCharacters;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#declarationOptions
class DeclarationOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentFilter
class DocumentFilter {
public:
    json toJSON() const;

    /// A language id, like `typescript`.
    string language;

    /// A Uri [scheme](#Uri.scheme), like `file` or `untitled`.
    string scheme;

    /// A glob pattern, like `*.{ts,js}`.
    ///
    /// Glob patterns can have the following syntax:
    /// - `*` to match one or more characters in a path segment
    /// - `?` to match on one character in a path segment
    /// - `**` to match any number of path segments, including none
    /// - `{}` to group sub patterns into an OR expression. (e.g. `**​/*.{ts,js}`
    ///   matches all TypeScript and JavaScript files)
    /// - `[]` to declare a range of characters to match in a path segment
    ///   (e.g., `example.[0-9]` to match on `example.0`, `example.1`, …)
    /// - `[!...]` to negate a range of characters to match in a path segment
    ///   (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but
    ///   not `example.0`)
    string pattern;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentSelector
using DocumentSelector = std::vector<DocumentFilter>;

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentRegistrationOptions
/// General text document registration options.
class TextDocumentRegistrationOptions {
public:
    void toJSON(json& j) const;

    /// A document selector to identify the scope of the registration. If set to
    /// null the document selector provided on the client side will be used.
    std::optional<DocumentSelector> documentSelector;
};

/// Static registration options to be returned in the initialize request.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#staticRegistrationOptions
class StaticRegistrationOptions {
public:
    void toJSON(json& j) const { j["id"] = id; }
    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    string id;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#declarationRegistrationOptions
class DeclarationRegistrationOptions : public DeclarationOptions,
                                       public TextDocumentRegistrationOptions,
                                       public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        DeclarationOptions::toJSON(j);
        TextDocumentRegistrationOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#definitionOptions
class DefinitionOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#typeDefinitionOptions
class TypeDefinitionOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#typeDefinitionRegistrationOptions
class TypeDefinitionRegistrationOptions : public TextDocumentRegistrationOptions,
                                          public TypeDefinitionOptions,
                                          public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        TypeDefinitionOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#referenceOptions
class ReferenceOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentHighlightOptions
class DocumentHighlightOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentSymbolOptions
class DocumentSymbolOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// A human-readable string that is shown when multiple outlines
    /// trees are shown for the same document.
    string label;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#codeActionOptions
class CodeActionOptions : public WorkDoneProgressOptions {
public:
    /// CodeActionKinds that this server may return.
    ///
    /// The list of kinds may be generic, such as `CodeActionKind.Refactor`,
    /// or the server may list out every specific kind they provide.
    std::vector<CodeActionKind> codeActionKinds;

    /// The server provides support to resolve additional information for a code action.
    bool resolveProvider;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#codeLensOptions
class CodeLensOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// Code lens has a resolve provider as well.
    bool resolveProvider;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#implementationOptions
class ImplementationOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#implementationRegistrationOptions
class ImplementationRegistrationOptions : public TextDocumentRegistrationOptions,
                                          public ImplementationOptions,
                                          public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        ImplementationOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentLinkOptions
class DocumentLinkOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// Document links have a resolve provider as well.
    bool resolveProvider;
};

///@sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentColorOptions
class DocumentColorOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentColorRegistrationOptions
class DocumentColorRegistrationOptions : public TextDocumentRegistrationOptions,
                                         public StaticRegistrationOptions,
                                         public DocumentColorOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
        DocumentColorOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentFormattingOptions
class DocumentFormattingOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentRangeFormattingOptions
class DocumentRangeFormattingOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentOnTypeFormattingOptions
class DocumentOnTypeFormattingOptions {
public:
    void toJSON(json& j) const;

    /// A character on which formatting should be triggered, like `{`.
    string firstTriggerCharacter;

    /// More trigger characters.
    std::vector<string> moreTriggerCharacter;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#renameOptions
class RenameOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// Renames should be checked and tested before being executed.
    bool prepareProvider;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#foldingRangeOptions
class FoldingRangeOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#foldingRangeRegistrationOptions
class FoldingRangeRegistrationOptions : public TextDocumentRegistrationOptions,
                                        public FoldingRangeOptions,
                                        public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        FoldingRangeOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#executeCommandOptions
class ExecuteCommandOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// The commands to be executed on the server
    std::vector<string> commands;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#selectionRangeOptions
class SelectionRangeOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#selectionRangeRegistrationOptions
class SelectionRangeRegistrationOptions : public SelectionRangeOptions,
                                          public TextDocumentRegistrationOptions,
                                          public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        SelectionRangeOptions::toJSON(j);
        TextDocumentRegistrationOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#linkedEditingRangeOptions
class LinkedEditingRangeOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#linkedEditingRangeRegistrationOptions
class LinkedEditingRangeRegistrationOptions : public TextDocumentRegistrationOptions,
                                              public LinkedEditingRangeOptions,
                                              public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        LinkedEditingRangeOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#callHierarchyOptions
class CallHierarchyOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#callHierarchyRegistrationOptions
class CallHierarchyRegistrationOptions : public TextDocumentRegistrationOptions,
                                         public CallHierarchyOptions,
                                         public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        CallHierarchyOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
class SemanticTokensLegend {
public:
    void toJSON(json& j) const;

    /// The token types a server uses.
    std::vector<string> tokenTypes;

    /// The token modifiers a server uses.
    std::vector<string> tokenModifiers;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensOptions
class SemanticTokensOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// The legend used by the server
    SemanticTokensLegend legend;

    /// Server supports providing semantic tokens for a specific range of a document.
    bool range;

    /// Server supports providing semantic tokens for a full document.
    struct Full {
        /// The server supports deltas for full documents.
        bool delta;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Full, delta)
    } full;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensRegistrationOptions
class SemanticTokensRegistrationOptions : public TextDocumentRegistrationOptions,
                                          public SemanticTokensOptions,
                                          public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        SemanticTokensOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#monikerOptions
class MonikerOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#monikerRegistrationOptions
class MonikerRegistrationOptions : public TextDocumentRegistrationOptions, public MonikerOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        MonikerOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#typeHierarchyOptions
class TypeHierarchyOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#typeHierarchyRegistrationOptions
class TypeHierarchyRegistrationOptions : public TextDocumentRegistrationOptions,
                                         public TypeHierarchyOptions,
                                         public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        TypeHierarchyOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// Inline value options used during static registration.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlineValueOptions
class InlineValueOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override { WorkDoneProgressOptions::toJSON(j); }
};

/// Inline value options used during static or dynamic registration.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlineValueRegistrationOptions
class InlineValueRegistrationOptions : public InlineValueOptions,
                                       public TextDocumentRegistrationOptions,
                                       public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override { InlineValueOptions::toJSON(j); }
};

/// Inlay hint options used during static registration.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlayHintOptions
class InlayHintOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// The server provides support to resolve additional information for an inlay hint item.
    bool resolveProvider;
};

/// Inlay hint options used during static or dynamic registration.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#inlayHintRegistrationOptions
class InlayHintRegistrationOptions : public InlayHintOptions,
                                     public TextDocumentRegistrationOptions,
                                     public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        InlayHintOptions::toJSON(j);
        TextDocumentRegistrationOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// Diagnostic options.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#diagnosticOptions
class DiagnosticOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// An optional identifier under which the diagnostics are managed by the client.
    string identifier;

    /// Whether the language has inter file dependencies meaning that
    /// editing code in one file can result in a different diagnostic
    /// set in another file. Inter file dependencies are common for
    /// most programming languages and typically uncommon for linters.
    bool interFileDependencies;

    /// The server provides support for workspace diagnostics as well.
    bool workspaceDiagnostics;
};

/// Diagnostic registration options.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#diagnosticRegistrationOptions
class DiagnosticRegistrationOptions : public TextDocumentRegistrationOptions,
                                      public DiagnosticOptions,
                                      public StaticRegistrationOptions {
public:
    void toJSON(json& j) const override {
        TextDocumentRegistrationOptions::toJSON(j);
        DiagnosticOptions::toJSON(j);
        StaticRegistrationOptions::toJSON(j);
    }
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workspaceSymbolOptions
class WorkspaceSymbolOptions : public WorkDoneProgressOptions {
public:
    void toJSON(json& j) const override;

    /// The server provides support to resolve additional information for a workspace symbol.
    bool resolveProvider;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#workspaceFoldersServerCapabilities
class WorkspaceFoldersServerCapabilities {
public:
    void toJSON(json& j) const;

    /// The server has support for workspace folders
    bool supported;

    /// Whether the server wants to receive workspace folder change notifications.
    ///
    /// If a string is provided, the string is treated as an ID under which the notification
    /// is registered on the client side. The ID can be used to unregister for these events
    /// using the `client/unregisterCapability` request.
    string changeNotifications;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#fileOperationPatternKind
enum class FileOperationPatternKind {
    /// The pattern matches a file only.
    File,
    /// The pattern matches a folder only.
    Folder
};
std::string FileOperationPatternKindToString(FileOperationPatternKind kind);

/// Matching options for the file operation pattern.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#fileOperationPatternOptions
class FileOperationPatternOptions {
public:
    void toJSON(json& j) const;

    /// The pattern should be matched ignoring casing.
    bool ignoreCase;
};

/// A pattern to describe in which file operation requests or notifications the server is interested
/// in.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#fileOperationPattern
class FileOperationPattern {
public:
    void toJSON(json& j) const;

    /// The glob pattern to match. Glob patterns can have the following syntax:
    /// - `*` to match one or more characters in a path segment
    /// - `?` to match on one character in a path segment
    /// - `**` to match any number of path segments, including none
    /// - `{}` to group sub patterns into an OR expression. (e.g. `**​/*.{ts,js}`
    ///   matches all TypeScript and JavaScript files)
    /// - `[]` to declare a range of characters to match in a path segment
    ///   (e.g., `example.[0-9]` to match on `example.0`, `example.1`, …)
    /// - `[!...]` to negate a range of characters to match in a path segment
    ///   (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but
    ///   not `example.0`)
    string glob;

    /// Whether to match files or folders with this pattern.
    ///
    /// Matches both if undefined.
    FileOperationPatternKind matches;

    /// Additional options used during matching.
    FileOperationPatternOptions options;
};

/// A filter to describe in which file operation requests or notifications the server is interested
/// in.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#fileOperationFilter
class FileOperationFilter {
public:
    void toJSON(json& j) const;

    /// A Uri like `file` or `untitled`.
    string scheme;
    /// The actual file operation pattern.
    FileOperationPattern pattern;
};

/// The options to register for file operations.
/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#fileOperationRegistrationOptions
class FileOperationRegistrationOptions {
public:
    void toJSON(json& j) const;

    /// The actual filters.
    std::vector<FileOperationFilter> filters;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#serverCapabilities
class ServerCapabilities {
public:
    void toJSON(json& j) const;

    /// The position encoding the server picked from the encodings offered
    /// by the client via the client capability `general.positionEncodings`.
    ///
    /// If the client didn't provide any position encodings the only valid
    /// value that a server can return is 'utf-16'.
    ///
    /// If omitted it defaults to 'utf-16'.
    PositionEncodingKind positionEncoding{PositionEncodingKind::UTF16};

    /// Defines how text documents are synced. Is either a detailed structure defining each
    /// notification or for backwards compatibility the TextDocumentSyncKind number.
    /// If omitted it defaults to `TextDocumentSyncKind.None`.
    TextDocumentSyncOptions textDocumentSync;

    // TODO: To be implemented
    /// Defines how notebook documents are synced.
    // notebookDocumentSync ?: NotebookDocumentSyncOptions |
    // NotebookDocumentSyncRegistrationOptions;

    /// The server provides completion support.
    CompletionOptions completionProvider;

    /// The server provides hover support.
    HoverOptions hoverProvider;

    /// The server provides signature help support.
    SignatureHelpOptions signatureHelpProvider;

    /// The server provides go to declaration support.
    DeclarationRegistrationOptions declarationProvider;

    /// The server provides goto definition support.
    DefinitionOptions definitionProvider;

    /// The server provides goto type definition support.
    TypeDefinitionRegistrationOptions typeDefinitionProvider;

    /// The server provides goto implementation support.
    ImplementationRegistrationOptions implementationProvider;

    /// The server provides find references support.
    ReferenceOptions referencesProvider;

    /// The server provides document highlight support.
    DocumentHighlightOptions documentHighlightProvider;

    /// The server provides document symbol support.
    DocumentSymbolOptions documentSymbolProvider;

    /// The server provides code actions. The `CodeActionOptions` return type is
    /// only valid if the client signals code action literal support via the
    /// property `textDocument.codeAction.codeActionLiteralSupport`.
    CodeActionOptions codeActionProvider;

    /// The server provides code lens.
    CodeLensOptions codeLensProvider;

    /// The server provides document link support.
    DocumentLinkOptions documentLinkProvider;

    /// The server provides color provider support.
    DocumentColorRegistrationOptions colorProvider;

    /// The server provides document formatting.
    DocumentFormattingOptions documentFormattingProvider;

    /// The server provides document range formatting.
    DocumentRangeFormattingOptions documentRangeFormattingProvider;

    /// The server provides document formatting on typing.
    DocumentOnTypeFormattingOptions documentOnTypeFormattingProvider;

    /// The server provides rename support. RenameOptions may only be specified if the client
    /// states that it supports `prepareSupport` in its initial `initialize` request.
    RenameOptions renameProvider;

    /// The server provides folding provider support.
    FoldingRangeRegistrationOptions foldingRangeProvider;

    /// The server provides execute command support.
    ExecuteCommandOptions executeCommandProvider;

    /// The server provides selection range support.
    SelectionRangeRegistrationOptions selectionRangeProvider;

    /// The server provides linked editing range support.
    LinkedEditingRangeRegistrationOptions linkedEditingRangeProvider;

    /// The server provides call hierarchy support.
    CallHierarchyRegistrationOptions callHierarchyProvider;

    /// The server provides semantic tokens support.
    SemanticTokensRegistrationOptions semanticTokensProvider;

    /// Whether server provides moniker support.
    MonikerRegistrationOptions monikerPr;

    /// The server provides type hierarchy support.
    TypeHierarchyRegistrationOptions typeHierarchyProvider;

    /// The server provides inline values.
    InlineValueRegistrationOptions inlineValueProvider;

    /// The server provides inlay hints.
    InlayHintRegistrationOptions inlayHintProv;

    /// The server has support for pull model diagnostics.
    DiagnosticRegistrationOptions diagnosticProvider;

    /// The server provides workspace symbol support.
    WorkspaceSymbolOptions workspaceSymbolProvider;

    /// Workspace specific server capabilities
    struct Workspace {
        void toJSON(json& j) const;
        // The server supports workspace folder.
        WorkspaceFoldersServerCapabilities workspaceFolders;

        /// The server is interested in file notifications/requests.
        struct FileOperations {
            void toJSON(json& j) const;
            /// The server is interested in receiving didCreateFiles notifications.
            FileOperationRegistrationOptions didCreate;

            /// The server is interested in receiving willCreateFiles requests.
            FileOperationRegistrationOptions willCreate;

            /// The server is interested in receiving didRenameFiles notifications.
            FileOperationRegistrationOptions didRename;

            /// The server is interested in receiving willRenameFiles requests.
            FileOperationRegistrationOptions willRename;

            /// The server is interested in receiving didDeleteFiles file notifications.
            FileOperationRegistrationOptions didDelete;

            /// The server is interested in receiving willDeleteFiles file requests.
            FileOperationRegistrationOptions willDelete;
        } fileOperations;
    } workspace;

    /// Experimental server capabilities.
    std::optional<LSPAny> experimental;
};

/// @sa
/// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initializeResult
class InitializeResult : public Result {
public:
    explicit InitializeResult() :
        Result(ResultKind::Initialize), serverInfo({"slang-lsp", "0.0.1"}) {}

    void toJSON(json& j) const override;
    static bool isKind(ResultKind kind) { return kind == ResultKind::Initialize; }

    /// The capabilities the language server provides.
    ServerCapabilities capabilities;

    /// Information about the server.
    struct ServerInfo {
        /// The name of the server as defined by the server.
        string name;
        /// The server's version as defined by the server.
        string version;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(ServerInfo, name, version)
    } serverInfo;
};

} // namespace slang::rpc