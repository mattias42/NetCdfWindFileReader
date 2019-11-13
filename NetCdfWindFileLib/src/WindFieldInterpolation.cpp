#include <WindFieldInterpolation.h>
#include <assert.h>

EstimatedValue TriLinearInterpolation(const std::vector<double>& inputCube, double idxX, double idxY, double idxZ)
{
    double c00 = inputCube[0] * (1.0 - idxZ) + inputCube[1] * idxZ;
    double c01 = inputCube[2] * (1.0 - idxZ) + inputCube[3] * idxZ;
    double c10 = inputCube[4] * (1.0 - idxZ) + inputCube[5] * idxZ;
    double c11 = inputCube[6] * (1.0 - idxZ) + inputCube[7] * idxZ;

    double c0 = c00 * (1.0 - idxY) + c01 * idxY;
    double c1 = c10 * (1.0 - idxY) + c11 * idxY;

    EstimatedValue result;
    result.value = c0 * (1.0 - idxX) + c1 * idxX;
    result.uncertainty = c1 - c0;
    return result;
}

void SelectCubeValues(const std::vector<float>& tensor, const std::vector<size_t>& tensorDimensions, const std::vector<size_t>& floorIdx, std::vector<double>& corners)
{
    assert(corners.size() == 8);

    const size_t timeDim = 0;
    const size_t lvlDim = 1;
    const size_t latDim = 2;
    const size_t lonDim = 3;

    // ----------- Pick out the neighoring u- and v- values at this point in time -----------
    // -----------      this is a small cube with 2x2x2 values     -----------
    for (size_t lvlIdx = floorIdx[lvlDim]; lvlIdx <= floorIdx[lvlDim] + 1; ++lvlIdx)
    {
        for (size_t latIdx = floorIdx[latDim]; latIdx <= floorIdx[latDim] + 1; ++latIdx)
        {
            for (size_t lonIdx = floorIdx[lonDim]; lonIdx <= floorIdx[lonDim] + 1; ++lonIdx)
            {
                size_t index = ((floorIdx[timeDim] * tensorDimensions[lvlDim] + lvlIdx) * tensorDimensions[latDim] + latIdx) * tensorDimensions[lonDim] + lonIdx;

                size_t minorIndex = ((lvlIdx - floorIdx[lvlDim]) * 2 + (latIdx - floorIdx[latDim])) * 2 + (lonIdx - floorIdx[lonDim]);

                corners[minorIndex] = tensor[index];
            }
        }
    }

}

InterpolatedWind InterpolateWind(
    const std::vector<float>& u,
    const std::vector<float>& v,
    const std::vector<size_t>& sizes,
    const std::vector<double>& spatialIndices)
{
    if (sizes.size() != 4) throw new std::invalid_argument("Invalid data to InterpolateWind, the data must be four-dimensional.");
    if (spatialIndices.size() != 3) throw new std::invalid_argument("Invalid data to InterpolateWind, there must be three spatial dimensions.");

    // defining the dimensions
    const size_t timeDim = 0;
    const size_t lvlDim = 1;
    const size_t latDim = 2;
    const size_t lonDim = 3;

    const size_t lvlFloor = (size_t)std::floor(spatialIndices[0]);
    const size_t latFloor = (size_t)std::floor(spatialIndices[1]);
    const size_t lonFloor = (size_t)std::floor(spatialIndices[2]);

    std::vector<size_t> floorIdx = { 0, lvlFloor, latFloor, lonFloor };

    std::vector<double> finalWindSpeeds(sizes[timeDim]);
    std::vector<double> finalWindSpeedErrors(sizes[timeDim]);
    std::vector<double> finalWindDirections(sizes[timeDim]);
    std::vector<double> finalWindDirectionErrors(sizes[timeDim]);

    // temporary variables in the loop below.
    std::vector<double> uValues(8);
    std::vector<double> vValues(8);
    std::vector<double> windSpeedTemp(8);
    std::vector<double> windDirTemp(8);

    // Dimensions are [time, level, latitude, longitude]
    for (size_t timeIdx = 0; timeIdx < sizes[timeDim]; ++timeIdx)
    {
        floorIdx[timeDim] = timeIdx;

        // ----------- Pick out the neighoring u- and v- values at this point in time -----------
        // -----------      this is a small cube with 2x2x2 values     -----------
        SelectCubeValues(u, sizes, floorIdx, uValues);
        SelectCubeValues(v, sizes, floorIdx, vValues);

        // Calculate the wind-speed and wind-direction at each corner in the cube
        for (size_t ii = 0; ii < 8; ++ii)
        {
            windSpeedTemp[ii] = std::sqrt(uValues[ii] * uValues[ii] + vValues[ii] * vValues[ii]);
            windDirTemp[ii] = 180.0 * std::atan2(-uValues[ii], -vValues[ii]) / 3.14159265358979323846;
        }

        // Now perform a tri-linear interpolation inside this cube with wind-speed values to calculate
        //  the inerpolated wind-speed
        auto interpSpeed = TriLinearInterpolation(windSpeedTemp, spatialIndices[0] - lvlFloor, spatialIndices[1] - latFloor, spatialIndices[2] - lonFloor);
        auto interpDirection = TriLinearInterpolation(windDirTemp, spatialIndices[0] - lvlFloor, spatialIndices[1] - latFloor, spatialIndices[2] - lonFloor);

        finalWindSpeeds[timeIdx] = interpSpeed.value;
        finalWindSpeedErrors[timeIdx] = interpSpeed.uncertainty;
        finalWindDirections[timeIdx] = interpDirection.value;
        finalWindDirectionErrors[timeIdx] = interpDirection.uncertainty;
    }

    InterpolatedWind result;
    result.speed = finalWindSpeeds;
    result.speedError = finalWindSpeedErrors;
    result.direction = finalWindDirections;
    result.directionError = finalWindDirectionErrors;
    return result;
}
