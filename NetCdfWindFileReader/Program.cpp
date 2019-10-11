#include <netcdf.h>
#include <stdio.h>
#include "NetCdfFileReader.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <time.h>
#include <WindFieldInterpolation.h>
#include "MathUtils.h"

int main(void)
{
    auto filename = "D:\\Development\\FromSantiago\\netcdfToText\\villarrica_200501_201701.nc";

    try
    {
        NetCdfFileReader fileReader;
        fileReader.Open(filename);

        // fileReader.PrintFileInformation();
        // return 1;

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

        auto result = InterpolateWind(u, v, uSize, { levelIdx, latitudeIdx, longitudeIdx });

        // Save all the values for the NovacProgram to read
        std::ofstream windFieldFile { "D:\\Development\\FromSantiago\\netcdfToText\\MattiasOutput_villarrica_200501_201701.txt" };
        windFieldFile << "date time ws wd" << std::endl;
        windFieldFile.precision(3);
        windFieldFile << std::fixed;
        for (size_t ii = 0; ii < time.size(); ++ii)
        {
            // time is hours since 1900-01-01 00:00:0.0
            time_t rawtimeSinceEpoch = (time_t)(time[ii] * 3600.0) - 2208988800L;

            // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
            char buf[80];
            struct tm ts;
            localtime_s(&ts, &rawtimeSinceEpoch);
            strftime(buf, sizeof(buf), "%Y.%m.%d %H:%M:%S", &ts);

            windFieldFile << buf << " ";
            windFieldFile << result.speed[ii] << " ";
            windFieldFile << result.direction[ii] << std::endl;
        }
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }

    std::cout << "Net cdf file reader done.";
}

