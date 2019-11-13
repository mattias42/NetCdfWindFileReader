#include <netcdf.h>
#include <iomanip>
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

        // First the mandatory variables
        NetCdfTensor longitude = fileReader.ReadVariable("longitude");
        Add<float>(longitude.values, -360.0F);

        NetCdfTensor latitude = fileReader.ReadVariable("latitude");

        NetCdfTensor level = fileReader.ReadVariable("level");

        NetCdfTensor time = fileReader.ReadVariable("time");

        NetCdfTensor u = fileReader.ReadVariable("u");

        NetCdfTensor v = fileReader.ReadVariable("v");

        // TODO: Check that the sizes of these variables agree...

        // Then the optional variables (which are not always defined in the file)
        NetCdfTensor relativeHumidity;
        if (fileReader.ContainsVariable("r"))
        {
            relativeHumidity = fileReader.ReadVariable("r");
        }
        else if (fileReader.ContainsVariable("rh"))
        {
            relativeHumidity = fileReader.ReadVariable("rh");
        }

        NetCdfTensor cloudCoverage;
        if (fileReader.ContainsVariable("cc"))
        {
            cloudCoverage = fileReader.ReadVariable("cc");
        }

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

        const double latitudeIdx = GetFractionalIndex(latitude.values, villarica_latitude);
        const double longitudeIdx = GetFractionalIndex(longitude.values, villarica_longitude);
        const double levelIdx = GetFractionalIndex(altitudes_km, villarica_altitude_m * 0.001);

        auto result = InterpolateWind(
            u.values,
            v.values,
            u.size,
            { levelIdx, latitudeIdx, longitudeIdx });

        // Save all the values for the NovacProgram to read
        std::ofstream windFieldFile{ "D:\\Development\\FromSantiago\\netcdfToText\\MattiasOutput_villarrica_200501_201701.txt" };
        windFieldFile << "date time ws wse wd wde" << std::endl;
        windFieldFile.precision(1);
        windFieldFile << std::fixed << std::setw(4) << std::setfill(' ');
        for (size_t ii = 0; ii < time.values.size(); ++ii)
        {
            // time is hours since 1900-01-01 00:00:0.0
            time_t rawtimeSinceEpoch = (time_t)(time.values[ii] * 3600.0) - 2208988800L;

            // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
            char buf[80];
            struct tm ts;
            gmtime_s(&ts, &rawtimeSinceEpoch);
            strftime(buf, sizeof(buf), "%Y.%m.%d %H:%M", &ts);

            windFieldFile << buf << " ";
            if (result.cloudCoverage.size() > 0)
            {
                windFieldFile << result.cloudCoverage[ii] << " ";
            }
            if (result.relativeHumidity.size() > 0)
            {
                windFieldFile << result.relativeHumidity[ii] << " ";
            }
            windFieldFile << result.speed[ii] << " ";
            windFieldFile << result.speedError[ii] << " ";
            windFieldFile << result.direction[ii] << " ";
            windFieldFile << result.directionError[ii] << std::endl;
        }
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }

    std::cout << "Net cdf file reader done.";
}

