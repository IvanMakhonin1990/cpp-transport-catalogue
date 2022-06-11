#pragma once
#include "geo.h"

#include <string>
#include <vector>

namespace Transport {
	namespace domain {
		struct Stop {
			std::string name;
			Transport::Geo::Coordinates position;
		};

		struct Bus {
			std::string name;
			std::vector<Stop*> stops;
			double distance = 0;
			double curvature = 0;
			size_t unique_stops = 0;
			bool is_roundtrip = false;
		};
		struct BusStat {
			double curvature;
			double route_length;
			int stop_count;
			int unique_stop_count;
		};
		

	}
}