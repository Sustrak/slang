//------------------------------------------------------------------------------
//! @file Matchers.h
//! @brief AST Matchers
//
// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT
//------------------------------------------------------------------------------

#pragma once
#include <map>
#include <string>

#include "slang/ast/ASTVisitor.h"
#include "slang/ast/Symbol.h"
#include "slang/ast/symbols/VariableSymbols.h"

namespace slang::ast::matchers {

using BoundNodesMap = std::map<std::string, const Symbol*>;

class MatchResult {
public:
    explicit MatchResult(const BoundNodesMap& boundNodes, ASTContext& context) :
        boundNodes(boundNodes), context(context) {}

    template<typename T>
    std::optional<const T*> getNodeAs(const std::string& id) const {
        if (const auto it = boundNodes.find(id); it != boundNodes.end()) {
            return it->second->as<const T>();
        }
        return {};
    }

private:
    const BoundNodesMap& boundNodes;
    ASTContext& context;
};

/**
 * Base matcher class, all matcher shall derive from it
 * @tparam NodeType AST Node the matcher is matching against
 */
template<typename NodeType>
class Matcher {
public:
    virtual ~Matcher() = default;

    virtual bool matches(const NodeType& node, ASTContext& context,
                         BoundNodesMap& boundNodes) const = 0;
};

/**
 * Base matcher class, that allows to `bind()` the result of the match to an ID
 */
class BindableMatcher {
public:
    template<typename NodeType, typename MatcherImp>
    BindableMatcher(const MatcherImp& matcher) {
        auto matcherCopy = std::make_shared<MatcherImp>(matcher);
        matcherFunc = [matcherCopy](const Symbol& node, ASTContext& context,
                                    BoundNodesMap& boundNodes) {
            if (const NodeType* specificNode = node.as_if<NodeType>()) {
                return matcherCopy->matches(*specificNode, context, boundNodes);
            }
            return false;
        };
    }

    BindableMatcher(std::function<bool(const Symbol&, ASTContext&, BoundNodesMap&)>&& func,
                             const std::string&& id) : matcherFunc(func), bindId(id) {}

    bool matches(const Symbol& symbol, ASTContext& context, BoundNodesMap& boundNodes) const {
        const bool match = matcherFunc(symbol, context, boundNodes);
        if (match && !bindId.empty()) {
            boundNodes[bindId] = &symbol;
        }
        return match;
    }

    BindableMatcher bind(const std::string&& id) {
        return BindableMatcher(std::move(matcherFunc), std::forward<const std::string>(id));
    }

private:
    std::function<bool(const Symbol&, ASTContext&, BoundNodesMap&)> matcherFunc;
    const std::string bindId;
};

template<typename NodeType>
class GenericMatcher final : public Matcher<NodeType> {
public:
    bool matches(const Symbol& node, ASTContext&, BoundNodesMap&) const override {
        return node.as_if<NodeType>() != nullptr;
    }
};

template<typename NodeType>
class ComposableMatcher : public Matcher<NodeType> {
public:
    template<typename... Matchers>
    explicit ComposableMatcher(Matchers... matchers) : matchers({BindableMatcher(matchers)...}) {}

protected:
    std::vector<BindableMatcher> matchers;
};

class VarDeclMatcher final : public Matcher<VariableSymbol> {
public:
    VarDeclMatcher() = default;
    std::vector<BindableMatcher> matchers;

    template<typename... MatcherArgs>
    explicit VarDeclMatcher(MatcherArgs... matchers) : matchers(matchers...) {}

    bool matches(const VariableSymbol& varDecl, ASTContext& context,
                 BoundNodesMap& boundNodes) const override {
        for (const auto& matcher : matchers) {
            if (!matcher.matches(varDecl, context, boundNodes)) {
                return false;
            }
        }
        return true;
    }
};

inline BindableMatcher varDecl() {
    return BindableMatcher(VarDeclMatcher());
}
// varDecl(innerMatcher1, innerMatcher2, ...)
template<typename... MatcherArgs>
BindableMatcher varDecl(MatcherArgs... innerMatchers) {
    return BindableMatcher(VarDeclMatcher(innerMatchers...));
}

class MatcherCallback {
public:
    virtual ~MatcherCallback() = default;
    virtual void run(const MatchResult& result) = 0;
};

class MatchFinder {
    struct MatcherEntry {
        BindableMatcher matcher;
        MatcherCallback* callback;
    };
    std::vector<MatcherEntry> _matchers;
    ASTContext _context; // Our simple context

public:
    explicit MatchFinder(const ASTContext& context) : _context(context) {}

    void addMatcher(const BindableMatcher& matcher, MatcherCallback* callback) {
        _matchers.push_back({matcher, callback});
    }

    void match(const Symbol& rootNode, const Symbol* parent = nullptr) {
        // Traverse the AST (pre-order traversal)
        // For each node, try all registered matchers

        for (const auto& [matcher, callback] : _matchers) {
            BoundNodesMap boundNodes; // Fresh for each matcher attempt on a node

            bool isMatch = false;
            // Special handling for HasParentMatcher (this is a simplification)
            // In real Clang, this is more integrated.
            // We'd need to try to downcast entry.matcher to a known HasParentMatcher wrapper
            // or have the DynamicMatcher itself expose a way to call this.
            // For this example, let's assume we can't easily get the typed HasParentMatcher back
            // from DynamicMatcher without more type information.
            // A real implementation would make DynamicMatcher richer or use RTTI/visitor pattern.
            //
            // HACKY WAY: Try to see if the matcher's internal function behaves like hasParent
            // This is not robust. A better way is to have specific logic for types of matchers.
            // For this example, we'll assume hasParent is matched like any other matcher,
            // but its internal logic relies on the parent being available IF it were called
            // differently. The current HasParentMatcher::matches will print a warning.
            //
            // Let's refine: if a matcher is a HasParentMatcher, we need to call it differently.
            // This means DynamicMatcher needs to be able to expose its underlying type or
            // we need a different registration mechanism for such context-aware matchers.

            // For now, let's assume a simplified hasParent that only checks the parent directly.
            // The `hasParent` factory would construct a matcher that, when `matches` is called on
            // a node, it somehow gets the parent from a global traversal context (bad) or
            // it is simply not supported in this simplified model without major changes to
            // MatchFinder.

            // Let's stick to the current simpler model: `hasParent` will use its standard `matches`
            // which will fail/warn. To make it work, `MatchFinder::match` would need to pass
            // `parent` to a special method on `DynamicMatcher` if it wraps a `HasParentMatcher`.
            // This gets complex quickly.

            // Simpler: if the matcher is `HasParentMatcher<T>`, call its special method.
            // How to know? We can't directly from DynamicMatcher easily.
            // Clang's system is more sophisticated with internal types.

            // Let's assume for this example, `hasParent` matchers are run, but they might not work
            // as intended without the parent context being explicitly plumbed. We will focus on
            // direct node properties for reliable matching in this example.

            if (matcher.matches(rootNode, _context, boundNodes)) {
                MatchResult result(boundNodes, _context);
                callback->run(result);
            }
        }

        rootNode.visit(makeVisitor([&](auto&, const Symbol& child) { match(child, &rootNode); }));
    }
};

} // namespace slang::ast::matchers