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
    explicit BindableMatcher(const MatcherImp& matcher) {
        auto matcherCopy = std::make_shared<MatcherImp>(matcher);
        matcherFunc = [matcherCopy](const Symbol& node, ASTContext& context,
                                    BoundNodesMap& boundNodes) {
            if (const NodeType* specificNode = node.as_if<NodeType>()) {
                return matcherCopy->matches(*specificNode, context, boundNodes);
            }
            return false;
        };
    }

    explicit BindableMatcher(std::function<bool(const Symbol&, ASTContext&, BoundNodesMap&)>&& func,
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

class VarDeclMatcher final : public ComposableMatcher<VariableSymbol> {
public:
    VarDeclMatcher() = default;

    template<typename... MatcherArgs>
    explicit VarDeclMatcher(MatcherArgs... matchers) : ComposableMatcher(matchers...) {}

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

class MatcherCallback {
public:
    virtual ~MatcherCallback() = default;
    virtual void run(const MatchResult& result) = 0;
};

class MatchFinder {};

} // namespace slang::ast::matchers