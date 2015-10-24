#pragma once

namespace slang {

class Token;
class SyntaxNode;

enum class SyntaxKind : uint16_t {
    Unknown,
    List,

    // directives
    BeginKeywordsDirective,
    CellDefineDirective,
    DefaultNetTypeDirective,
    DefineDirective,
    ElseDirective,
    ElseIfDirective,
    EndKeywordsDirective,
    EndCellDefineDirective,
    EndIfDirective,
    IfDefDirective,
    IfNDefDirective,
    IncludeDirective,
    LineDirective,
    NoUnconnectedDriveDirective,
    PragmaDirective,
    ResetAllDirective,
    TimescaleDirective,
    UnconnectedDriveDirective,
    UndefDirective,
    UndefineAllDirective,

    // macros
    MacroUsage,
    MacroFormalArgumentList,
    MacroFormalArgument,
    MacroArgumentDefault,

    // arguments
    OrderedArgument,
    NamedArgument,
    ArgumentList,
    ParameterValueAssignment,

    // patterns
    VariablePattern,
    WildcardPattern,
    ExpressionPattern,
    TaggedPattern,
    OrderedStructurePatternMember,
    NamedStructurePatternMember,
    StructurePattern,
    MatchesClause,
    ConditionalPattern,
    ConditionalPredicate,

    // unary expressions
    UnaryPlusExpression,
    UnaryMinusExpression,
    UnaryBitwiseAndExpression,
    UnaryBitwiseNandExpression,
    UnaryBitwiseOrExpression,
    UnaryBitwiseNorExpression,
    UnaryBitwiseXorExpression,
    UnaryBitwiseXnorExpression,
    UnaryPreincrementExpression,
    UnaryPredecrementExpression,
    LogicalNotExpression,
    BitwiseNotExpression,

    // primary expressions
    NullLiteralExpression,
    StringLiteralExpression,
    IntegerLiteralExpression,
    RealLiteralExpression,
    TimeLiteralExpression,
    WildcardLiteralExpression,
    OneStepLiteralExpression,
    ParenthesizedExpression,
    MinTypMaxExpression,
    EmptyQueueExpression,
    ConcatenationExpression,
    MultipleConcatenationExpression,
    StreamingConcatenationExpression,
    StreamExpression,
    StreamExpressionWithRange,
    NewClassExpression,
    NewArrayExpression,

    // selectors
    BitSelect,
    SimpleRangeSelect,
    AscendingRangeSelect,
    DescendingRangeSelect,
    ElementSelect,

    // postfix expressions
    ElementSelectExpression,
    MemberAccessExpression,
    InvocationExpression,
    PostincrementExpression,
    PostdecrementExpression,

    // binary expressions
    AddExpression,
    SubtractExpression,
    MultiplyExpression,
    DivideExpression,
    PowerExpression,
    ModExpression,
    EqualityExpression,
    InequalityExpression,
    CaseEqualityExpression,
    CaseInequalityExpression,
    WildcardEqualityExpression,
    WildcardInequalityExpression,
    LessThanExpression,
    LessThanEqualExpression,
    GreaterThanExpression,
    GreaterThanEqualExpression,
    LogicalAndExpression,
    LogicalOrExpression,
    BinaryAndExpression,
    BinaryOrExpression,
    BinaryXorExpression,
    BinaryXnorExpression,
    LogicalImplicationExpression,
    LogicalEquivalenceExpression,
    LogicalShiftLeftExpression,
    LogicalShiftRightExpression,
    ArithmeticShiftLeftExpression,
    ArithmeticShiftRightExpression,
    TaggedUnionExpression,
    InsideExpression,
    ConditionalExpression,

    // assignment expressions
    AssignmentExpression,
    AddAssignmentExpression,
    SubtractAssignmentExpression,
    MultiplyAssignmentExpression,
    DivideAssignmentExpression,
    ModAssignmentExpression,
    AndAssignmentExpression,
    OrAssignmentExpression,
    XorAssignmentExpression,
    LogicalLeftShiftAssignmentExpression,
    LogicalRightShiftAssignmentExpression,
    ArithmeticLeftShiftAssignmentExpression,
    ArithmeticRightShiftAssignmentExpression,

    // names
    LocalScope,
    UnitScope,
    RootScope,
    IdentifierName,
    IdentifierSelectName,
    ClassName,
    ScopedName,
    SystemName,
    ThisHandle,
    SuperHandle,
    ClassScope,

    // timing control
    DelayControl,
    CycleDelay,
    EventControl,
    IffClause,
    SignalEventExpression,
    BinaryEventExpression,
    ParenthesizedEventExpression,
    ImplicitEventControl,
    ParenImplicitEventControl,
    EventControlWithExpression,
    RepeatedEventControl,

