//
// Created by Pascal Havé on 11/06/2020.
//

#ifndef LIBKRIGING_PARSER__GRAMMAR_SYMBOL_HPP
#define LIBKRIGING_PARSER__GRAMMAR_SYMBOL_HPP

#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/rules.hpp>
#include <tao/pegtl/internal/pegtl_string.hpp>

namespace language {
    
    // clang-format off
struct index : plus< digit > {};
// number are always positive; prefix op is matched for prefix sign
struct number : seq< seq< plus< digit >,
                          opt< seq< one < '.' >, star< digit > > >
                        >,
                     opt< seq< one< 'E', 'e' >,
                               opt< one< '+', '-' > >,
                               plus<digit>
                              >
                        >
                   >{};

struct ignored : space {};
struct plus : pad< one< '+' >, ignored > {};
struct minus : pad< one< '-' >, ignored > {};
struct multiply : pad< one< '*' >, ignored > {};
struct divide : pad< one< '/' >, ignored > {};
struct prefix_plus : seq< star< ignored >, one< '+' > > {};
struct prefix_minus : seq< star< ignored >, one< '-' > > {};

struct rand_func : TAO_PEGTL_STRING( "rand" ) {};
struct exp_func : TAO_PEGTL_STRING( "exp" ) {};
struct sqrt_func : TAO_PEGTL_STRING( "sqrt" ) {};
struct norm2_func : TAO_PEGTL_STRING( "norm2" ) {};
struct dot_func : TAO_PEGTL_STRING( "dot" ) {};
struct abs_func : TAO_PEGTL_STRING( "abs" ) {};
struct sum_func : TAO_PEGTL_STRING( "sum" ) {};

struct nullary_a2s_function_name : sor< rand_func > {};
struct unary_s2s_function_name : sor< exp_func, sqrt_func, abs_func > {};
struct unary_v2s_function_name : sor< norm2_func, sum_func > {};
struct unary_v2v_function_name : sor< abs_func > {};
struct binary_v2s_function_name : sor< dot_func > {};
struct comma: pad< one< ',' >, space > {};

struct x_var : TAO_PEGTL_STRING("x") {};
struct y_var : TAO_PEGTL_STRING("y") {};
struct vector_variable : sor< x_var, y_var > {};
struct pi_constant : TAO_PEGTL_STRING("pi") {};
struct e_constant : TAO_PEGTL_STRING("e") {};
struct scalar_constant : sor< pi_constant, e_constant > {};

struct restricted_identifier : sor < vector_variable, nullary_a2s_function_name, unary_s2s_function_name, unary_v2s_function_name, unary_v2v_function_name, binary_v2s_function_name, scalar_constant > {};
struct scalar_variable : seq< not_at< restricted_identifier >, identifier > {};
struct indexed_vector_variable : seq < vector_variable, one<'_'>, index > {};
    // clang-format on
}

#endif//LIBKRIGING_PARSER__GRAMMAR_SYMBOL_HPP
