//
// Created by Pascal Havé on 11/06/2020.
//

#include "ASTNode.hpp"
#include "grammar_symbol.hpp"
#include <iostream>
#include <utility>

class Function : public IFunction {
public:
    explicit Function(std::string s) : m_string(std::move(s)) {}

public:
    [[nodiscard]] std::string string() const override { return m_string; }

private:
    std::string m_string;
};


std::unique_ptr<IFunction> f(ASTNode &node) {
    if (node.is_root()) return f(*node.children.front());

    if (node.is<language::unary_s2s_function_name>() || node.is<language::unary_v2s_function_name>() ||
        node.is<language::unary_v2v_function_name>()) {
        return std::make_unique<Function>(node.string() + "(" + f(*node.children[0])->string() + ")");
    } else if (node.is<language::binary_v2s_function_name>()) {
        return std::make_unique<Function>(node.string() + "(" + f(*node.children[0])->string() + "," +
                                          f(*node.children[1])->string() + ")");
    } else if (node.is<language::scalar_variable>() || node.is<language::scalar_constant>() ||
               node.is<language::vector_variable>() || node.is<language::number>() || node.is<language::index>()) {
        return std::make_unique<Function>(node.content());
    } else if (node.is<language::indexed_vector_variable>()) {
        return std::make_unique<Function>(f(*node.children[0])->string() + "_" + f(*node.children[1])->string());
    } else if (node.is<language::prefix_plus>()) {
        return std::make_unique<Function>("+" + f(*node.children[0])->string());
    } else if (node.is<language::prefix_minus>()) {
        return std::make_unique<Function>("-" + f(*node.children[0])->string());
    } else if (node.is<language::plus>()) {
        return std::make_unique<Function>(f(*node.children[0])->string() + "+" + f(*node.children[1])->string());
    } else if (node.is<language::minus>()) {
        return std::make_unique<Function>(f(*node.children[0])->string() + "-" + f(*node.children[1])->string());
    } else if (node.is<language::multiply>()) {
        return std::make_unique<Function>(f(*node.children[0])->string() + "*" + f(*node.children[1])->string());
    } else if (node.is<language::divide>()) {
        return std::make_unique<Function>(f(*node.children[0])->string() + "/" + f(*node.children[1])->string());
    } else {
        return std::make_unique<Function>("undef(" + node.name() + ")");
    }
}
