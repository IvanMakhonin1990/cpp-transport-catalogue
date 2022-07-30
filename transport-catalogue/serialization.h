#pragma once
#include <unordered_map>
#include <string_view>
#include <fstream>
#include <cassert>

#include "transport_catalogue.h"

//#include <transport_catalogue.pb.h>

namespace Transport {
	namespace Serialization {
		void Serialize(const TransportCatalogue& transportCatalogue,  std::ofstream& out);
		Transport::TransportCatalogue DeserializeTransportCatalogue(std::istream& in);
	}
	
}