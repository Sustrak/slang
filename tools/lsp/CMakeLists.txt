# ~~~
# SPDX-FileCopyrightText: Michael Popoloski
# SPDX-License-Identifier: MIT
# ~~~

# FetchContent_Declare(
#         kissnet
#         GIT_REPOSITORY https://github.com/Ybalrid/kissnet
#         GIT_TAG adbfe52d5147b206dd68c83da15b6e5805fe28dd
#         GIT_SHALLOW ON)
# FetchContent_MakeAvailable(kissnet)
#
# if(NOT TARGET kissnet::kissnet)
#     message(
#             FATAL_ERROR "Could not find kissnet package")
# endif()

add_library(slang_lsp_obj_lib OBJECT src/Server.cpp src/RPC.cpp)

target_include_directories(slang_lsp_obj_lib PUBLIC include ../../include)
target_link_libraries(slang_lsp_obj_lib PUBLIC slang::slang)

add_executable(slang_lsp src/lsp.cpp)
add_executable(slang::lsp ALIAS slang_lsp)

target_link_libraries(slang_lsp PRIVATE slang_lsp_obj_lib)
set_target_properties(slang_lsp PROPERTIES OUTPUT_NAME "slang-lsp")

if(SLANG_INCLUDE_INSTALL)
    install(TARGETS slang_lsp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_executable(my_test test.cpp)
target_compile_features(my_test PRIVATE cxx_std_17)

