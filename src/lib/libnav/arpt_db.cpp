/*
	Author: discord/bruh4096#4512
*/

#include "arpt_db.hpp"


namespace libnav
{
	// Public member functions

	ArptDB::ArptDB(std::unordered_map<std::string, airport_data>* a_db, std::unordered_map<std::string, std::unordered_map<std::string, runway_entry>>* r_db,
		std::string sim_arpt_path, std::string custom_arpt_path, std::string custom_rnw_path)
	{
		arpt_db = a_db;
		rnw_db = r_db;
		sim_arpt_db_path = sim_arpt_path;
		custom_arpt_db_path = custom_arpt_path;
		custom_rnw_db_path = custom_rnw_path;

		if (!does_db_exist(custom_arpt_db_path, custom_arpt_db_sign) || !does_db_exist(custom_rnw_db_path, custom_rnw_db_sign))
		{
			write_arpt_db.store(true, std::memory_order_seq_cst);
			sim_db_loaded = std::async(std::launch::async, [](ArptDB* ptr) -> int { return ptr->load_from_sim_db(); }, this);
			if (!does_db_exist(custom_arpt_db_path, custom_arpt_db_sign))
			{
				apt_db_created = true;
				arpt_db_task = std::async(std::launch::async, [](ArptDB* ptr) {ptr->write_to_arpt_db(); }, this);
			}
			if (!does_db_exist(custom_rnw_db_path, custom_rnw_db_sign))
			{
				rnw_db_created = true;
				rnw_db_task = std::async(std::launch::async, [](ArptDB* ptr) {ptr->write_to_rnw_db(); }, this);
			}
		}
		else
		{
			arpt_db_task = std::async(std::launch::async, [](ArptDB* ptr) {ptr->load_from_custom_arpt(); }, this);
			rnw_db_task = std::async(std::launch::async, [](ArptDB* ptr) {ptr->load_from_custom_rnw(); }, this);
		}
	}

	bool ArptDB::is_loaded()
	{
		// Wait until all of the threads finish
		arpt_db_task.get();
		rnw_db_task.get();
		if (apt_db_created || rnw_db_created)
		{
			return bool(sim_db_loaded.get());
		}
		return true;
	}

	// These functions need to be public because they're used in 
	// other threads when ArptDB object is constructed.

	int ArptDB::load_from_sim_db()
	{
		std::ifstream file(sim_arpt_db_path, std::ifstream::in);
		if (file.is_open())
		{
			std::string line;
			int i = 0;
			int limit = N_ARPT_LINES_IGNORE;
			airport tmp_arpt = { "", {{0, 0}, 0, 0} };
			rnw_data tmp_rnw = { "", {} };
			double max_rnw_length_m = 0;

			while (getline(file, line))
			{
				if (i >= limit && line != "")
				{
					int row_code;
					std::string junk;
					std::stringstream s(line);
					s >> row_code;

					if (tmp_arpt.icao != "" && tmp_rnw.icao != "" && (row_code == LAND_ARPT || row_code == DB_EOF))
					{
						// Offload airport data
						double threshold = MIN_RWY_LENGTH_M;

						if (max_rnw_length_m >= threshold && tmp_arpt.data.transition_alt_ft + tmp_arpt.data.transition_level > 0)
						{
							std::unordered_map<std::string, runway_entry> apt_runways;
							size_t n_runways = tmp_rnw.runways.size();

							for (int i = 0; i < n_runways; i++)
							{
								runway rnw = tmp_rnw.runways.at(i);
								std::pair<std::string, runway_entry> tmp = std::make_pair(rnw.id, rnw.data);
								apt_runways.insert(tmp);
								tmp_arpt.data.pos.lat_deg += rnw.data.start.lat_deg;
								tmp_arpt.data.pos.lon_deg += rnw.data.start.lon_deg;
							}

							tmp_arpt.data.pos.lat_deg /= n_runways;
							tmp_arpt.data.pos.lon_deg /= n_runways;

							// Update queues

							add_to_arpt_queue(tmp_arpt);
							add_to_rnw_queue(tmp_rnw);

							// Update internal data

							std::pair<std::string, airport_data> apt = std::make_pair(tmp_arpt.icao, tmp_arpt.data);
							std::pair<std::string, std::unordered_map<std::string, runway_entry>> rnw_pair = std::make_pair(tmp_arpt.icao, apt_runways);
							arpt_db->insert(apt);
							rnw_db->insert(rnw_pair);
						}

						tmp_arpt.icao = "";
						tmp_rnw.icao = "";
						tmp_arpt.data.pos = { 0, 0 };
						tmp_arpt.data.transition_alt_ft = 0;
						tmp_arpt.data.transition_level = 0;
						tmp_rnw.runways.clear();
						max_rnw_length_m = 0;
					}

					// Parse data

					if (row_code == LAND_ARPT)
					{
						s >> tmp_arpt.data.elevation_ft;
					}
					else if (row_code == MISC_DATA)
					{
						std::string var_name;
						s >> var_name;
						if (var_name == "icao_code")
						{
							std::string icao_code;
							s >> icao_code;
							tmp_arpt.icao = icao_code;
							tmp_rnw.icao = icao_code;
						}
						else if (var_name == "transition_alt")
						{
							s >> tmp_arpt.data.transition_alt_ft;
						}
						else if (var_name == "transition_level")
						{
							s >> tmp_arpt.data.transition_level;
						}
					}
					else if (row_code == LAND_RUNWAY && tmp_arpt.icao != "")
					{
						double tmp = parse_runway(line, &tmp_rnw.runways);
						if (tmp > max_rnw_length_m)
						{
							max_rnw_length_m = tmp;
						}
					}
					else if (row_code == DB_EOF)
					{
						break;
					}
				}
				i++;
			}
			file.close();
			write_arpt_db.store(false, std::memory_order_seq_cst);
			return 1;
		}
		file.close();
		return 0;
	}

