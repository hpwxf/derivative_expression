#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// doc : https://github.com/catchorg/Catch2/blob/master/docs/Readme.mda
// More example in https://github.com/catchorg/Catch2/tree/master/examples

#include <tao/pegtl/string_input.hpp>
#include "../src/grammar.hpp"
using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

TEST_CASE("Echo valid input expression", "[diff][echo]") {
  using record = std::tuple<char const*, char const*, Number>;
  const Vector x{1, 2, 3};
  const std::size_t diff_index = 0;

  const Number pi = 4 * atan(1);
  const Number e = exp(1);
  auto dot = [](const Vector& a, const Vector& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; };
  auto norm2 = [=](const Vector& a) { return dot(a, a); };
  auto add = [](const Vector& a, const Vector& b) { return Vector{a[0] + b[0], a[1] + b[1], a[2] + b[2]}; };
  auto prod = [](const Number a, const Vector& b) { return Vector{a * b[0], a * b[1], a * b[2]}; };

  auto [expression, expected_diff, expected_diff_value] = GENERATE_COPY(table<char const*, const char*, Number>({
      // set of (expression, expected result)
      record{"2", "0", 0},
      record{"pi", "0", 0},
      record{"pi", "0", 0},
      record{"x_1", "0", 0},
      record{"x_0", "1", 1},
      record{"2*x_0", "2*1+0*x_0", 2},
      record{"x_0*x_1", "x_0*0+1*x_1", x[1]},
      record{"x_0*x_0+x_1*x_1", "x_0*1+1*x_0+x_1*0+0*x_1", 2 * x[diff_index]},
      record{"dot(x,x)", "dot(<x_0=1>,x)+dot(x,<x_0=1>)", 2 * x[diff_index]},
      record{"exp(x_0)", "exp(x_0)*1", exp(x[diff_index])},
      record{"exp(-0.5 * dot(x,x))",
             "exp(-0.5*dot(x,x))*-(0.5*(dot(<x_0=1>,x)+dot(x,<x_0=1>))+0*dot(x,x))",
             exp(-0.5 * dot(x, x)) * -x[diff_index]},
      //
      record{"0", "0", 0}
      //
  }));

  SECTION(expression) {
    string_input in(expression, "valid input expression");
    try {
      const auto root = parse(in);
      std::unique_ptr<IScalarFunction> f = build_function(*root);
      std::unique_ptr<IScalarFunction> df = f->diff(diff_index);
      INFO("diff_0 of " << expression << " should be " << expected_diff);
      REQUIRE(df->string() == expected_diff);
      INFO("eval of diff_0 of " << expression << " should be " << expected_diff_value);
      REQUIRE(df->apply(x) == expected_diff_value);
    } catch (const parse_error& e) {
      const auto p = e.positions.front();
      std::cout << e.what() << '\n' << in.line_at(p) << '\n' << std::string(p.byte_in_line, ' ') << '^' << std::endl;
      throw;
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }
}
