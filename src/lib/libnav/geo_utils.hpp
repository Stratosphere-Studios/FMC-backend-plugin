/*
	Author: discord/bruh4096#4512
*/

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


#define DEG_TO_RAD M_PI / 180.0
#define RAD_TO_DEG 180.0 / M_PI
#define NM_TO_M 1852;
#define FT_TO_NM 1 / 6076.12;
#define M_TO_FT 3.28084

constexpr double EARTH_RADIUS_NM = 3441.0;


namespace geo
{
	/*
		Common measurement units by postfixes:
		_deg: degrees
		_nm: nautical miles
		_ft: feet
	*/


	/*
		Function: rad_to_pos_deg
		Description:
		Function that converts a value in radians to degrees.
		Param:
		rad: a value in radians
		Return:
		Returns a non-negative value in degrees. 
	*/

	inline double rad_to_pos_deg(double rad)
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

		/*
			Function: get_great_circle_bearing_deg
			Description:
			Function that calculates great circle bearing between 2 points on Earth's surface.
			Param:
			other: second point
			Return:
			Returns a great circle bearing(non-negative value)
		*/

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

		/*
			Function: get_ang_dist_deg
			Description:
			Function that calculates angular distance between to points on Earth's surface.
			Param:
			other: second point
			Return:
			Returns an angular distance in radians.
		*/

