//
// Created by Pascal Havé on 11/06/2020.
//

#include "grammar.hpp"

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include "ASTNode.hpp"
#include "grammar_symbol.hpp"

using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

namespace language {  // the grammar

// clang-format off

struct open_bracket : pad< one< '(' >, ignored > {};
struct close_bracket : pad< one< ')' >, ignored > {};

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
struct scalar_factor : sor< scalar_bracketed, scalar_function, scalar_variable, indexed_vector_variable, scalar_constant, number >{};
struct vector_factor : sor< vector_bracketed, vector_function, vector_variable >{};
struct scalar_term : list< scalar_factor, sor< multiply, divide > > {};
struct vector_term : seq< star< seq< scalar_factor,
                                     multiply> >,
                          vector_factor,
                          star< seq< sor< multiply, divide >, 
                                     scalar_factor > >
                        > {};
struct scalar_expression : seq< opt< sor< prefix_plus, prefix_minus> >,
                                list_must< scalar_term, sor<plus,minus> >
                              > {};
struct vector_expression : seq< opt< sor< prefix_plus, prefix_minus> >,
                                list_must< vector_term, sor<plus,minus> >
                              > {};
struct grammar : must< scalar_expression, eof > {};
// clang-format on

// after a node is stored successfully, you can add an optional transformer like this:
struct rearrange
    : parse_tree::apply<rearrange>  // allows bulk selection, see selector<...>
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
  template <typename... States>
  static void transform(std::unique_ptr<ASTNode>& n, States&&... st) {
    // return; // premature return for debugging without rearrange 
    if (n->children.size() == 0) {
      ;  // noop
    } else if (n->children.size() == 1) {
      if (n->is<scalar_expression>() || n->is<vector_expression>() || n->is<scalar_term>() || n->is<vector_term>()) {
        n = std::move(n->children.back());
      }
    } else {
      auto& c = n->children;
      n->remove_content();
      auto r = std::move(c.back());
      c.pop_back();
      auto o = std::move(c.back());
      c.pop_back();

      //      std::cerr << "-----\n";
      //      std::cerr << "n=" << n->name() << " : " << ((n->has_content()) ? n->string() : "") << "\n";
      //      std::cerr << "o=" << o->name() << " : " << ((o->has_content()) ? o->string() : "") << "\n";
      //      std::cerr << "r=" << r->name() << " : " << ((r->has_content()) ? r->string() : "") << "\n";

      if (o->is<prefix_plus>() || o->is<prefix_minus>()) {
        assert(c.empty());
        o->children.emplace_back(std::move(r));
        n = std::move(o);
      } else {
        assert(o->is<plus>() || o->is<minus>() || o->is<multiply>() || o->is<divide>());
        o->children.emplace_back(std::move(n));
        o->children.emplace_back(std::move(r));
        n = std::move(o);
        if (n->children.size() > 0) {
          transform(n->children.front(), st...);
        }
      }
    }
  }
};

// after a node is stored successfully, you can add an optional transformer like this:
struct collapse_function_name
    : parse_tree::apply<collapse_function_name>  // allows bulk selection, see selector<...>
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
  template <typename... States>
  static void transform(std::unique_ptr<ASTNode>& n, States&&... st) {
    assert(n->is<nullary_a2s_function>() || n->is<unary_s2s_function>() || n->is<unary_v2s_function>()
           || n->is<unary_v2v_function>() || n->is<binary_v2s_function>());
    auto& c = n->children;
    auto f = std::move(c[0]);
    for (std::size_t i = 1; i < c.size(); ++i)
      f->children.emplace_back(std::move(c[i]));
    n = std::move(f);
  }
};

// select which rules in the grammar will produce parse tree nodes:
template <typename Rule>
using selector = parse_tree::selector<
    Rule,
    parse_tree::store_content::on<index,
                                  number,
                                  scalar_constant,
                                  scalar_variable,
                                  indexed_vector_variable,
                                  vector_variable,
                                  // scalar_function, vector_function,
                                  nullary_a2s_function_name,
                                  unary_s2s_function_name,
                                  unary_v2s_function_name,
                                  unary_v2v_function_name,
                                  binary_v2s_function_name>,
    parse_tree::remove_content::on<multiply, divide, plus, minus, prefix_plus, prefix_minus>,
    rearrange::on<scalar_term, vector_term, scalar_expression, vector_expression>,
    collapse_function_name::
        on<nullary_a2s_function, unary_s2s_function, unary_v2s_function, unary_v2v_function, binary_v2s_function>>;

}  // namespace language

std::unique_ptr<ASTNode> parse(string_input<>& in) {
  if (analyze<language::grammar>() != 0) {
    std::cerr << "there are problems in grammar" << std::endl;
    return {};
  }

  return parse_tree::parse<language::grammar, ASTNode, language::selector>(in);
}
