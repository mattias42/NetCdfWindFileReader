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

static std::vector<int> GetDimensionIndicesOfVariable(int ncFileId, int variableIdx)
{
    int nofDimensions = 0;
    int status = nc_inq_varndims(ncFileId, variableIdx, &nofDimensions);

    std::vector<int> dimensions(nofDimensions);
    status = nc_inq_vardimid(ncFileId, variableIdx, dimensions.data());

    return dimensions;
}

static std::vector<size_t> GetSizeOfVariable(int ncFileIdx, int variableIdx)
{
    auto dimensionIndices = GetDimensionIndicesOfVariable(ncFileIdx, variableIdx);
    std::vector<size_t> sizes(dimensionIndices.size());

    for (size_t ii = 0; ii < dimensionIndices.size(); ++ii)
    {
        size_t value = 0;
        int status = nc_inq_dimlen(ncFileIdx, dimensionIndices[ii], &value);
        sizes[ii] = value;
    }

    return sizes;
}

static std::vector<float> Read1dFloatVariable(int ncFileId, int variableIdx)
{
    std::vector<size_t> variableSize = GetSizeOfVariable(ncFileId, variableIdx);
    if (variableSize.size() != 1)
    {
        std::stringstream message;
        message << "Function 'Read1dFloatVariable' can only be used to read a one-dimensional variable, but variable " << variableIdx << " contains " << variableSize.size() << " dimensions.";
        throw std::invalid_argument(message.str());
    }

    std::vector<float> values(variableSize[0]);

    int status = nc_get_var_float(ncFileId, variableIdx, values.data());

    return values;
}

static size_t GetNumberOfElements(std::vector<size_t> sizes)
{
    size_t product = 1;

    for (size_t dim : sizes)
    {
        product *= dim;
    }

    return product;
}

static std::vector<float> ReadNdFloatVariable(int ncFileId, int variableIdx)
{
    std::vector<size_t> variableSize = GetSizeOfVariable(ncFileId, variableIdx);

    size_t size = GetNumberOfElements(variableSize);

    std::vector<float> values(size);

    int status = nc_get_var_float(ncFileId, variableIdx, values.data());

    return values;
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
        auto longitude = fileReader.ReadVariableAsFloat("longitude");
        auto latitude = fileReader.ReadVariableAsFloat("latitude");
        auto level = fileReader.ReadVariableAsFloat("level");
        auto time = fileReader.ReadVariableAsFloat("time");
        auto u = fileReader.ReadVariableAsFloat("u");
        auto v = fileReader.ReadVariableAsFloat("v");

    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }

    std::cout << "Net cdf file reader done.";
}

