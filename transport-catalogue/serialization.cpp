
#include <transport_catalogue.pb.h>
#include "serialization.h"
#include "domain.h"
#include "transport_router.h"
#include "graph.h"


namespace Transport {
	namespace Serialization {



		void Serializator::Serialize(std::ofstream& out)
		{
			dataInfo.SerializePartialToOstream(&out);
		}
		void Serializator::AddTransportCatalogue(const TransportCatalogue& transportCatalogue)
		{
			auto transportCatalogueInfo = dataInfo.mutable_transportcatalogueinfo();
			transportCatalogueInfo->set_m_bus_velocity(transportCatalogue.GetBusVelocity());
			transportCatalogueInfo->set_m_bus_wait_time(transportCatalogue.GetBusWaitTime());

			//Serialize(transportCatalogue)
			//Stops
			
			auto& stops = transportCatalogue.GetAllStops();
			int i = 0;
			for (auto& stop : stops)
			{
				auto stopPtr = transportCatalogueInfo->add_stops();
				stopPtr->set_id(i);
				stop_to_id.emplace(stop.name, i);
				stopPtr->set_name(stop.name);
				stopPtr->mutable_position()->set_lat(stop.position.lat);
				stopPtr->mutable_position()->set_lng(stop.position.lng);
				////
				++i;
			}

			auto& distances = transportCatalogue.GetDistances();
			for (auto& distance : distances) {
				auto distancePtr = transportCatalogueInfo->add_distances();
				auto it1 = stop_to_id.find(distance.first.first->name);
				assert(stop_to_id.end() != it1);
				distancePtr->set_stop1id(it1->second);
				auto it2 = stop_to_id.find(distance.first.second->name);
				assert(stop_to_id.end() != it2);
				distancePtr->set_stop2id(it2->second);
				distancePtr->set_distance(distance.second);
			}
			i = 0;
			auto& buses = transportCatalogue.GetAllBuses();
			for (auto& bus : buses) {
				bus_to_id.emplace(bus.first, i);
				auto busPtr = transportCatalogueInfo->add_buses();
				busPtr->set_name(bus.second->name);
				busPtr->set_is_roundtrip(bus.second->is_roundtrip);
				busPtr->set_id(i);
				for (auto& stop : bus.second->stops) {
					auto it = stop_to_id.find(stop->name);
					assert(stop_to_id.end() != it);
					busPtr->add_stops(it->second);
				}
				++i;
			}
		}
		void Serializator::AddMapRenderer(const Renderer::MapRenderer& mapRenderer)
		{
			auto mapRendererInfo = dataInfo.mutable_maprendererinfo();
			mapRendererInfo->set_m_width(mapRenderer.m_width);
			mapRendererInfo->set_m_height(mapRenderer.m_height);
			mapRendererInfo->set_m_padding(mapRenderer.m_padding);
			mapRendererInfo->set_m_line_width(mapRenderer.m_line_width);
			mapRendererInfo->set_m_stop_radius(mapRenderer.m_stop_radius);
			mapRendererInfo->set_m_bus_label_font_size(mapRenderer.m_bus_label_font_size);
			mapRendererInfo->set_m_bus_label_offset_dx(mapRenderer.m_bus_label_offset_dx);
			mapRendererInfo->set_m_bus_label_offset_dy(mapRenderer.m_bus_label_offset_dy);
			mapRendererInfo->set_m_stop_label_font_size(mapRenderer.m_stop_label_font_size);
			mapRendererInfo->set_m_stop_label_offset_dx(mapRenderer.m_stop_label_offset_dx);
			mapRendererInfo->set_m_stop_label_offset_dy(mapRenderer.m_stop_label_offset_dy);

			auto f = [](transport_catalogue_serialize::Color* colorPtr, const svg::Color& c) {
				if (std::holds_alternative<svg::Rgb>(c)) {
					auto color = std::get< svg::Rgb>(c);
					auto rgbPtr = colorPtr->mutable_rgbcolor();
					rgbPtr->set_red(color.red);
					rgbPtr->set_green(color.green);
					rgbPtr->set_blue(color.blue);
				}
				else if (std::holds_alternative<svg::Rgba>(c)) {
					auto color = std::get< svg::Rgba>(c);
					auto rgbaPtr = colorPtr->mutable_rgbacolor();
					rgbaPtr->set_red(color.red);
					rgbaPtr->set_green(color.green);
					rgbaPtr->set_blue(color.blue);
					rgbaPtr->set_opacity(color.opacity);
				}
				else if (std::holds_alternative<std::string>(c)) {
					auto strPtr = colorPtr->mutable_stringcolor();
					*strPtr = std::get< std::string>(c);
				}
			};
			f(mapRendererInfo->mutable_m_underlayer_color(), mapRenderer.m_underlayer_color);
			mapRendererInfo->set_m_underlayer_width(mapRenderer.m_underlayer_width);

			for (auto& color : mapRenderer.m_color_palette) {
				auto cPtr = mapRendererInfo->add_m_color_palette();
				f(cPtr, color);
			}
		}

