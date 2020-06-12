#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// doc : https://github.com/catchorg/Catch2/blob/master/docs/Readme.mda
// More example in https://github.com/catchorg/Catch2/tree/master/examples

#include <tao/pegtl/string_input.hpp>
#include "../src/grammar.hpp"
using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

TEST_CASE("Echo valid input expression", "[echo]") {
  auto e = GENERATE("exp(2)",  //
                    "exp(-pi*a-e*norm2(x)+dot(x-y,x)*x_2)",
                    "exp ( -pi * a - e * norm2 ( x ) + dot ( x - y , x ) * x_2 ) ");

  SECTION(e) {
    string_input in(e, "valid input expression");
    try {
      const auto root = parse(in);
      std::string expr_without_blank = e;
      expr_without_blank.erase(
          std::remove_if(expr_without_blank.begin(), expr_without_blank.end(), isspace),
          expr_without_blank.end());

      std::unique_ptr<IFunction> out = f(*root);
      REQUIRE(expr_without_blank == out->string());
    } catch (const parse_error& e) {
      const auto p = e.positions.front();
      std::cout << e.what() << '\n' << in.line_at(p) << '\n' << std::string(p.byte_in_line, ' ') << '^' << std::endl;
      throw;
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }
}

TEST_CASE("Invalid input expression", "[!shouldfail]") {
  auto e = GENERATE("exp(--2)",  // invalid double --
                    "x",         // vector expression (requires scalar expression)
                    "exp(x)"     // scalar function on vector
  );
  SECTION(e) {
    string_input in(e, "invalid input expression");
    try {
      parse(in);
    } catch (const parse_error& e) {
      const auto p = e.positions.front();
      std::cerr << e.what() << '\n' << in.line_at(p) << '\n' << std::string(p.byte_in_line, ' ') << '^' << std::endl;
      throw;
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}
