add_library(parser
        ASTNode.cpp ASTNode.hpp
        Demangle.cpp Demangle.hpp
        grammar.hpp grammar.cpp grammar_symbol.hpp)

if (CXX_CLANG_TIDY)
    set_target_properties(parser
            PROPERTIES
            CXX_CLANG_TIDY ${CXX_CLANG_TIDY})
endif ()

target_link_libraries(parser)

target_include_directories(parser
    PUBLIC
        # path of the headers after installation
        $<INSTALL_INTERFACE:include>
        # path of the headers before installation
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # path of the generated headers before installation
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)