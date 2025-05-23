// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT

#include "Test.h"
#include "slang/matchers/Matchers.h"

using namespace slang::ast::matchers;

class Callback : public MatcherCallback {
public:
    bool found = false;
    void run(const MatchResult&  /*result*/) override {
        found = true;
    }
};

TEST_CASE("Match basic") {
    auto tree = SyntaxTree::fromText(R"(
module top();
    logic a;
endmodule
)");

    Compilation compilation;
    compilation.addSyntaxTree(tree);
    NO_COMPILATION_ERRORS;

    auto context = ASTContext(compilation.createScriptScope(), LookupLocation::max);
    MatchFinder finder(context);
    Callback callback;
    finder.addMatcher(moduleDeclaration().bind("module"), &callback);

    CHECK(callback.found);
}
