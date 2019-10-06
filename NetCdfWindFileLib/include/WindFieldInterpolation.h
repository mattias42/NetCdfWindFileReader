#pragma once
#include <vector>

// Returns the (first) index into the provided vector where the valueToFind lies between
//  the value before and the value after.
//  This assumes that values is a one-dimensional vector
//  But does not assume that values is sorted in any way.
//  @throws std::invalid_argument if the value cannot be found.
double GetFractionalIndex(const std::vector<float>& values, float valueToFind);

// Interpolates between the values floor(index) and ceil(index)
//  in the provided one-dimensional vector.
//  @throws std::invalid_argument if index < 0 or index >= values.size();
double Interpolate(const std::vector<float>& values, double index);

// Performs a tri-linear interpolation on the input values, which must have the dimensions 2x2x2
//  at the index values (which all must be in the interval [0,1])
double TriLinearInterpolation(const std::vector<double>& inputCube, double idxZ, double idxY, double idxX); 

/** Performs a linear interpolation to retrieve the (wind speed, wind direction)
    from the provided wind-field for all points in time.
    @param u The u-component of the wind-field.
    @param v The v-component of the wind-field.
    @param size The size of u and v in each dimension.
    @param spatialIndices The indices to interpolate for in the spatial dimensions.
    This assumes that the first dimension of the data is time and the remaining 
        three dimensions are spatial dimensions.
    @throws invalid_argument if u and v are not four-dimensional matrices.
    @return two vector:s with the wind speeds and wind directions (in degrees)
        the number of values equals the size[0].
    */
std::pair<std::vector<double>, std::vector<double>> InterpolateWind(
    const std::vector<double>& u,
    const std::vector<double>& v,
    const std::vector<size_t>& size,
    const std::vector<double>& spatialIndices);

