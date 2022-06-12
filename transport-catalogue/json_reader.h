#pragma once

#include "json.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <string>
#include <iostream>

namespace Transport {
	namespace JsonReader {
		class JSONReader {
		public:
			json::Document ReadAndProcessDocument(std::istream& input);
			svg::Document ReadAndProcessSvgDocument(std::istream& input);

			TransportCatalogue FillTransportCatalogue(const json::Node& requests);

			json::Document FillOutputRequests(const Transport::TransportCatalogue& transport, const json::Node& requests, const Transport::Renderer::MapRenderer& map_renderer );

			Transport::Renderer::MapRenderer ParseRenderSettings(const json::Node& node);
		};
	}
}  // namespace Transport
