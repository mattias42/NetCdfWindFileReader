#include <WindFieldInterpolation.h>

std::pair<std::vector<double>, std::vector<double>> InterpolateWind(
    const std::vector<double>& u,
    const std::vector<double>& v,
    const std::vector<size_t>& size,
    const std::vector<double>& spatialIndices)
{
    if (size.size() != 4) throw new std::invalid_argument("Invalid data to InterpolateWind, the data must be four-dimensional.");


    return std::pair<std::vector<double>, std::vector<double>>();
}
