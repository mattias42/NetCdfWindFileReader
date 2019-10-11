#include "NetCdfFileReader.h"
#include <MathUtils.h>
#include <netcdf.h>
#include <sstream>

NetCdfFileReader::NetCdfFileReader()
{
    m_netCdfFileHandle = 0;
}

NetCdfFileReader::~NetCdfFileReader()
{
    Close();
}

void NetCdfFileReader::Open(const std::string& filename)
{
    if (m_netCdfFileHandle != 0)
    {
        Close();
    }

    int status = nc_open(filename.c_str(), NC_NOWRITE, &m_netCdfFileHandle);

    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to open net-cdf file with path: '" << filename << "' Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }
}

void NetCdfFileReader::Close()
{
    if (m_netCdfFileHandle != 0)
    {
        nc_close(m_netCdfFileHandle);
        m_netCdfFileHandle = 0;
    }
}

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

void NetCdfFileReader::PrintFileInformation()
{
    // Inquire the file about groups
    int nofDimensions = 0;
    int nofVariables = 0;
    int nofAttributes = 0;
    int unlimdimidp = 0;
    int status = nc_inq(this->m_netCdfFileHandle, &nofDimensions, &nofVariables, &nofAttributes, &unlimdimidp);
    if (status != NC_NOERR)
    {
        printf("Error reading groups: " + status);
    }

    int fileFormat = 0;
    status = nc_inq_format(this->m_netCdfFileHandle, &fileFormat);
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
        status = nc_inq_attname(this->m_netCdfFileHandle, NC_GLOBAL, attributeIdx, attributeName);
        status = nc_inq_atttype(this->m_netCdfFileHandle, NC_GLOBAL, attributeName, &attributeType);
        status = nc_inq_attlen(this->m_netCdfFileHandle, NC_GLOBAL, attributeName, &numberOfValues);

        auto typeName = FormatType(attributeType);

        printf("  %d: '%s' of type %s with %zu values\n", attributeIdx, attributeName, typeName.c_str(), numberOfValues);
    }

    printf("Number of dimensions in file: %d\n", nofDimensions);
    for (int dimensionIdx = 0; dimensionIdx < nofDimensions; ++dimensionIdx)
    {
        char nameBuffer[128];
        size_t length = 0;
        nc_inq_dim(this->m_netCdfFileHandle, dimensionIdx, nameBuffer, &length);

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
        nc_inq_var(this->m_netCdfFileHandle, variableIdx, variablename, &type, &nofDimensions, nullptr, &numberOfAttributes);

        auto typeName = FormatType(type);

        printf("  %d: '%s' of type %s with %d dimensions and %d attributes\n", variableIdx, variablename, typeName.c_str(), nofDimensions, numberOfAttributes);

        // get the attributes
        for (int attributeIdx = 0; attributeIdx < numberOfAttributes; ++attributeIdx)
        {
            char attributeName[128];
            nc_inq_attname(this->m_netCdfFileHandle, variableIdx, attributeIdx, attributeName);
            printf("        attribute %d: %s\n", attributeIdx, attributeName);
        }

        // get the dimensions
        std::vector<int> dimensions(nofDimensions);
        nc_inq_vardimid(this->m_netCdfFileHandle, variableIdx, dimensions.data());
        for (int dimensionIdx = 0; dimensionIdx < nofDimensions; ++dimensionIdx)
        {
            char name[128];
            nc_inq_dimname(this->m_netCdfFileHandle, dimensions[dimensionIdx], name);
            printf("        dimension %d: %s\n", dimensionIdx, name);
        }
    }
}

std::vector<int> NetCdfFileReader::GetDimensionIndicesOfVariable(int variableIdx)
{
    int nofDimensions = 0;
    int status = nc_inq_varndims(m_netCdfFileHandle, variableIdx, &nofDimensions);
    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the number of dimensions of variable '" << variableIdx << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    std::vector<int> dimensions(nofDimensions);
    status = nc_inq_vardimid(m_netCdfFileHandle, variableIdx, dimensions.data());
    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the dimension indices of variable '" << variableIdx << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    return dimensions;
}

