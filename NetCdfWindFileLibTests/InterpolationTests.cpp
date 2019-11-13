#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <algorithm>
#include <WindFieldInterpolation.h>

TEST_CASE("GetFractionalIndex increasing values, finds correct quarter points", "[GetFractionalIndex]")
{
    std::vector<float> input = { 0.0, 1.0, 2.0 };

    auto result1 = GetFractionalIndex(input, 0.25F);
    auto result2 = GetFractionalIndex(input, 0.75F);
    auto result3 = GetFractionalIndex(input, 1.75F);

    REQUIRE(result1 == 0.25);
    REQUIRE(result2 == 0.75);
    REQUIRE(result3 == 1.75);
}

TEST_CASE("GetFractionalIndex dcreasing values, finds correct quarter points", "[GetFractionalIndex]")
{
    std::vector<float> input = { 2.0, 1.0, 0.0 };

    auto result1 = GetFractionalIndex(input, 0.25F);
    auto result2 = GetFractionalIndex(input, 0.75F);
    auto result3 = GetFractionalIndex(input, 1.75F);

    REQUIRE(result1 == 2.0 - 0.25);
    REQUIRE(result2 == 2.0 - 0.75);
    REQUIRE(result3 == 2.0 - 1.75);
}

TEST_CASE("TriLinearInterpolation unit cube with all ones, returns one", "[TriLinearInterpolation]")
{
    std::vector<double> input = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    auto result = TriLinearInterpolation(input, 0.0, 0.0, 0.0);

    REQUIRE(result.value == 1.0);
    REQUIRE(result.uncertainty == 0.0);
}

TEST_CASE("TriLinearInterpolation returns correct value at single corner", "[TriLinearInterpolation]")
{
    std::vector<double> input = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };

    REQUIRE(1.0 == TriLinearInterpolation(input, 0.0, 0.0, 0.0).value);
    REQUIRE(2.0 == TriLinearInterpolation(input, 0.0, 0.0, 1.0).value);
    REQUIRE(3.0 == TriLinearInterpolation(input, 0.0, 1.0, 0.0).value);
    REQUIRE(4.0 == TriLinearInterpolation(input, 0.0, 1.0, 1.0).value);
    REQUIRE(5.0 == TriLinearInterpolation(input, 1.0, 0.0, 0.0).value);
    REQUIRE(6.0 == TriLinearInterpolation(input, 1.0, 0.0, 1.0).value);
    REQUIRE(7.0 == TriLinearInterpolation(input, 1.0, 1.0, 0.0).value);
    REQUIRE(8.0 == TriLinearInterpolation(input, 1.0, 1.0, 1.0).value);
}

TEST_CASE("Returns vector with one value per time", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 2, 2, 2 };
    std::vector<float> u(48);
    std::vector<float> v(48);
    std::vector<double> indices(3);

    InterpolatedWind result;
    InterpolateWind(u, v, size, indices, result);

    REQUIRE(result.speed.size() == 6);
    REQUIRE(result.direction.size() == 6);
}

TEST_CASE("Unit-wind field, returns all wind-speeds of sqrt_2", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 2, 2, 2 };
    std::vector<float> u(48);
    std::fill_n(begin(u), 48, 1.0F);
    std::vector<float> v(48);
    std::fill_n(begin(v), 48, 1.0F);
    std::vector<double> indices(3);

    InterpolatedWind result;
    InterpolateWind(u, v, size, indices, result);

    REQUIRE(result.speed.size() == 6);
    REQUIRE(result.speed[0] == std::sqrt(2.0));
    REQUIRE(result.speed[3] == std::sqrt(2.0));
    REQUIRE(result.speed[5] == std::sqrt(2.0));
}

TEST_CASE("Unit-wind field, returns all wind-directions of -135 degrees", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 2, 2, 2 };
    std::vector<float> u(48);
    std::fill_n(begin(u), 48, 1.0F);
    std::vector<float> v(48);
    std::fill_n(begin(v), 48, 1.0F);
    std::vector<double> indices(3);

    InterpolatedWind result;
    InterpolateWind(u, v, size, indices, result);

    REQUIRE(result.direction.size() == 6);
    REQUIRE(result.direction[0] == -135.0);
    REQUIRE(result.direction[3] == -135.0);
    REQUIRE(result.direction[5] == -135.0);
}

TEST_CASE("Unit-wind field, returns all wind-speed errors of zero (no gradient in wind field)", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 2, 2, 2 };
    std::vector<float> u(48);
    std::fill_n(begin(u), 48, 1.0F);
    std::vector<float> v(48);
    std::fill_n(begin(v), 48, 1.0F);
    std::vector<double> indices(3);

    InterpolatedWind result;
    InterpolateWind(u, v, size, indices, result);

    REQUIRE(result.speedError.size() == 6);
    REQUIRE(result.speedError[0] == 0.0);
    REQUIRE(result.speedError[3] == 0.0);
    REQUIRE(result.speedError[5] == 0.0);
}

TEST_CASE("Unit-wind field, returns all wind-direction errors of zero (no gradient in wind field)", "[InterpolateWind]")
{
    std::vector<size_t> size = { 6, 2, 2, 2 };
    std::vector<float> u(48);
    std::fill_n(begin(u), 48, 1.0F);
    std::vector<float> v(48);
    std::fill_n(begin(v), 48, 1.0F);
    std::vector<double> indices(3);

    InterpolatedWind result;
    InterpolateWind(u, v, size, indices, result);

    REQUIRE(result.directionError.size() == 6);
    REQUIRE(result.directionError[0] == 0.0);
    REQUIRE(result.directionError[3] == 0.0);
    REQUIRE(result.directionError[5] == 0.0);
}