    // statements
    EmptyStatement,
    ElseClause,
    ConditionalStatement,
    DefaultCaseItem,
    PatternCaseItem,
    StandardCaseItem,
    CaseStatement,
    ForeverStatement,
    LoopStatement,
    DoWhileStatement,
    ReturnStatement,
    JumpStatement,
    TimingControlStatement,

    // assignment statements
    NonblockingAssignmentStatement,
    BlockingAssignmentStatement,
    AddAssignmentStatement,
    SubtractAssignmentStatement,
    MultiplyAssignmentStatement,
    DivideAssignmentStatement,
    ModAssignmentStatement,
    AndAssignmentStatement,
    OrAssignmentStatement,
    XorAssignmentStatement,
    LogicalLeftShiftAssignmentStatement,
    LogicalRightShiftAssignmentStatement,
    ArithmeticLeftShiftAssignmentStatement,
    ArithmeticRightShiftAssignmentStatement
};

enum class TokenKind : uint16_t;

SyntaxKind getUnaryPrefixExpression(TokenKind kind);
SyntaxKind getUnaryPostfixExpression(TokenKind kind);
SyntaxKind getLiteralExpression(TokenKind kind);
SyntaxKind getBinaryExpression(TokenKind kind);
SyntaxKind getKeywordNameExpression(TokenKind kind);
SyntaxKind getAssignmentStatement(TokenKind kind);
int getPrecedence(SyntaxKind kind);
bool isRightAssociative(SyntaxKind kind);
bool isPossibleExpression(TokenKind kind);

std::ostream& operator<<(std::ostream& os, SyntaxKind kind);

// discriminated union of Token and SyntaxNode
struct TokenOrSyntax {
    union {
        Token* token;
        SyntaxNode* node;
    };
    bool isToken;

    TokenOrSyntax(Token* token) : token(token), isToken(true) {}
    TokenOrSyntax(SyntaxNode* node) : node(node), isToken(false) {}
    TokenOrSyntax(std::nullptr_t) : token(nullptr), isToken(true) {}
};

// base class for all syntax nodes
class SyntaxNode {
public:
    uint32_t childCount = 0;
    SyntaxKind kind;

    SyntaxNode(SyntaxKind kind) : kind(kind) {}

    // convenience methods that wrap writeTo
    // toFullString() includes trivia, toString() does not
    std::string toString();
    std::string toFullString();

    void writeTo(Buffer<char>& buffer, bool includeTrivia, bool includeMissing = false);
    Token* getFirstToken();

    template<typename T>
    T* as() {
        // TODO: assert kind
        return static_cast<T*>(this);
    }

    virtual TokenOrSyntax getChild(uint32_t);
};

template<typename T>
class SyntaxList : public SyntaxNode {
public:
    SyntaxList(ArrayRef<T*> elements) :
        SyntaxNode(SyntaxKind::List), 
        elements(elements)
    {
        childCount = elements.count();
    }

    uint32_t count() const { return elements.count(); }

    T* const* begin() const { return elements.begin(); }
    T* const* end() const { return elements.end(); }

    const T* operator[](uint32_t index) const { return elements[index]; }

protected:
    TokenOrSyntax getChild(uint32_t index) override final { return elements[index]; }

private:
    ArrayRef<T*> elements;
};

class TokenList : public SyntaxNode {
public:
    TokenList(ArrayRef<Token*> elements) :
        SyntaxNode(SyntaxKind::List),
        elements(elements)
    {
        childCount = elements.count();
    }

    uint32_t count() const { return elements.count(); }

    Token* const* begin() const { return elements.begin(); }
    Token* const* end() const { return elements.end(); }

    const Token* operator[](uint32_t index) const { return elements[index]; }

protected:
    TokenOrSyntax getChild(uint32_t index) override final { return elements[index]; }

private:
    ArrayRef<Token*> elements;
};

template<typename T>
class SeparatedSyntaxList : public SyntaxNode {
public:
    SeparatedSyntaxList(ArrayRef<TokenOrSyntax> elements) :
        SyntaxNode(SyntaxKind::List), 
        elements(elements)
    {
        childCount = elements.count();
    }

    uint32_t count() const { return (uint32_t)std::ceil(elements.count() / 2.0); }

    const T* operator[](uint32_t index) const {
        index <<= 1;
        ASSERT(!elements[index].isToken);
        return static_cast<const T*>(elements[index].node);
    }

protected:
    TokenOrSyntax getChild(uint32_t index) override final { return elements[index]; }

private:
    ArrayRef<TokenOrSyntax> elements;
};

}