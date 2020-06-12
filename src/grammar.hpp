//
// Created by Pascal Havé on 11/06/2020.
//

#ifndef LIBKRIGING_PARSER__GRAMMAR_HPP
#define LIBKRIGING_PARSER__GRAMMAR_HPP

#include "ASTNode.hpp"
#include <tao/pegtl/string_input.hpp>

std::unique_ptr<ASTNode> parse(tao::TAO_PEGTL_NAMESPACE::string_input<> & in);


#endif//LIBKRIGING_PARSER__GRAMMAR_HPP