	void ArptDB::write_to_arpt_db()
	{
		std::ofstream out(custom_arpt_db_path, std::ofstream::out);
		out << "ARPTDB\n";
		while (arpt_queue.size() || write_arpt_db.load(std::memory_order_seq_cst))
		{
			if (arpt_queue.size())
			{
				std::lock_guard<std::mutex> lock(arpt_queue_mutex);
				uint8_t precision = N_DOUBLE_OUT_PRECISION;
				airport data = arpt_queue[0];
				arpt_queue.erase(arpt_queue.begin());

				std::string arpt_lat = strutils::double_to_str(data.data.pos.lat_deg, precision);
				std::string arpt_lon = strutils::double_to_str(data.data.pos.lon_deg, precision);
				std::string arpt_icao_pos = data.icao + " " + arpt_lat + " " + arpt_lon;

				out << arpt_icao_pos << " " << data.data.elevation_ft << " " << data.data.transition_alt_ft << " " << data.data.transition_level << "\n";
			}
		}
		out.close();
	}

	void ArptDB::write_to_rnw_db()
	{
		std::ofstream out(custom_rnw_db_path, std::ofstream::out);
		out << "RNWDB\n";
		while (rnw_queue.size() || write_arpt_db.load(std::memory_order_seq_cst))
		{
			if (rnw_queue.size())
			{
				std::lock_guard<std::mutex> lock(rnw_queue_mutex);
				uint8_t precision = N_DOUBLE_OUT_PRECISION;
				rnw_data data = rnw_queue[0];
				rnw_queue.erase(rnw_queue.begin());
				for (int i = 0; i < data.runways.size(); i++)
				{
					std::string rnw_start_lat = strutils::double_to_str(data.runways[i].data.start.lat_deg, precision);
					std::string rnw_start_lon = strutils::double_to_str(data.runways[i].data.start.lon_deg, precision);
					std::string rnw_end_lat = strutils::double_to_str(data.runways[i].data.end.lat_deg, precision);
					std::string rnw_end_lon = strutils::double_to_str(data.runways[i].data.end.lon_deg, precision);

					std::string rnw_start = rnw_start_lat + " " + rnw_start_lon;
					std::string rnw_end = rnw_end_lat + " " + rnw_end_lon;

					std::string rnw_icao_pos = data.icao + " " + data.runways[i].id + " " + rnw_start + " " + rnw_end;

					out << rnw_icao_pos << " " << data.runways[i].data.displ_threshold_m << "\n";
				}
			}
		}
		out.close();
	}

	void ArptDB::load_from_custom_arpt()
	{
		std::ifstream file(custom_arpt_db_path, std::ifstream::in);
		if (file.is_open())
		{
			std::string line;
			while (getline(file, line))
			{
				if (line != custom_arpt_db_sign)
				{
					std::string icao;
					airport_data tmp;
					std::stringstream s(line);
					s >> icao >> tmp.pos.lat_deg >> tmp.pos.lon_deg >> tmp.elevation_ft >> tmp.transition_alt_ft >> tmp.transition_level;
					std::pair<std::string, airport_data> tmp_pair = std::make_pair(icao, tmp);
					arpt_db->insert(tmp_pair);
				}
			}
			file.close();
		}
		file.close();
	}

