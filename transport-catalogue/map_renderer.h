#pragma once

#include "svg.h"
#include "json.h"
#include "domain.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "json_builder.h"

#include <vector>

namespace Transport {
	namespace Renderer {
		class MapRenderer {
		public:
			svg::Document RenderBuses(const Transport::TransportCatalogue& catalogue) const;
			json::Node GetRenderedNode(const Transport::TransportCatalogue& catalogue);

		public:
			double m_width = 1200.0;
			double m_height = 1200.0;
			double m_padding = 50.0;
			double m_line_width = 14.0;
			//stops circles radius
			double m_stop_radius = 5.0;
			//text size for buses
			size_t m_bus_label_font_size = 20;
			//dx of text
			double m_bus_label_offset_dx = 7.0;
			//dy of text
			double m_bus_label_offset_dy = 15.0;
			//size of text
			size_t m_stop_label_font_size = 20;
			//offset dx of text
			double m_stop_label_offset_dx = 7.0;
			//offset dy of text
			double m_stop_label_offset_dy = -3.0;
			svg::Color m_underlayer_color = svg::Rgba(255, 255, 255, 0.85);
			//stroke-width
			double m_underlayer_width = 3.0;

			std::vector<svg::Color> m_color_palette = { "green" , svg::Rgb(255, 160, 0), "red" };

		private:
			void RenderBusesNames(const std::vector<std::string_view>& buses, const Transport::TransportCatalogue& catalogue, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const;
			void RenderLines(const std::vector<std::string_view>& buses, const Transport::TransportCatalogue& catalogue, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const;
			void RenderCircles(const std::vector<Transport::domain::Stop*>& stops, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const;
			void RenderStopsNames(const std::vector<Transport::domain::Stop*>& stops, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const;
		};
	}
}
