#include <WindFieldInterpolation.h>

std::pair<std::vector<double>, std::vector<double>> InterpolateWind(
    const std::vector<float>& u,
    const std::vector<float>& v,
    const std::vector<size_t>& sizes,
    const std::vector<double>& spatialIndices)
{
    if (sizes.size() != 4) throw new std::invalid_argument("Invalid data to InterpolateWind, the data must be four-dimensional.");
    if (spatialIndices.size() != 3) throw new std::invalid_argument("Invalid data to InterpolateWind, there must be three spatial dimensions.");

    std::vector<double> finalWindSpeeds(sizes[0]);
    std::vector<double> finalWindDirections(sizes[0]);

    // temporary variables in the loop below.
    std::vector<double> uValues(8);
    std::vector<double> vValues(8);
    std::vector<double> windSpeedTemp(8);
    std::vector<double> windDirTemp(8);

    // Dimensions are [time, level, lat, lon]
    for (size_t timeIdx = 0; timeIdx < sizes[0]; ++timeIdx)
    {
        // ----------- Pick out the neighoring u- and v- values at this point in time -----------
        // -----------      this is a small cube with 2x2x2 values     -----------
        size_t dim2Floor = (size_t)std::floor(spatialIndices[2]);
        size_t dim1Floor = (size_t)std::floor(spatialIndices[1]);
        size_t dim0Floor = (size_t)std::floor(spatialIndices[0]);

        for (size_t lvlIdx = dim0Floor; lvlIdx <= dim0Floor + 1; ++lvlIdx)
        {
            for (size_t latIdx = dim1Floor; latIdx <= dim1Floor + 1; ++latIdx)
            {
                for (size_t lonIdx = dim2Floor; lonIdx <= dim2Floor + 1; ++lonIdx)
                {
                    size_t index = ((timeIdx * sizes[1] + lvlIdx) * sizes[2] + latIdx) * sizes[3] + lonIdx;

                    size_t minorIndex = ((lvlIdx - dim0Floor) * 2 + (latIdx - dim1Floor)) * 2 + (lonIdx - dim2Floor);

                    uValues[minorIndex] = u[index];
                    vValues[minorIndex] = v[index];
                }
            }
        }

        // Calculate the wind-speed and wind-direction at each corner in the cube
        for (size_t ii = 0; ii < 8; ++ii)
        {
            windSpeedTemp[ii] = std::sqrt(uValues[ii] * uValues[ii] + vValues[ii] * vValues[ii]);
            windDirTemp[ii] = 180.0 * std::atan2(-uValues[ii], -vValues[ii]) / 3.14159265358979323846;
        }

        // Now perform a tri-linear interpolation inside this cube with wind-speed values to calculate
        //  the inerpolated wind-speed
        finalWindSpeeds[timeIdx] = TriLinearInterpolation(windSpeedTemp, spatialIndices[0] - dim0Floor, spatialIndices[1] - dim1Floor, spatialIndices[2] - dim2Floor);
        finalWindDirections[timeIdx] = TriLinearInterpolation(windDirTemp, spatialIndices[0] - dim0Floor, spatialIndices[1] - dim1Floor, spatialIndices[2] - dim2Floor);
    }

    return std::pair<std::vector<double>, std::vector<double>>(finalWindSpeeds, finalWindDirections);
}