	void ArptDB::load_from_custom_rnw()
	{
		std::ifstream file(custom_rnw_db_path, std::ifstream::in);
		if (file.is_open())
		{
			std::string line;
			std::string curr_icao = "";
			std::unordered_map<std::string, runway_entry> runways = {};
			while (getline(file, line))
			{
				if (line != custom_rnw_db_sign)
				{
					std::string icao, rnw_id;
					runway_entry tmp;
					std::stringstream s(line);
					s >> icao;
					if (icao != curr_icao)
					{
						if (curr_icao != "")
						{
							std::pair<std::string, std::unordered_map<std::string, runway_entry>> icao_runways = std::make_pair(curr_icao, runways);
							rnw_db->insert(icao_runways);
						}
						curr_icao = icao;
						runways.clear();
					}
					s >> rnw_id >> tmp.start.lat_deg >> tmp.start.lon_deg >> tmp.end.lat_deg >> tmp.end.lon_deg >> tmp.displ_threshold_m;
					std::pair<std::string, runway_entry> str_rnw_entry = std::make_pair(rnw_id, tmp);
					runways.insert(str_rnw_entry);
				}
			}
			file.close();
		}
		file.close();
	}

	// Normal user interface functions:

	bool ArptDB::is_airport(std::string icao_code)
	{
		std::lock_guard<std::mutex> lock(arpt_db_mutex);
		return arpt_db->find(icao_code) != arpt_db->end();
	}

	int ArptDB::get_airport_data(std::string icao_code, airport_data* out)
	{
		if (is_airport(icao_code))
		{
			std::lock_guard<std::mutex> lock(arpt_db_mutex);
			airport_data tmp = arpt_db->at(icao_code);
			out->pos.lat_deg = tmp.pos.lat_deg;
			out->pos.lon_deg = tmp.pos.lon_deg;
			out->elevation_ft = tmp.elevation_ft;
			out->transition_alt_ft = tmp.transition_alt_ft;
			out->transition_level = tmp.transition_level;
			return 1;
		}
		return 0;
	}

	int ArptDB::get_apt_rwys(std::string icao_code, runway_data* out)
	{
		if (is_airport(icao_code))
		{
			std::lock_guard<std::mutex> lock(rnw_db_mutex);
			int n_runways = 0;
			runway_data tmp = rnw_db->at(icao_code);
			for (auto& it : tmp)
			{
				out->insert(it);
				n_runways++;
			}
			return n_runways;
		}
		return 0;
	}

	int ArptDB::get_rnw_data(std::string apt_icao, std::string rnw_id, runway_entry* out)
	{
		if (is_airport(apt_icao))
		{
			std::lock_guard<std::mutex> lock(rnw_db_mutex);
			runway_data tmp = rnw_db->at(apt_icao);

			if (tmp.find(rnw_id) != tmp.end())
			{
				*out = tmp.at(rnw_id);
				return 1;
			}
		}
		return 0;
	}


	// Private member functions:

	bool ArptDB::does_db_exist(std::string path, std::string sign)
	{
		std::ifstream file(path, std::ifstream::in);
		if (file.is_open())
		{
			std::string line;
			getline(file, line);
			if (line == sign)
			{
				file.close();
				return true;
			}
		}
		file.close();
		return false;
	}

	double ArptDB::parse_runway(std::string line, std::vector<runway>* rnw)
	{
		std::stringstream s(line);
		int limit_1 = N_RNW_ITEMS_IGNORE_BEGINNING + 1; // Add 1 because we don't need the row code
		int limit_2 = N_RNW_ITEMS_IGNORE_END;
		std::string junk;
		runway rnw_1;
		runway rnw_2;
		for (int i = 0; i < limit_1; i++)
		{
			s >> junk;
		}
		s >> rnw_1.id >> rnw_1.data.start.lat_deg >> rnw_1.data.start.lon_deg >> rnw_1.data.displ_threshold_m;
		for (int i = 0; i < limit_2; i++)
		{
			s >> junk;
		}
		s >> rnw_2.id >> rnw_1.data.end.lat_deg >> rnw_1.data.end.lon_deg >> rnw_2.data.displ_threshold_m;
		rnw_2.data.start.lat_deg = rnw_1.data.end.lat_deg;
		rnw_2.data.start.lon_deg = rnw_1.data.end.lon_deg;
		rnw_2.data.end.lat_deg = rnw_1.data.start.lat_deg;
		rnw_2.data.end.lon_deg = rnw_1.data.start.lon_deg;

		rnw->push_back(rnw_1);
		rnw->push_back(rnw_2);

		return rnw_1.data.get_implied_length_meters();
	}

	void ArptDB::add_to_arpt_queue(airport arpt)
	{
		std::lock_guard<std::mutex> lock(arpt_queue_mutex);
		arpt_queue.push_back(arpt);
	}

	void ArptDB::add_to_rnw_queue(rnw_data rnw)
	{
		std::lock_guard<std::mutex> lock(rnw_queue_mutex);
		rnw_queue.push_back(rnw);
	}
}