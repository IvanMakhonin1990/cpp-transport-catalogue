
#include <transport_catalogue.pb.h>
#include "serialization.h"
#include "domain.h"


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
			std::unordered_map<std::string_view, int> stop_to_id;
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

			auto& buses = transportCatalogue.GetAllBuses();
			for (auto& bus : buses) {
				auto busPtr = transportCatalogueInfo->add_buses();
				busPtr->set_name(bus.second->name);
				busPtr->set_is_roundtrip(bus.second->is_roundtrip);
				for (auto& stop : bus.second->stops) {
					auto it = stop_to_id.find(stop->name);
					assert(stop_to_id.end() != it);
					busPtr->add_stops(it->second);
				}
			}
		}
		void Serializator::SetMapRenderer(const Renderer::MapRenderer& mapRenderer)
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
			auto t = mapRendererInfo->m_underlayer_color();
			mapRendererInfo->set_m_underlayer_width(mapRenderer.m_underlayer_width);
			
			for (auto& color : mapRenderer.m_color_palette) {
				auto cPtr = mapRendererInfo->add_m_color_palette();
				f(cPtr, color);
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

			std::unordered_map<int, std::string_view> id_to_name;
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

			if (mapRendererInfo.m_color_palette().size()>0) {
				renderer.m_color_palette.clear();
			}
			for (auto c : mapRendererInfo.m_color_palette())
			{
				renderer.m_color_palette.push_back(svg::Color());
				f(c, renderer.m_color_palette.back());
			}
			return renderer;
		}
}
}



	