		void Serializator::AddRouter(const Router::TransporRouter& router)
		{
			auto edges = dataInfo.mutable_router()->mutable_edgeinfos();
			for (auto edge : router.GetEdges()) {
				transport_catalogue_serialize::EdgeInfo edgeInfo;
				edgeInfo.set_m_bus(bus_to_id.at(edge.m_bus->name));
				edgeInfo.set_m_is_wait(edge.m_is_wait);
				edgeInfo.set_m_source(stop_to_id.at(edge.m_source->name));
				edgeInfo.set_m_span(edge.m_span);
				edgeInfo.set_m_target(stop_to_id.at(edge.m_target->name));
				*edges->Add() = std::move(edgeInfo);
			}

			auto p_stops_by_id = dataInfo.mutable_router()->mutable_stop_by_id();
			for (auto [name, id] : router.GetStopVertixes()) {
				transport_catalogue_serialize::StopById stop_by_id;
				stop_by_id.set_id(id);
				stop_by_id.set_stop_id(stop_to_id.at(name->name));
				*p_stops_by_id->Add() = std::move(stop_by_id);
			}
			auto p_graph = dataInfo.mutable_router()->mutable_graph();

			for (auto& edge : router.GetGraph()->GetEdges()) {
				transport_catalogue_serialize::Edge p_edge;
				p_edge.set_from(edge.from);
				p_edge.set_to(edge.to);
				p_edge.set_weight(edge.weight);
				*(p_graph->add_edges()) = std::move(p_edge);
			}

			for (auto& list : router.GetGraph()->GetAllIncidentEdges()) {
				auto p_list = p_graph->add_incidence_lists();
				for (auto id : list) {
					p_list->add_edge_id(id);
				}
			}

			auto p_router = dataInfo.mutable_router()->mutable_router();

			for (const auto& data : router.GetRouter()->GetRoutesInternalData()) {
				transport_catalogue_serialize::RoutesInternalData p_data;
				for (const auto& intern : data) {
					transport_catalogue_serialize::OptionalRouteInternalData p_internal;
					if (intern.has_value()) {
						auto& value = intern.value();
						auto p_value = p_internal.mutable_route_internal_data();
						p_value->set_weight(value.weight);
						if (value.prev_edge.has_value()) {
							p_value->set_prev_edge(value.prev_edge.value());
						}
						else {
							p_value->set_isnull(true);
						}
					}
					else {
						p_internal.set_isnull(true);
					}
					    
					*p_data.add_routes_internal_data() = std::move(p_internal);
				}
				*p_router->add_routes_internal_data() = std::move(p_data);
			}
		}

