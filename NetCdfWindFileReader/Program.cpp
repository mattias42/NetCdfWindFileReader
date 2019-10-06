#include <netcdf.h>
#include <stdio.h>
#include "NetCdfFileReader.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <WindFieldInterpolation.h>

    template<class T>
void Add(std::vector<T>& elements, T value)
{
    for (size_t ii = 0; ii < elements.size(); ++ii)
    {
        elements[ii] += value;
    }
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

        // These values are to be taken from the list of volcanoes...
        double villarica_latitude = -39.42;
        double villarica_longitude = -71.93;
        double villarica_altitude_m = 2847.0;

        // These are fixed and can be written into the program...
        const std::vector<float> levels
        {
            225, 250, 300, 350, 400, 450,
            500, 550, 600, 650, 700, 750,
            775, 800, 825, 850, 875, 900,
            925, 950, 975, 1000
        };
        const std::vector<float> altitudes_km
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

        // Dimensions are [time, level, lat, lon]
        // extract the wind-speed at all times for a fixed level, lat, lon.
        std::vector<double> indices = { 0.0, levelIdx, latitudeIdx, longitudeIdx };
        for (size_t timeIdx = 0; timeIdx < time.size(); ++timeIdx)
        {
            indices[0] = (double)timeIdx;

            // ----------- Pick out the neighoring u- and v- values at this point in time -----------
            // -----------      this is a small cube with 2x2x2 values     -----------
            std::vector<double> uValues(8);
            std::vector<double> vValues(8);

            size_t lonFloor = (size_t)std::floor(longitudeIdx);
            size_t latFloor = (size_t)std::floor(latitudeIdx);
            size_t lvlFloor = (size_t)std::floor(levelIdx);

            for (size_t lvlIdx = lvlFloor; lvlIdx <= lvlFloor + 1; ++lvlIdx)
            {
                for (size_t latIdx = latFloor; latIdx <= latFloor + 1; ++latIdx)
                {
                    for (size_t lonIdx = lonFloor; lonIdx <= lonFloor + 1; ++lonIdx)
                    {
                        size_t index = ((timeIdx * uSize[1] + lvlIdx) * uSize[2] + latIdx) * uSize[3] + lonIdx;

                        size_t minorIndex = ((lvlIdx - lvlFloor) * 2 + (latIdx - latFloor)) * 2 + (lonIdx - lonFloor);

                        uValues[minorIndex] = u[index];
                        vValues[minorIndex] = v[index];
                    }
                }
            }

            // Calculate the wind-speed and wind-direction at each corner in the cube
            std::vector<double> windSpeedTemp(8);
            std::vector<double> windDirTemp(8);
            for (size_t ii = 0; ii < 8; ++ii)
            {
                windSpeedTemp[ii] = std::sqrt(u[ii] * u[ii] + v[ii] * v[ii]);
                windDirTemp[ii] = 180.0 * std::atan2(-u[ii], -v[ii]) / 3.14159265358979323846;
            }

            // Now perform a tri-linear interpolation inside this cube with wind-speed values to calculate
            //  the inerpolated wind-speed
            double ws = TriLinearInterpolation(windSpeedTemp, levelIdx - lvlFloor, latitudeIdx - latFloor, longitudeIdx - lonFloor);
            double wd = TriLinearInterpolation(windDirTemp, levelIdx - lvlFloor, latitudeIdx - latFloor, longitudeIdx - lonFloor);

        }
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }

    std::cout << "Net cdf file reader done.";
}

