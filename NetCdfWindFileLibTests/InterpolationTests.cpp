#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <WindFieldInterpolation.h>

TEST_CASE("Returns vector with one value per time", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 1, 1, 1 };
    std::vector<double> u(6);
    std::vector<double> v(6);
    std::vector<double> indices(3);

    auto result = InterpolateWind(u, v, size, indices);

    REQUIRE(result.first.size() == 6);
    REQUIRE(result.second.size() == 6);
}
