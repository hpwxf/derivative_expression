#include <iostream>
#include <string>

#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>
#include "src/ASTNode.hpp"
#include "src/Demangle.hpp"
using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

#include "src/grammar.hpp"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " EXPR\n"
              << "Generate a 'dot' file from expression.\n\n"
              << "Example: " << argv[0] << " \"(2*a + 3*b) / (4*n)\" | dot -Tpng -o parse_tree.png\n";
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    string_input in(argv[i], "from command line");
    try {
      const auto root = parse(in);
      if (argc == 2) parse_tree::print_dot(std::cout, *root);
      std::unique_ptr<IScalarFunction> a = build_function(*root);
      std::cerr << a->string() << std::endl;

      Vector x = {1, 2, 3};
      std::cerr << "f({1,2}) = " << a->apply(x) << std::endl;
    } catch (const parse_error& e) {
      const auto p = e.positions.front();
      std::cerr << e.what() << std::endl
                << in.line_at(p) << std::endl
                << std::string(p.byte_in_line, ' ') << '^' << std::endl;
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
  return 1;
}
