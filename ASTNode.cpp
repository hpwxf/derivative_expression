//
// Created by Pascal Havé on 11/06/2020.
//

#include "ASTNode.hpp"
#include "grammar_symbol.hpp"
#include <iostream>

std::string f(ASTNode &node) {
    if (node.is_root()) return f(*node.children.front());

    if (node.is<language::unary_s2s_function_name>() || node.is<language::unary_v2s_function_name>() ||
        node.is<language::unary_v2v_function_name>()) {
        return node.string() + "(" + f(*node.children[0]) + ")";
    } else if (node.is<language::binary_v2s_function_name>()) {
        return node.string() + "(" + f(*node.children[0]) + "," + f(*node.children[1]) + ")";
    } else if (node.is<language::scalar_variable>() || node.is<language::scalar_constant>() ||
               node.is<language::vector_variable>() || node.is<language::number>() || node.is<language::index>()) {
        return node.content();
    } else if (node.is<language::indexed_vector_variable>()) {
        return f(*node.children[0]) + "_" + f(*node.children[1]);
    } else if (node.is<language::prefix_plus>()) {
        return "+" + f(*node.children[0]);
    } else if (node.is<language::prefix_minus>()) {
        return "-" + f(*node.children[0]);
    } else if (node.is<language::plus>()) {
        return f(*node.children[0]) + "+" + f(*node.children[1]);
    } else if (node.is<language::minus>()) {
        return f(*node.children[0]) + "-" + f(*node.children[1]);
    } else if (node.is<language::multiply>()) {
        return f(*node.children[0]) + "*" + f(*node.children[1]);
    } else if (node.is<language::divide>()) {
        return f(*node.children[0]) + "/" + f(*node.children[1]);
    } else {
        return "undef(" + node.name() + ")";
    }
}
