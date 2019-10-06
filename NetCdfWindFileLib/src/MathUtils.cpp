#include <MathUtils.h>

double GetFractionalIndex(const std::vector<float>& values, float valueToFind)
{
    for (size_t ii = 1; ii < values.size(); ++ii)
    {
        if (values[ii - 1] <= valueToFind && values[ii] >= valueToFind)
        {
            return (ii - 1) + (valueToFind - values[ii - 1]) / (values[ii] - values[ii - 1]);
        }
        else if (values[ii - 1] >= valueToFind && values[ii] <= valueToFind)
        {
            return (ii - 1) + 1 - (valueToFind - values[ii]) / (values[ii - 1] - values[ii]);
        }
    }

    throw std::invalid_argument("Cannot find the value in the provided vector.");
}

double Interpolate(const std::vector<float>& values, double index)
{
    int ii = (int)std::floor(index);
    double alpha = index - (double)ii;

    if (ii < 0.0 || ii >= values.size() - 2)
    {
        throw std::invalid_argument("Invalid index for interpolation.");
    }

    return values[ii] * (1.0 - alpha) + values[ii + 1] * alpha;
}

double TriLinearInterpolation(const std::vector<double>& inputCube, double idxX, double idxY, double idxZ)
{
    double c00 = inputCube[0] * (1.0 - idxX) + inputCube[4] * idxX;
    double c01 = inputCube[1] * (1.0 - idxX) + inputCube[5] * idxX;
    double c10 = inputCube[2] * (1.0 - idxX) + inputCube[6] * idxX;
    double c11 = inputCube[3] * (1.0 - idxX) + inputCube[7] * idxX;

    double c0 = c00 * (1.0 - idxY) + c10 * idxY;
    double c1 = c01 * (1.0 - idxY) + c11 * idxY;

    return c0 * (1.0 - idxZ) + c1 * idxZ;
}

