#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


#define CATCH_CONFIG_MAIN
#if __has_include(<catch2/catch.hpp>)
#include <catch2/catch.hpp>
#else
#include <catch.hpp>
#endif

#include <sticky.hpp>

SCENARIO("Decoding Sticky data")
{
    GIVEN("Some randomly shifted data packet")
    {
        WHEN("data is valid")
        {
            THEN("Loading file returns nullopt")
            {
                //REQUIRE(cdf::io::load("wrongfile.cdf") == std::nullopt);
            }
        }
    }
}
