#pragma once

#include "json.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <string>
#include <iostream>

namespace Transport {
	namespace json_reader {

		json::Document ReadAndProcessDocument(std::istream& input);
		svg::Document ReadAndProcessSvgDocument(std::istream& input);
		
		TransportCatalogue FillTransportCatalogue(const json::Node& requests);

		json::Document FillOutputRequests(const Transport::TransportCatalogue& transport, const json::Node& requests, const json::Node& map_settings);
	}
}  // namespace Transport
