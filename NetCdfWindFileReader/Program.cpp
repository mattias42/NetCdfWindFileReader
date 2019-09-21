#include <netcdf.h>
#include <stdio.h>
#include "NetCdfFileReader.h"
#include <sstream>
#include <iostream>

static std::string FormatType(nc_type type)
{
    switch (type)
    {
    case NC_BYTE: return "byte";
    case NC_CHAR: return "char";
    case NC_SHORT: return "short";
    case NC_INT: return "int";
    case NC_FLOAT: return "float";
    case NC_DOUBLE: return "double";
    case NC_STRING: return "string";
    default: return "unknown";
    }
}

bool PrintFileInformation(int ncFileId)
{
    // Inquire the file about groups
    int locationIds[64];
    int nofDimensions = 0;
    int nofVariables = 0;
    int nofAttributes = 0;
    int unlimdimidp = 0;
    int status = nc_inq(ncFileId, &nofDimensions, &nofVariables, &nofAttributes, &unlimdimidp);
    if (status != NC_NOERR)
    {
        printf("Error reading groups: " + status);
        return false;
    }

    int fileFormat = 0;
    status = nc_inq_format(ncFileId, &fileFormat);
    if (status == NC_NOERR)
    {
        switch (fileFormat)
        {
        case NC_FORMAT_CLASSIC: printf(" File format: Classic\n"); break;
        case NC_FORMAT_64BIT_OFFSET: printf(" File format: 64-bit offset\n"); break;
        case NC_FORMAT_CDF5: printf(" File format: Cdf5\n"); break;
        case NC_FORMAT_NETCDF4: printf(" File format: NetCdf4\n"); break;
        case NC_FORMAT_NETCDF4_CLASSIC: printf(" File format: NetCdf4-Classic\n"); break;
        default: printf(" File format: Unknown\n"); break;
        }
    }

    printf("Number of global attributes in file: %d\n", nofAttributes);
    for (int attributeIdx = 0; attributeIdx < nofAttributes; ++attributeIdx)
    {
        char attributeName[512];
        nc_type attributeType;
        size_t numberOfValues = 0;
        status = nc_inq_attname(ncFileId, NC_GLOBAL, attributeIdx, attributeName);
        status = nc_inq_atttype(ncFileId, NC_GLOBAL, attributeName, &attributeType);
        status = nc_inq_attlen(ncFileId, NC_GLOBAL, attributeName, &numberOfValues);

        auto typeName = FormatType(attributeType);

        printf("  %d: '%s' of type %s with %zu values\n", attributeIdx, attributeName, typeName.c_str(), numberOfValues);
    }

    printf("Number of dimensions in file: %d\n", nofDimensions);
    for (int dimensionIdx = 0; dimensionIdx < nofDimensions; ++dimensionIdx)
    {
        char nameBuffer[128];
        size_t length = 0;
        nc_inq_dim(ncFileId, dimensionIdx, nameBuffer, &length);

        printf("  %d: '%s' with %zu values\n", dimensionIdx, nameBuffer, length);
    }

    printf("Number of unlimited dimensions in file: %d\n", unlimdimidp);

    // List the variables
    printf("Number of variables in file: %d\n", nofVariables);
    for (int variableIdx = 0; variableIdx < nofVariables; ++variableIdx)
    {
        char variablename[128];
        nc_type type;
        int nofDimensions = 0;
        int numberOfAttributes = 0;
        nc_inq_var(ncFileId, variableIdx, variablename, &type, &nofDimensions, nullptr, &numberOfAttributes);

        auto typeName = FormatType(type);

        printf("  %d: '%s' of type %s with %d dimensions and %d attributes\n", variableIdx, variablename, typeName.c_str(), nofDimensions, numberOfAttributes);

        // get the attributes
        for (int attributeIdx = 0; attributeIdx < numberOfAttributes; ++attributeIdx)
        {
            char attributeName[128];
            nc_inq_attname(ncFileId, variableIdx, attributeIdx, attributeName);
            printf("        attribute %d: %s\n", attributeIdx, attributeName);
        }

        // get the dimensions
        std::vector<int> dimensions(nofDimensions);
        nc_inq_vardimid(ncFileId, variableIdx, dimensions.data());
        for (int dimensionIdx = 0; dimensionIdx < nofDimensions; ++dimensionIdx)
        {
            char name[128];
            nc_inq_dimname(ncFileId, dimensions[dimensionIdx], name);
            printf("        dimension %d: %s\n", dimensionIdx, name);
        }
    }

    return true;
}

template<class T>
void Add(std::vector<T>& elements, T value)
{
    for (size_t ii = 0; ii < elements.size(); ++ii)
    {
        elements[ii] += value;
    }
}

// Returns the (first) index into the provided vector where the valueToFind is found
//  This assumes that values is a one-dimensional vector
//  But does not assume that values is sorted in any way.
//  @throws std::invalid_argument if the value cannot be found.
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

// Interpolates between the values floor(index) and ceil(index)
//  @throws std::invalid_argument if index < 0 or index >= values.size();
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

int main(void)
{
    auto filename = "D:\\Development\\FromSantiago\\netcdfToText\\villarrica_200501_201701.nc";

    try
    {
        NetCdfFileReader fileReader;
        fileReader.Open(filename);

        /* if (!PrintFileInformation(ncFileId))
        {
            return 1;
        } */

        // get the different variables which we need
        auto longitudeSize = fileReader.GetSizeOfVariable("longitude");
        auto longitude = fileReader.ReadVariableAsFloat("longitude");
        Add<float>(longitude, -360.0F);

        auto latitudeSize = fileReader.GetSizeOfVariable("latitude");
        auto latitude = fileReader.ReadVariableAsFloat("latitude");

        auto levelSize = fileReader.GetSizeOfVariable("level");
        auto level = fileReader.ReadVariableAsFloat("level");

        auto timeSize = fileReader.GetSizeOfVariable("time");
        auto time = fileReader.ReadVariableAsFloat("time");

        auto uSize = fileReader.GetSizeOfVariable("u");
        auto u = fileReader.ReadVariableAsFloat("u");

        auto vSize = fileReader.GetSizeOfVariable("v");
        auto v = fileReader.ReadVariableAsFloat("v");

        double villarica_latitude = -39.42;
        double villarica_longitude = -71.93;
        double villarica_altitude_m = 2847.0;

        // TODO: where to find these
        std::vector<float> levels
        {
            225, 250, 300, 350, 400, 450,
            500, 550, 600, 650, 700, 750,
            775, 800, 825, 850, 875, 900,
            925, 950, 975, 1000
        };

        // TODO: where to find these
        std::vector<float> altitudes_km
        {
            10.42F,9.59F, 8.81F, 7.38F, 6.71F, 5.50F,
            4.94F, 4.42F, 3.48F, 3.06F, 2.67F, 2.31F,
            1.98F, 1.68F, 1.41F, 1.17F, 0.95F, 0.76F,
            0.60F, 0.46F, 0.24F, 0.10F
        };
        double altitudeIdx = GetFractionalIndex(altitudes_km, villarica_altitude_m * 0.001);
        double villarica_level = Interpolate(levels, altitudeIdx);

        double latitudeIdx = GetFractionalIndex(latitude, villarica_latitude);
        double longitudeIdx = GetFractionalIndex(longitude, villarica_longitude);
        double levelIdx = GetFractionalIndex(level, villarica_level);




    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }

    std::cout << "Net cdf file reader done.";
}

