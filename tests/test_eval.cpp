//
// Created by Pascal Havé on 16/06/2020.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// doc : https://github.com/catchorg/Catch2/blob/master/docs/Readme.mda
// More example in https://github.com/catchorg/Catch2/tree/master/examples

#include <tao/pegtl/string_input.hpp>
#include "../src/grammar.hpp"
using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

TEST_CASE("Evaluate expression", "[eval]") {
  using record = std::tuple<char const*, Number>;
  const Vector x{1, 2, 3};
  const Number pi = 4 * atan(1);
  const Number e = exp(1);
  auto dot = [](const Vector& a, const Vector& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; };
  auto norm2 = [=](const Vector& a) { return dot(a, a); };
  auto add = [](const Vector& a, const Vector& b) { return Vector{a[0] + b[0], a[1] + b[1], a[2] + b[2]}; };
  auto prod = [](const Number a, const Vector& b) { return Vector{a * b[0], a * b[1], a * b[2]}; };

  auto [expression, expected_value] = GENERATE_COPY(table<char const*, Number>({
      // set of (expression, expected result)
      record{"2", 2},
      record{"2+2", 4},
      record{"2-2*2", 2 - 2 * 2},
      record{"2-(2./6+2)*4", 2 - (2. / 6 + 2) * 4},
      record{"-(+2)", -2},
      record{"exp(2)", exp(2.)},
      record{"dot(x,x)", dot(x, x)},
      record{"norm2(x)", norm2(x)},
      record{"dot(x-x,x+x*2)", 0},
      record{"exp(-dot(x-2*x,-x)/x_2/e)", exp(-dot(add(x, prod(-2, x)), prod(-1, x)) / x[2] / e)}
      //
  }));

  SECTION(expression) {
    string_input in(expression, "valid input expression");
    try {
      const auto root = parse(in);
      std::unique_ptr<IScalarFunction> f = build_function(*root);
      INFO("evaluation of " << expression << " should be " << expected_value);
      REQUIRE(f->apply(x) == expected_value);
    } catch (const parse_error& e) {
      const auto p = e.positions.front();
      std::cout << e.what() << '\n' << in.line_at(p) << '\n' << std::string(p.byte_in_line, ' ') << '^' << std::endl;
      throw;
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }
}
