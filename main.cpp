// Copyright (c) 2017-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#include <iostream>
#include <string>
#include <type_traits>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>

using namespace tao::TAO_PEGTL_NAMESPACE;// NOLINT

namespace language {
    // the grammar

    // clang-format off
    struct index : plus< digit > {};
    struct number : seq< opt< one< '+', '-' > >,
            seq<
                    plus< digit >,
                    opt< seq< one < '.' >, star< digit > > >
            >,
            opt<
                    seq<
                            one< 'E', 'e' >,
                            opt< one< '+', '-' > >,
                            plus<digit>
                    >
            >
    >{};

    struct plus : pad< one< '+' >, space > {};
    struct minus : pad< one< '-' >, space > {};
    struct multiply : pad< one< '*' >, space > {};
    struct divide : pad< one< '/' >, space > {};
    struct power : pad< one< '^' >, space > {};
    
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

    struct open_bracket : seq< one< '(' >, star< space > > {};
    struct close_bracket : seq< star< space >, one< ')' > > {};

    struct scalar_expression;
    struct vector_expression;
    struct nullary_a2s_function : if_must < nullary_a2s_function_name, open_bracket, close_bracket > {};
    struct unary_s2s_function : seq < unary_s2s_function_name, open_bracket, scalar_expression, close_bracket > {};
    struct unary_v2s_function : seq < unary_v2s_function_name, open_bracket, vector_expression, close_bracket > {};
    struct unary_v2v_function : if_must < unary_v2v_function_name, open_bracket, vector_expression, close_bracket > {};
    struct binary_v2s_function : if_must < binary_v2s_function_name, open_bracket, vector_expression, comma, vector_expression, close_bracket > {};
    struct scalar_function : sor < nullary_a2s_function, unary_s2s_function, unary_v2s_function, binary_v2s_function > {};
    struct vector_function : sor < unary_v2v_function > {};
    struct scalar_bracketed : seq< open_bracket, scalar_expression, close_bracket > {};
    struct vector_bracketed : seq< open_bracket, vector_expression, close_bracket > {};
    struct indexed_vector_variable : seq < vector_variable, one<'_'>, index > {};
    struct scalar_factor : sor< scalar_bracketed, scalar_function, scalar_variable, indexed_vector_variable, scalar_constant, number >{};
    struct vector_factor : sor< vector_bracketed, vector_function, vector_variable >{};
    struct prefix_op : sor< plus, minus > {};
    struct scalar_term : seq< 
            opt< prefix_op >, 
            scalar_factor, 
            star<seq<sor< multiply, divide >, scalar_factor> >
        > {};
    struct vector_term : seq<
            opt<sor< plus, minus >>,
            star<seq<scalar_factor, multiply>>, 
            vector_factor, 
            star<seq<sor< multiply, divide >, scalar_factor >>
        > {};
    struct infix_op : sor< plus, minus > {};
    struct scalar_expression : list_must< scalar_term, infix_op > {};
    struct vector_expression : list_must< vector_term, infix_op > {};

    struct grammar : must< scalar_expression, eof > {};
    // clang-format on

    // after a node is stored successfully, you can add an optional transformer like this:
    struct rearrange : parse_tree::apply<rearrange>// allows bulk selection, see selector<...>
    {
        // recursively rearrange nodes. the basic principle is:
        //
        // from:          TERM/EXPR
        //                /   |   \          (LHS... may be one or more children, followed by OP,)
        //             LHS... OP   RHS       (which is one operator, and RHS, which is a single child)
        //
        // to:               OP
        //                  /  \             (OP now has two children, the original TERM/EXPR and RHS)
        //         TERM/EXPR    RHS          (Note that TERM/EXPR has two fewer children now)
        //             |
        //            LHS...
        //
        // if only one child is left for LHS..., replace the TERM/EXPR with the child directly.
        // otherwise, perform the above transformation, then apply it recursively until LHS...
        // becomes a single child, which then replaces the parent node and the recursion ends.
        template<typename... States>
        static void transform(std::unique_ptr<parse_tree::node> &n, States &&... st) {
            if (n->children.size() == 0) {
                ;// noop
            } else if (n->children.size() == 1) {
                if (n->is<scalar_expression>()
                    || n->is<vector_expression>()
                    || n->is<scalar_term>()
                    || n->is<vector_term>()) {
                    n = std::move(n->children.back());
                }
            } else {
                auto &c = n->children;
                n->remove_content();
                auto r = std::move(c.back());
                c.pop_back();
                auto o = std::move(c.back());
                c.pop_back();

                if (o->is<prefix_op>()) {
                    o->children.emplace_back(std::move(r));
                    n = std::move(o);
                } else {
                    o->children.emplace_back(std::move(n));
                    o->children.emplace_back(std::move(r));
                    n = std::move(o);
                }
                if (n->children.size() > 0) { transform(n->children.front(), st...); }
            }
        }
    };

    // after a node is stored successfully, you can add an optional transformer like this:
    struct collapse_function_name : parse_tree::apply<collapse_function_name>// allows bulk selection, see selector<...>
    {
        // recursively rearrange nodes. the basic principle is:
        //
        // from:          FUNCTION
        //                /      \       (FUNCTION content is full sub-expression)
        //       FUNCTION_NAME  ARGS...
        //
        // to:         FUNCTION_NAME
        //                  /  \         (now FUNCTION content is its name)
        //                 ARGS...
        template<typename... States>
        static void transform(std::unique_ptr<parse_tree::node> &n, States &&... st) {
            assert(n->is<nullary_a2s_function>()
                   || n->is<unary_s2s_function>()
                   || n->is<unary_v2s_function>()
                   || n->is<unary_v2v_function>()
                   || n->is<binary_v2s_function>());
            auto &c = n->children;
            auto f = std::move(c[0]);
            for(std::size_t i=1;i<c.size();++i)
                f->children.emplace_back(std::move(c[i]));
            n = std::move(f);
        }
    };


    // select which rules in the grammar will produce parse tree nodes:

    template<typename Rule>
    using selector = parse_tree::selector<
            Rule,
            parse_tree::store_content::on<index, number, prefix_op, infix_op, scalar_constant, scalar_variable,
                                          vector_variable,
                                          // scalar_function, vector_function,
                                          nullary_a2s_function_name, unary_s2s_function_name, unary_v2s_function_name, unary_v2v_function_name, binary_v2s_function_name
                                          >,
            parse_tree::remove_content::on<multiply, divide>,
            rearrange::on<scalar_term, vector_term, scalar_expression, vector_expression>,
            collapse_function_name::on<nullary_a2s_function, unary_s2s_function, unary_v2s_function, unary_v2v_function, binary_v2s_function>
        >;

}// namespace language

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " EXPR\n"
                  << "Generate a 'dot' file from expression.\n\n"
                  << "Example: " << argv[0] << " \"(2*a + 3*b) / (4*n)\" | dot -Tpng -o parse_tree.png\n";
        return 1;
    }

    if (analyze<language::grammar>() != 0) {
        std::cerr << "there are problems" << std::endl;
        return 1;
    }

    argv_input<> in(argv, 1);
    try {
        const auto root = parse_tree::parse<language::grammar, language::selector>(in);
        parse_tree::print_dot(std::cout, *root);

        return 0;
    } catch (const parse_error &e) {
        const auto p = e.positions.front();
        std::cerr << e.what() << std::endl
                  << in.line_at(p) << std::endl
                  << std::string(p.byte_in_line, ' ') << '^' << std::endl;
    } catch (const std::exception &e) { std::cerr << e.what() << std::endl; }
    return 1;
}
