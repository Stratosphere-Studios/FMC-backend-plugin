#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


#define DEG_TO_RAD M_PI / 180.0
#define RAD_TO_DEG 180.0 / M_PI
#define EARTH_RADIUS_NM 3441.0;
#define NM_TO_M 1852;


namespace geo
{
	inline double rad_to_pos_deg(double rad) //Converts radians to degrees. The output is always positive
	{
		double out = rad * RAD_TO_DEG + 360.0;
		while (out > 360.0)
		{
			out -= 360.0;
		}
		return out;
	}

	struct point
	{
		double lat_deg, lon_deg;

		double getGreatCircleBearingDeg(point other)
		{
			double lat1_rad = lat_deg * DEG_TO_RAD;
			double lon1_rad = lon_deg * DEG_TO_RAD;
			double lat2_rad = other.lat_deg * DEG_TO_RAD;
			double lon2_rad = other.lon_deg * DEG_TO_RAD;
			double dlon = lon2_rad - lon1_rad;
			double a = sin(dlon) * cos(lat2_rad);
			double b = cos(lat1_rad) * sin(lat2_rad) - sin(lat1_rad) * cos(lat2_rad) * cos(dlon);
			if (a == 0)
			{
				return -1;
			}
			else
			{
				double theta = atan2(a, b);
				return rad_to_pos_deg(theta);
			}
		}

		double getGreatCircleDistanceNM(point other)
		{
			double lat1_rad = lat_deg * DEG_TO_RAD;
			double lon1_rad = lon_deg * DEG_TO_RAD;
			double lat2_rad = other.lat_deg * DEG_TO_RAD;
			double lon2_rad = other.lon_deg * DEG_TO_RAD;
			double dlon = lon2_rad - lon1_rad;
			double dlat = lat2_rad - lat1_rad;
			double a1 = sin(dlat / 2);
			double a2 = sin(dlon / 2);
			double a = (a1 * a1) + cos(lat1_rad) * cos(lat2_rad) * (a2 * a2);
			double c = 2 * atan2(sqrt(a), sqrt(1 - a));
			return c * EARTH_RADIUS_NM;
		}
	};
}
