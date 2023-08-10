/*
	Author: discord/bruh4096#4512
*/

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


#define DEG_TO_RAD M_PI / 180.0
#define RAD_TO_DEG 180.0 / M_PI
#define EARTH_RADIUS_NM 3441.0;
#define NM_TO_M 1852;
#define FT_TO_NM 1 / 6076.12;
#define M_TO_FT 3.28084


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

		double get_great_circle_bearing_deg(point other)
		{
			double lat1_rad = lat_deg * DEG_TO_RAD;
			double lon1_rad = lon_deg * DEG_TO_RAD;
			double lat2_rad = other.lat_deg * DEG_TO_RAD;
			double lon2_rad = other.lon_deg * DEG_TO_RAD;
			double dlon = lon2_rad - lon1_rad;
			double a = sin(dlon) * cos(lat2_rad);
			double b = cos(lat1_rad) * sin(lat2_rad) - sin(lat1_rad) * cos(lat2_rad) * cos(dlon);
			if (b == 0)
			{
				return 0;
			}
			else
			{
				double theta = atan2(a, b);
				return rad_to_pos_deg(theta);
			}
		}

		double get_great_circle_distance_nm(point other)
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

	/*
		Calculates lat,long of a point given its bearing and distance from a reference point.
	*/

	inline point get_pos_from_brng_dist(point ref, double brng_deg, double dist_nm)
	{
		double ref_lat_rad = ref.lat_deg * DEG_TO_RAD;
		double ref_lon_rad = ref.lon_deg * DEG_TO_RAD;
		double brng_rad = brng_deg * DEG_TO_RAD;
		double ang_dist_rad = dist_nm / EARTH_RADIUS_NM;
		point ret{};
		double tmp_lat = asin(sin(ref_lat_rad) * cos(ang_dist_rad) + cos(ref_lat_rad) * sin(ang_dist_rad) * cos(brng_rad));
		double tmp_lon = atan2(sin(brng_rad) * sin(ang_dist_rad) * cos(ref_lat_rad), 
							   cos(ang_dist_rad)-sin(ref_lat_rad) * sin(tmp_lat));
		ret.lat_deg = tmp_lat * RAD_TO_DEG;
		ret.lon_deg = (ref_lon_rad + tmp_lon) * RAD_TO_DEG;

		return ret;
	}

	struct point3d
	{
		point p;
		double alt_ft;

		double get_true_dist_nm(point3d other)
		{
			double lat_dist_nm = p.get_great_circle_distance_nm(other.p);
			double v_dist_nm = (alt_ft - other.alt_ft) * FT_TO_NM;

			return sqrt(lat_dist_nm * lat_dist_nm + v_dist_nm * v_dist_nm);
		}
	};
}
