#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

// doc : https://github.com/catchorg/Catch2/blob/master/docs/Readme.mda
// More example in https://github.com/catchorg/Catch2/tree/master/examples

TEST_CASE("Trivial test", "[core]") {
  SECTION("checking trivial test ok") { REQUIRE((2 == 2)); }
}
