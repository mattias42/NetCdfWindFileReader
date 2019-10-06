#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <algorithm>
#include <WindFieldInterpolation.h>

TEST_CASE("TriLinearInterpolation unit cube with all ones, returns one", "[TriLinearInterpolation]")
{
    std::vector<double> input = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    auto result = TriLinearInterpolation(input, 0.0, 0.0, 0.0);

    REQUIRE(result == 1.0);
}

TEST_CASE("TriLinearInterpolation returns correct value at single corner", "[TriLinearInterpolation]")
{
    std::vector<double> input = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };

    REQUIRE(1.0 == TriLinearInterpolation(input, 0.0, 0.0, 0.0));
    REQUIRE(2.0 == TriLinearInterpolation(input, 0.0, 0.0, 1.0));
    REQUIRE(3.0 == TriLinearInterpolation(input, 0.0, 1.0, 0.0));
    REQUIRE(4.0 == TriLinearInterpolation(input, 0.0, 1.0, 1.0));
    REQUIRE(5.0 == TriLinearInterpolation(input, 1.0, 0.0, 0.0));
    REQUIRE(6.0 == TriLinearInterpolation(input, 1.0, 0.0, 1.0));
    REQUIRE(7.0 == TriLinearInterpolation(input, 1.0, 1.0, 0.0));
    REQUIRE(8.0 == TriLinearInterpolation(input, 1.0, 1.0, 1.0));
}

TEST_CASE("Returns vector with one value per time", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 2, 2, 2 };
    std::vector<float> u(48);
    std::vector<float> v(48);
    std::vector<double> indices(3);

    auto result = InterpolateWind(u, v, size, indices);

    REQUIRE(result.first.size() == 6);
    REQUIRE(result.second.size() == 6);
}

TEST_CASE("Unit-wind field, returns all wind-speeds of sqrt_2", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 2, 2, 2 };
    std::vector<float> u(48);
    std::fill_n(begin(u), 48, 1.0F);
    std::vector<float> v(48);
    std::fill_n(begin(v), 48, 1.0F);
    std::vector<double> indices(3);

    auto result = InterpolateWind(u, v, size, indices);

    REQUIRE(result.first.size() == 6);
    REQUIRE(result.first[0] == std::sqrt(2.0));
    REQUIRE(result.first[3] == std::sqrt(2.0));
    REQUIRE(result.first[5] == std::sqrt(2.0));
}