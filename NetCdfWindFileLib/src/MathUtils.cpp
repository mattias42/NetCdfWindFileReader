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



