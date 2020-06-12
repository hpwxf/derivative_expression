// Copyright (c) 2017-2020 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#include <iostream>
#include <string>

#include "Demangle.hpp"
#include "ASTNode.hpp"
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>
#include <tao/pegtl/string_input.hpp>

#include "grammar.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " EXPR\n"
                  << "Generate a 'dot' file from expression.\n\n"
                  << "Example: " << argv[0] << " \"(2*a + 3*b) / (4*n)\" | dot -Tpng -o parse_tree.png\n";
        return 1;
    }

    string_input in(argv[1], "from command line");
    try {
        const auto root = parse(in);
        parse_tree::print_dot(std::cout, *root);
        std::cerr << f(*root) << std::endl;
        
        return 0;
    } catch (const parse_error &e) {
        const auto p = e.positions.front();
        std::cerr << e.what() << std::endl
                  << in.line_at(p) << std::endl
                  << std::string(p.byte_in_line, ' ') << '^' << std::endl;
    } catch (const std::exception &e) { std::cerr << e.what() << std::endl; }
    return 1;
}
