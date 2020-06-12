#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// doc : https://github.com/catchorg/Catch2/blob/master/docs/Readme.mda
// More example in https://github.com/catchorg/Catch2/tree/master/examples

#include <tao/pegtl/string_input.hpp>
#include "../src/grammar.hpp"
using namespace tao::TAO_PEGTL_NAMESPACE;  // NOLINT

TEST_CASE("Trivial test", "[core]") {
  SECTION("checking trivial test ok") { REQUIRE((2 == 2)); }
}

TEST_CASE("Echo valid input expression", "[echo]") {
  auto e = GENERATE("exp(2)");

  string_input in(e, "input expression");
  try {
    const auto root = parse(in);
    std::unique_ptr<IFunction> a = f(*root);
    REQUIRE(e == a->string());
  } catch (const parse_error& e) {
    const auto p = e.positions.front();
    std::cerr << e.what() << std::endl
              << in.line_at(p) << std::endl
              << std::string(p.byte_in_line, ' ') << '^' << std::endl;
    throw;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}