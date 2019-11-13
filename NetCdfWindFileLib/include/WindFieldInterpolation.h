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

struct EstimatedValue
{
    double value;
    double uncertainty;
};

// Performs a tri-linear interpolation on the input values, which must have the dimensions 2x2x2
//  at the index values (which all must be in the interval [0,1])
EstimatedValue TriLinearInterpolation(const std::vector<double>& inputCube, double idxZ, double idxY, double idxX);

struct InterpolatedWind
{
    std::vector<double> speed;
    std::vector<double> speedError;
    std::vector<double> direction; // [degrees]
    std::vector<double> directionError; // [degrees]
    std::vector<double> cloudCoverage; // fractional cloud coverage in the range [0, 1]
    std::vector<double> relativeHumidity; // [%]
};

/** Performs a linear interpolation to retrieve the
    wind speed, wind speed error, wind direction and wind direction error
    from the provided wind-field for all points in time.
    @param u The u- (eastward) component of the wind-field.
    @param v The v- (northward) component of the wind-field.
    @param size The size of u and v in each dimension.
    @param spatialIndices The indices to interpolate for in the spatial dimensions.
    This assumes that the first dimension of the data is time and the remaining 
        three dimensions are spatial dimensions.
    @throws invalid_argument if u and v are not four-dimensional matrices.
    @return two vector:s with the wind speeds and wind directions (in degrees)
        the number of values equals size[0].
    */
void InterpolateWind(
    const std::vector<float>& u,
    const std::vector<float>& v,
    const std::vector<size_t>& sizes,
    const std::vector<double>& spatialIndices,
    InterpolatedWind& result);

/** Performs a linear interpolation to retrieve values from the given four-dimensional
    vector at all points in time for the provided spatial indices.
    This differens from the function 'InterpolateWind' in that no values are calculated,
    only pure interpolation is performed.
    @throws invalid_argument if values is not a four-dimensional matrix.
    */
void InterpolateValue(
    const std::vector<float>& values,
    const std::vector<size_t>& sizes,
    const std::vector<double>& spatialIndices,
    std::vector<double>& result);