		double get_ang_dist_rad(point other)
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
			return 2 * atan2(sqrt(a), sqrt(1 - a));
		}

		/*
			Function: get_great_circle_distance_nm
			Description:
			Function that calculates great circle distance between 2 points on Earth's surface.
			Param:
			other: second point
			Return:
			Returns a great circle distance(non-negative value)
		*/

		double get_great_circle_distance_nm(point other)
		{
			return get_ang_dist_rad(other) * EARTH_RADIUS_NM;
		}

		/*
			Function: get_line_dist_nm
			Description:
			Function that calculates the length of a straight line segment that connects 2 points
			Param:
			other: second point.
			alt1_ft: altitude of this point AMSL
			alt2_ft: altitude of another point AMSL
			Return:
			Returns a non-negative distance value.
		*/

		double get_line_dist_nm(point other, double alt1_ft, double alt2_ft)
		{
			double elev1_nm = alt1_ft * FT_TO_NM;
			double elev2_nm = alt2_ft * FT_TO_NM;
			double ang_dist_rad = get_ang_dist_rad(other);
			double a = EARTH_RADIUS_NM + elev1_nm;
			double b = EARTH_RADIUS_NM + elev2_nm;
			return sqrt(std::pow(a, 2) + std::pow(b, 2) - 2 * a * b * cos(ang_dist_rad));
		}
	};

	/*
		Function: get_pos_from_brng_dist
		Description:
		Function that calculates lat,long of a point given its bearing and distance from a reference point.
		Param:
		ref: point to which the bearing and distance are given.
		brng_deg: bearing to ref
		dist_nm: distance to ref
		Return:
		Returns an estimated position.
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
			cos(ang_dist_rad) - sin(ref_lat_rad) * sin(tmp_lat));
		ret.lat_deg = tmp_lat * RAD_TO_DEG;
		ret.lon_deg = (ref_lon_rad + tmp_lon) * RAD_TO_DEG;

		return ret;
	}

	struct point3d
	{
		point p;
		double alt_ft;

		/*
			Function: get_true_dist_nm
			Description:
			Function that calculates the length of a straight line segment that connects 2 points
			Param:
			other: second point.
			Return:
			Returns a non-negative distance value.
		*/

		double get_true_dist_nm(point3d other)
		{
			return p.get_line_dist_nm(other.p, alt_ft, other.alt_ft);
		}
	};

	/*
		Function: get_dme_dme_pos
		Description:
		Function that returns estimates of aircraft using distances from 2 points(presumably DMEs), their positions
		and altitude of the aircraft. . This function uses an algorithm described here:
		https://aviation.stackexchange.com/questions/46135/how-can-i-triangulate-a-position-using-two-dmes
		Param:
		dme_u: coordinates of westmost dme
		dme_s: coordinates of eastmost dme
		d_u_nm: distance from dme1 to the aircraft
		d_s_nm: distance from dme2 to the aircraft
		elev_u_ft: elevation of dme_u AMSL
		elev_s_ft: elevation of dme_s AMSL
		ac_alt_ft: barometric altitude of the aircraft
		arr: pointer to array where the calculated estimates will be written. The array's length MUST be equal to 2,
		since there will at most be 2 estimates of the position.
		Return:
		returns number of position estimates written to arr.
	*/

	inline int get_dme_dme_pos(point dme_u, point dme_s, double d_u_nm, double d_s_nm, double elev_u_ft, 
							   double elev_s_ft, double ac_alt_ft, point* arr)
	{
		double lat_u_rad = dme_u.lat_deg * DEG_TO_RAD;
		double lon_u_rad = dme_u.lon_deg * DEG_TO_RAD;
		double lat_s_rad = dme_s.lat_deg * DEG_TO_RAD;
		double lon_s_rad = dme_s.lon_deg * DEG_TO_RAD;
		double lat_diff = lat_s_rad - lat_u_rad;
		double lon_diff = lon_s_rad - lon_u_rad;
		double a = cos(lat_s_rad) * sin(lat_u_rad);
		double b = sin(lat_s_rad) * cos(lat_u_rad);

		double elev_1_nm = elev_u_ft * FT_TO_NM;
		double elev_2_nm = elev_s_ft * FT_TO_NM;
		double ac_alt_nm = ac_alt_ft * FT_TO_NM;

		// Step 0: Convert slant-ranges to angular distance
		double a_u = (d_u_nm - ac_alt_nm + elev_1_nm) * (d_u_nm + ac_alt_nm - elev_1_nm);
		double a_s = (d_s_nm - ac_alt_nm + elev_1_nm) * (d_s_nm + ac_alt_nm - elev_1_nm);
		double b_u = (EARTH_RADIUS_NM + elev_1_nm) * (EARTH_RADIUS_NM + ac_alt_nm);
		double b_s = (EARTH_RADIUS_NM + elev_2_nm) * (EARTH_RADIUS_NM + ac_alt_nm);
		double theta_ua = 2 * asin(0.5 * (sqrt(a_u / b_u)));
		double theta_sa = 2 * asin(0.5 * (sqrt(a_s / b_s)));
		// Step 1: Solve the spherical triangle for each station
		double sin_lat = std::pow(sin(0.5 * lat_diff), 2);
		double sin_lon = std::pow(sin(0.5 * lon_diff), 2);
		double theta_us = 2 * asin(sqrt(sin_lat + a * sin_lon));
		double psi_su = atan2((cos(lat_s_rad) * sin(lon_diff)), (b - a * cos(lon_diff)));
		// Step 2: Confirm inputs are consistent and a solution exists
		if (theta_ua + theta_sa >= theta_us && abs(theta_ua - theta_sa) <= theta_us)
		{
			// Step 3: Solve the spherical triangle USA
			double beta_u = acos((cos(theta_sa) - cos(theta_us) * cos(theta_ua)) / (sin(theta_us) * sin(theta_ua)));
			// Step 4: With all data now available, compute aircraft latitude and longitude
			double psi_rad[2] = { psi_su + beta_u, psi_su - beta_u };
			for (int i = 0; i < 2; i++)
			{
				double tmp_1 = sin(theta_ua) * cos(psi_rad[i]);
				arr[i].lat_deg = asin(sin(lat_u_rad) * cos(theta_ua) + cos(lat_u_rad) * tmp_1) * RAD_TO_DEG;
				arr[i].lon_deg = (atan2(sin(psi_rad[i]) * sin(theta_ua), cos(lat_u_rad) * cos(theta_ua) - 
					sin(lat_u_rad) * tmp_1) + lon_u_rad) * RAD_TO_DEG;
			}
			return 2;
		}
		return 0;
	}
}
