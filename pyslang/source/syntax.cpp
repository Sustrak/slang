//------------------------------------------------------------------------------
// pyslang.cpp
// File is under the MIT license; see LICENSE for details
//------------------------------------------------------------------------------
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "slang/parsing/Parser.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/SourceManager.h"

namespace py = pybind11;
using namespace pybind11::literals;
using namespace slang;

void registerSyntax(py::module_& m) {
    py::class_<Trivia>(m, "Trivia")
        .def(py::init<>())
        .def(py::init<TriviaKind, string_view>())
        .def_readonly("kind", &Trivia::kind)
        .def("getExplicitLocation", &Trivia::getExplicitLocation)
        .def("syntax", &Trivia::syntax)
        .def("getRawText", &Trivia::getRawText)
        .def("getSkippedTokens", &Trivia::getSkippedTokens);

    py::class_<Token>(m, "Token")
        .def(py::init<>())
        .def(py::init<BumpAllocator&, TokenKind, span<Trivia const>, string_view, SourceLocation>())
        .def(py::init<BumpAllocator&, TokenKind, span<Trivia const>, string_view, SourceLocation,
                      string_view>())
        .def(py::init<BumpAllocator&, TokenKind, span<Trivia const>, string_view, SourceLocation,
                      SyntaxKind>())
        .def(py::init<BumpAllocator&, TokenKind, span<Trivia const>, string_view, SourceLocation,
                      logic_t>())
        .def(py::init<BumpAllocator&, TokenKind, span<Trivia const>, string_view, SourceLocation,
                      const SVInt&>())
        .def(py::init<BumpAllocator&, TokenKind, span<Trivia const>, string_view, SourceLocation,
                      double, bool, optional<TimeUnit>>())
        .def(py::init<BumpAllocator&, TokenKind, span<Trivia const>, string_view, SourceLocation,
                      LiteralBase, bool>())
        .def_property_readonly("isMissing", &Token::isMissing)
        .def_property_readonly("range", &Token::range)
        .def_property_readonly("location", &Token::location)
        .def_property_readonly("trivia", &Token::trivia)
        .def_property_readonly("valueText", &Token::valueText)
        .def_property_readonly("rawText", &Token::rawText)
        .def_property_readonly("isOnSameLine", &Token::isOnSameLine)
        .def_property_readonly("valid", &Token::valid)
        .def_property_readonly("value",
                               [](const Token& t) -> py::object {
                                   switch (t.kind) {
                                       case TokenKind::IntegerLiteral:
                                           return py::cast(t.intValue());
                                       case TokenKind::RealLiteral:
                                       case TokenKind::TimeLiteral:
                                           return py::cast(t.realValue());
                                       case TokenKind::UnbasedUnsizedLiteral:
                                           return py::cast(t.bitValue());
                                       case TokenKind::StringLiteral:
                                       case TokenKind::Identifier:
                                           return py::cast(t.valueText());
                                       default:
                                           return py::none();
                                   }
                               })
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<SyntaxTree>(m, "SyntaxTree")
        .def_readwrite("isLibrary", &SyntaxTree::isLibrary)
        .def_static("fromFile", py::overload_cast<string_view>(&SyntaxTree::fromFile))
        .def_static("fromText",
                    py::overload_cast<string_view, string_view, string_view>(&SyntaxTree::fromText),
                    "text"_a, "name"_a = "source", "path"_a = "")
        .def("diagnostics", &SyntaxTree::diagnostics)
        .def("sourceManager", py::overload_cast<>(&SyntaxTree::sourceManager))
        .def("root", py::overload_cast<>(&SyntaxTree::root))
        .def("options", &SyntaxTree::options)
        .def("getMetadata", &SyntaxTree::getMetadata)
        .def_static("getDefaultSourceManager", &SyntaxTree::getDefaultSourceManager);
}