int NetCdfFileReader::GetIndexOfVariable(const std::string& variableName)
{
    int index = 0;
    int status = nc_inq_varid(m_netCdfFileHandle, variableName.c_str(), &index);

    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the index of variable '" << variableName << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    return index;
}

std::vector<size_t> NetCdfFileReader::GetSizeOfVariable(int variableIdx)
{
    auto dimensionIndices = GetDimensionIndicesOfVariable(variableIdx);

    std::vector<size_t> sizes(dimensionIndices.size());

    for (size_t ii = 0; ii < dimensionIndices.size(); ++ii)
    {
        size_t value = 0;
        int status = nc_inq_dimlen(m_netCdfFileHandle, dimensionIndices[ii], &value);

        if (status != NC_NOERR)
        {
            std::stringstream msg;
            msg << "Failed to retrieve the dimensions of variable '" << variableIdx << "'. Error code returned was: " << status;
            throw NetCdfException(msg.str().c_str(), status);
        }

        sizes[ii] = value;
    }

    return sizes;
}

std::vector<size_t> NetCdfFileReader::GetSizeOfVariable(const std::string& variableName)
{
    int index = GetIndexOfVariable(variableName);

    return GetSizeOfVariable(index);
}

std::vector<float> NetCdfFileReader::ReadVariableAsFloat(int variableIdx)
{
    std::vector<size_t> variableSize = GetSizeOfVariable(variableIdx);

    size_t totalNumberOfElements = ProductOfElements(variableSize);

    std::vector<float> values(totalNumberOfElements);

    int status = nc_get_var_float(m_netCdfFileHandle, variableIdx, values.data());
    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the values of variable '" << variableIdx << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    return values;
}

std::vector<float> NetCdfFileReader::ReadVariableAsFloat(int variableIdx, const LinearScaling& scaling)
{
    std::vector<float> values = ReadVariableAsFloat(variableIdx);

    for (size_t ii = 0; ii < values.size(); ++ii)
    {
        values[ii] = values[ii] * scaling.scaleFactor + scaling.offset;
    }

    return values;
}

std::vector<float> NetCdfFileReader::ReadVariableAsFloat(const std::string& variableName)
{
    int index = GetIndexOfVariable(variableName);

    LinearScaling variableScaling;
    if (GetLinearScalingForVariable(index, variableScaling))
    {
        return ReadVariableAsFloat(index, variableScaling);
    }
    else
    {
        return ReadVariableAsFloat(index);
    }
}

std::vector<float> NetCdfFileReader::ReadVariableAsFloat(const std::string& variableName, const LinearScaling& scaling)
{
    int index = GetIndexOfVariable(variableName);

    return ReadVariableAsFloat(index, scaling);
}

int NetCdfFileReader::GetNumberOfAttributesForVariable(int variableIdx)
{
    int numberOfAttributes = 0;
    int error = nc_inq_natts(this->m_netCdfFileHandle, &numberOfAttributes);

    if (error == NC_NOERR)
    {
        return numberOfAttributes;
    }
    return -1;
}

bool NetCdfFileReader::GetLinearScalingForVariable(int variableIdx, NetCdfFileReader::LinearScaling& scaling)
{
    int numberOfAttributes = GetNumberOfAttributesForVariable(variableIdx);
    if (numberOfAttributes < 0)
    {
        return false;
    }

    bool scalingFoundInFile = false;
    double scaleFactorValue = 1.0;
    if (NC_NOERR == nc_get_att_double(this->m_netCdfFileHandle, variableIdx, "scale_factor", &scaleFactorValue))
    {
        scaling.scaleFactor = scaleFactorValue;
        scalingFoundInFile = true;
    }

    double additionalOffsetValue = 1.0;
    if (NC_NOERR == nc_get_att_double(this->m_netCdfFileHandle, variableIdx, "add_offset", &additionalOffsetValue))
    {
        scaling.offset = additionalOffsetValue;
        scalingFoundInFile = true;
    }

    return scalingFoundInFile;
}
