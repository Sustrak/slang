// SPDX-FileCopyrightText: Michael Popoloski
// SPDX-License-Identifier: MIT

#include "Test.h"

class Callback : public MatcherCallback {
public:
    bool found = false;
    virtual void run(const MatchResult& result) override {
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

    MatchFinder finder;
    Callback callback;
    finder.addMatcher(moduleDeclaration().bind("module"), &callback);

    CHECK(callback.found);
}
