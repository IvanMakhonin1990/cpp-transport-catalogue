#pragma once

#include "json.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string>
#include <iostream>
#include <string_view>
#include <map>

namespace Transport {
	namespace JsonReader {

		struct JsonFormer {
			json::Builder& operator()(const Router::TransportRouter::WaitBus& act) const {
				return m_builder.StartDict()
					.Key("type").Value("Wait")
					.Key("stop_name").Value(act.stop->name)
					.Key("time").Value(act.m_time)
					.EndDict();
			}

			json::Builder& operator()(const Router::TransportRouter::MoveBus& act) const {
				return m_builder.StartDict()
					.Key("type").Value("Bus")
					.Key("bus").Value(act.m_bus->name)
					.Key("span_count").Value(act.m_span)
					.Key("time").Value(act.m_time)
					.EndDict();
			}

			json::Builder& m_builder;
		};

		class JSONReader {
		public:
			json::Document ReadAndProcessDocument(std::istream& input);
			svg::Document ReadAndProcessSvgDocument(std::istream& input);

			void FillTransportCatalogue(const json::Node& requests, const json::Node& routing_settings);
			
			void FillTransportCatalogue(const std::map<std::string, json::Node>& requests);
			
			template<typename T>
			void FillTransportCatalogue(const T& ds) {
			    m_transport_catalogue = ds.GetTransportCatalogue();
			}

			json::Document FillOutputRequests(const json::Node& requests, const Transport::Renderer::MapRenderer& map_renderer = Transport::Renderer::MapRenderer());

			Transport::Renderer::MapRenderer ParseRenderSettings(const json::Node& node);

			const Transport::TransportCatalogue& GetTransportCatalogue() const;

			void RouteStat(const json::Node& route_request, json::Builder& builder);

			void GetJsonRoute(const Router::TransportRouter::Result& route, json::Builder& builder);

		private:
			Transport::TransportCatalogue m_transport_catalogue;
			std::unique_ptr<Router::TransportRouter> router_;
	   };
	}
}  // namespace Transport