		Deserializator::Deserializator(std::istream& in)
		{
			assert(dataInfo.ParseFromIstream(&in));
		}
		TransportCatalogue Deserializator::GetTransportCatalogue() const
		{
			auto transportCatalogueInfo = dataInfo.transportcatalogueinfo();

			Transport::TransportCatalogue transportCatalogue;

			transportCatalogue.SetBusVelocity(transportCatalogueInfo.m_bus_velocity());
			transportCatalogue.SetBusWaitTime(transportCatalogueInfo.m_bus_wait_time());

			std::list<std::tuple<std::string, std::string, double>> distances;

			
			for (auto& item : transportCatalogueInfo.stops()) {
				Geo::Coordinates coordinates;
				coordinates.lat = std::move(item.position().lat());
				coordinates.lng = std::move(item.position().lng());
				transportCatalogue.AddStop(item.name(), coordinates);
				id_to_name.emplace(item.id(), item.name());
			}
			for (auto& item : transportCatalogueInfo.distances()) {
				auto it1 = id_to_name.find(item.stop1id());
				assert(id_to_name.end() != it1);
				auto it2 = id_to_name.find(item.stop2id());
				assert(id_to_name.end() != it2);
				transportCatalogue.SetStopsDistance(it1->second, it2->second, item.distance());
			}
			for (auto& item : transportCatalogueInfo.buses()) {
				
				bus_id_to_name.emplace(item.id(), item.name());
				std::vector<std::string_view> stops_view;
				stops_view.reserve(item.stops().size());
				for (auto& stops_item : item.stops()) {
					auto it1 = id_to_name.find(stops_item);
					assert(id_to_name.end() != it1);
					stops_view.push_back(it1->second);
				}
				transportCatalogue.AddBus(item.name(), stops_view, item.is_roundtrip());
			}
			return transportCatalogue;
		}
		Renderer::MapRenderer Deserializator::GetMapRenderer() const
		{
			auto mapRendererInfo = dataInfo.maprendererinfo();
			Renderer::MapRenderer renderer;

			renderer.m_width = mapRendererInfo.m_width();
			renderer.m_height = mapRendererInfo.m_height();
			renderer.m_padding = mapRendererInfo.m_padding();
			renderer.m_line_width = mapRendererInfo.m_line_width();
			renderer.m_stop_radius = mapRendererInfo.m_stop_radius();
			renderer.m_bus_label_font_size = mapRendererInfo.m_bus_label_font_size();
			renderer.m_bus_label_offset_dx = mapRendererInfo.m_bus_label_offset_dx();
			renderer.m_bus_label_offset_dy = mapRendererInfo.m_bus_label_offset_dy();
			renderer.m_stop_label_font_size = mapRendererInfo.m_stop_label_font_size();
			renderer.m_stop_label_offset_dx = mapRendererInfo.m_stop_label_offset_dx();
			renderer.m_stop_label_offset_dy = mapRendererInfo.m_stop_label_offset_dy();
			auto f = [](const transport_catalogue_serialize::Color& color, svg::Color& newColor) {
				if (color.Value_case() == transport_catalogue_serialize::Color::ValueCase::kRgbaColor) {
					newColor = svg::Rgba(color.rgbacolor().red(), color.rgbacolor().green(),
						color.rgbacolor().blue(), color.rgbacolor().opacity());
				}
				if (color.Value_case() == transport_catalogue_serialize::Color::ValueCase::kRgbColor) {
					newColor = svg::Rgb(color.rgbcolor().red(), color.rgbcolor().green(),
						color.rgbcolor().blue());
				}
				if (color.Value_case() == transport_catalogue_serialize::Color::ValueCase::kStringColor) {
					newColor = color.stringcolor();
				}
			};

			if (mapRendererInfo.has_m_underlayer_color()) {
				auto color = mapRendererInfo.m_underlayer_color();
				f(mapRendererInfo.m_underlayer_color(), renderer.m_underlayer_color);
			}

			renderer.m_underlayer_width = mapRendererInfo.m_underlayer_width();

			if (mapRendererInfo.m_color_palette().size() > 0) {
				renderer.m_color_palette.clear();
			}
			for (auto c : mapRendererInfo.m_color_palette())
			{
				renderer.m_color_palette.push_back(svg::Color());
				f(c, renderer.m_color_palette.back());
			}
			return renderer;
		}
		std::unique_ptr<Router::TransporRouter> Deserializator::GetRouter(const TransportCatalogue& transportCatalogue)
		{
			auto ptr = std::make_unique< Router::TransporRouter>();
			auto& p_router = dataInfo.router();

			for (auto& edgeInfo : p_router.edgeinfos()) {
				Router::TransporRouter::EdgeInfo edge;
				edge.m_bus = &transportCatalogue.FindBus(bus_id_to_name.at(edgeInfo.m_bus()));
				edge.m_is_wait = edgeInfo.m_is_wait();
				edge.m_source = &transportCatalogue.FindStop(id_to_name.at(edgeInfo.m_source()));
				edge.m_target = &transportCatalogue.FindStop(id_to_name.at(edgeInfo.m_target()));
				edge.m_span = edgeInfo.m_span();
				ptr->GetEdges().push_back(edge);
			}

			auto stops_count = p_router.stop_by_id_size();
			for (auto i = 0; i < stops_count; ++i) {
				auto& p_stop_by_id = p_router.stop_by_id(i);
				auto& stop = transportCatalogue.FindStop(id_to_name.at(p_stop_by_id.stop_id()));
				ptr->GetStopVertixes().emplace(&stop, p_stop_by_id.id());
			}

			auto& p_graph = p_router.graph();
			auto edge_count = p_graph.edges_size();
			auto& graph = ptr->GetGraph();
			auto incidence_lists_count = p_graph.incidence_lists_size();
			for (auto i = 0; i < incidence_lists_count; ++i) {
				graph::DirectedWeightedGraph<double>::IncidenceList list;
				auto& p_list = p_graph.incidence_lists(i);
				auto list_count = p_list.edge_id_size();
				for (auto j = 0; j < list_count; ++j) {
					list.push_back(p_list.edge_id(j));
				}
				graph->GetAllIncidentEdges().push_back(list);
			}
			
			for (auto i = 0; i < edge_count; ++i) {
				graph::Edge<double> edge;//Edge<Weight>
				auto& p_edge = p_graph.edges(i);
				edge.from = p_edge.from();
				edge.to = p_edge.to();
				edge.weight = p_edge.weight();
				//graph->AddEdge(edge);
				//auto& es = graph->GetEdges();
				graph->GetEdges().push_back(std::move(edge));
			}

			

			auto& router_ = p_router.router();
			auto& routes_internal_data = ptr->GetRouter()->GetRoutesInternalData();
			auto routes_internal_data_count = router_.routes_internal_data_size();
			
			routes_internal_data.resize(routes_internal_data_count);

			for (int i = 0; i < routes_internal_data_count; ++i) {
				auto& p_internal_data = router_.routes_internal_data(i);
				auto internal_data_count = p_internal_data.routes_internal_data_size();
				routes_internal_data[i].resize(internal_data_count);
				for (int j = 0; j < internal_data_count; ++j) {
					
					auto& p_optional_data = p_internal_data.routes_internal_data(j);
					if (p_optional_data.optional_route_internal_data_case() ==
						transport_catalogue_serialize::OptionalRouteInternalData::kRouteInternalData) {
						graph::Router<double>::RouteInternalData data;
						auto& p_data = p_optional_data.route_internal_data();
						data.weight = p_data.weight();
						if (p_data.optional_prev_edge_case() ==
							transport_catalogue_serialize::RouteInternalData::kPrevEdge) {
							data.prev_edge = p_data.prev_edge();
						}
						else {
							data.prev_edge = std::nullopt;
						}
						routes_internal_data[i][j] = std::move(data);
					}
					else {
						routes_internal_data[i][j] = std::nullopt;
					}
				}
			}
			return ptr;
		}
	}
}



	