// Copyright (c) 2017-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#include <iostream>
#include <string>
#include <type_traits>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>

using namespace tao::TAO_PEGTL_NAMESPACE;// NOLINT

namespace example {
    // the grammar

    // clang-format off
    struct index : plus< digit > {};
    struct number : plus< digit > {};
    struct scalar_var : identifier {};

    struct x_var : TAO_PEGTL_STRING("x") {};
    struct y_var : TAO_PEGTL_STRING("y") {};
    struct vector_var : sor< x_var, y_var > {};

    struct plus : pad< one< '+' >, space > {};
    struct minus : pad< one< '-' >, space > {};
    struct multiply : pad< one< '*' >, space > {};
    struct divide : pad< one< '/' >, space > {};
    struct power : pad< one< '^' >, space > {};
    
    struct rand_func : TAO_PEGTL_STRING( "rand" ) {};
    struct exp_func : TAO_PEGTL_STRING( "exp" ) {};
    struct sqrt_func : TAO_PEGTL_STRING( "sqrt" ) {};
    struct norm_func : TAO_PEGTL_STRING( "norm" ) {};
    struct dot_func : TAO_PEGTL_STRING( "dot" ) {};
    
    struct noary_function_name : sor< rand_func > {};
    struct unary_function_name : sor< exp_func, sqrt_func, norm_func > {};
    struct binary_function_name : sor< dot_func > {};
    struct comma: pad< one< ',' >, space > {};

    struct open_bracket : seq< one< '(' >, star< space > > {};
    struct close_bracket : seq< star< space >, one< ')' > > {};

    struct expression;
    struct noary_function : if_must < noary_function_name, open_bracket, close_bracket > {};
    struct unary_function : if_must < unary_function_name, open_bracket, expression, close_bracket > {};
    struct binary_function : if_must < binary_function_name, open_bracket, expression, comma, expression, close_bracket > {};
    struct function : sor < noary_function, unary_function, binary_function > {};
    struct bracketed : if_must< open_bracket, expression, close_bracket > {};
    struct factor : sor< number, bracketed, function, scalar_var >{};
    struct term : list_must< factor, sor< multiply, divide > > {};
    struct expression : list_must< term, sor< plus, minus > > {};

    struct grammar : must< expression, eof > {};
    // clang-format on

    // after a node is stored successfully, you can add an optional transformer like this:
    struct rearrange
        : parse_tree::apply<rearrange>// allows bulk selection, see selector<...>
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
            if (n->children.size() == 1) {
                n = std::move(n->children.back());
            } else {
                n->remove_content();
                auto &c = n->children;
                auto r = std::move(c.back());
                c.pop_back();
                auto o = std::move(c.back());
                c.pop_back();
                o->children.emplace_back(std::move(n));
                o->children.emplace_back(std::move(r));
                n = std::move(o);
                transform(n->children.front(), st...);
            }
        }
    };

    // select which rules in the grammar will produce parse tree nodes:

    template<typename Rule>
    using selector = parse_tree::selector<
            Rule,
            parse_tree::store_content::on<
                    number,
                    scalar_var,
                    function>,
            parse_tree::remove_content::on<
                    plus,
                    minus,
                    multiply,
                    divide>,
            rearrange::on<
                    term,
                    expression>
            >;

}// namespace example

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " EXPR\n"
                  << "Generate a 'dot' file from expression.\n\n"
                  << "Example: " << argv[0] << " \"(2*a + 3*b) / (4*n)\" | dot -Tpng -o parse_tree.png\n";
        return 1;
    }
    argv_input<> in(argv, 1);
    try {
        const auto root = parse_tree::parse<example::grammar, example::selector>(in);
        parse_tree::print_dot(std::cout, *root);
        return 0;
    } catch (const parse_error &e) {
        const auto p = e.positions.front();
        std::cerr << e.what() << std::endl
                  << in.line_at(p) << std::endl
                  << std::string(p.byte_in_line, ' ') << '^' << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
