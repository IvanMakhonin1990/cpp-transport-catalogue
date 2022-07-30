#include "map_renderer.h"

#include <cassert>
#include <algorithm>
//#include <execution>
#include <cassert>
#include <mutex>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

using namespace Transport::Geo;

void Transport::Renderer::MapRenderer::RenderBusesNames(const std::vector<std::string_view>& buses, const Transport::TransportCatalogue& catalogue, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const {
	size_t i = 0;
	for (auto& bus_name : buses) {
		auto& bus = catalogue.FindBus(bus_name);
		if (bus.stops.empty()) {
			continue;
		}
		{
			svg::Text under;
			under.SetFillColor(m_underlayer_color);
			under.SetStrokeColor(m_underlayer_color);
			under.SetStrokeWidth(m_underlayer_width);
			under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			under.SetPosition(projector(bus.stops.front()->position));
			under.SetOffset({ m_bus_label_offset_dx,m_bus_label_offset_dy });
			under.SetFontSize(m_bus_label_font_size);
			under.SetFontFamily("Verdana");
			under.SetFontWeight("bold");
			under.SetData(std::string(bus_name));
			doc.Add(under);
			svg::Text t;

			t.SetPosition(projector(bus.stops.front()->position));
			t.SetOffset({ m_bus_label_offset_dx,m_bus_label_offset_dy });
			t.SetFontSize(m_bus_label_font_size);
			t.SetFontFamily("Verdana");
			t.SetFontWeight("bold");
			t.SetData(std::string(bus_name));
			t.SetFillColor(m_color_palette[i]);
			doc.Add(t);
		}

		if (!bus.is_roundtrip && bus.stops[bus.stops.size() / 2]->name != bus.stops.front()->name)
		{
			svg::Text under;
			under.SetFillColor(m_underlayer_color);
			under.SetStrokeColor(m_underlayer_color);
			under.SetStrokeWidth(m_underlayer_width);
			under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			under.SetPosition(projector(bus.stops[bus.stops.size() / 2]->position));
			under.SetOffset({ m_bus_label_offset_dx,m_bus_label_offset_dy });
			under.SetFontSize(m_bus_label_font_size);
			under.SetFontFamily("Verdana");
			under.SetFontWeight("bold");
			under.SetData(std::string(bus_name));
			doc.Add(under);
			svg::Text t;

			t.SetPosition(projector(bus.stops[bus.stops.size() / 2]->position));
			t.SetOffset({ m_bus_label_offset_dx,m_bus_label_offset_dy });
			t.SetFontSize(m_bus_label_font_size);
			t.SetFontFamily("Verdana");
			t.SetFontWeight("bold");
			t.SetData(std::string(bus_name));
			t.SetFillColor(m_color_palette[i]);
			doc.Add(t);
		}

		++i;
		if (m_color_palette.size() <= i) {
			i = 0;
		}
	}
}

void Transport::Renderer::MapRenderer::RenderLines(const std::vector<std::string_view>& buses, const Transport::TransportCatalogue& catalogue, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const {
	size_t i = 0;
	for (auto& bus_name : buses) {
		auto& bus = catalogue.FindBus(bus_name);
		if (bus.stops.empty()) {
			continue;
		}
		svg::Polyline p;
		p.SetStrokeColor(m_color_palette[i]);
		p.SetFillColor("none");
		p.SetStrokeWidth(m_line_width);
		p.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		p.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		++i;
		if (m_color_palette.size() <= i) {
			i = 0;
		}

		//assert(!bus.name.empty());

		for (auto it = bus.stops.begin(); it != bus.stops.end(); ++it) {

			p.AddPoint(projector((*it)->position));//{(*it)->position.lng, (*it)->position.lat});
			//svg::li

		}
		doc.Add(p);
	}
}

void Transport::Renderer::MapRenderer::RenderCircles(const std::vector<Transport::domain::Stop*>& stops, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const {
	for (auto stop : stops) {
		svg::Circle circle;
		circle.SetCenter(projector(stop->position));
		circle.SetRadius(m_stop_radius);
		circle.SetFillColor("white");
		doc.Add(circle);
	}
}

void Transport::Renderer::MapRenderer::RenderStopsNames(const std::vector<Transport::domain::Stop*>& stops, const Transport::Geo::SphereProjector& projector, svg::Document& doc) const {
	for (auto stop : stops) {
		svg::Text under;
		under.SetFillColor(m_underlayer_color);
		under.SetStrokeColor(m_underlayer_color);
		under.SetStrokeWidth(m_underlayer_width);
		under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		under.SetPosition(projector(stop->position));
		under.SetOffset({ m_stop_label_offset_dx,m_stop_label_offset_dy });
		under.SetFontSize(m_stop_label_font_size);
		under.SetFontFamily("Verdana");
		under.SetData(stop->name);
		doc.Add(under);

		svg::Text t;

		t.SetPosition(projector(stop->position));
		t.SetOffset({ m_stop_label_offset_dx,m_stop_label_offset_dy });
		t.SetFontSize(m_stop_label_font_size);
		t.SetFontFamily("Verdana");
		//t.SetFontWeight("bold");
		t.SetData(std::string(stop->name));
		t.SetFillColor("black");
		doc.Add(t);
	}
}

svg::Document Transport::Renderer::MapRenderer::RenderBuses(const Transport::TransportCatalogue& catalogue) const
{
	svg::Document doc;
	auto& stops = catalogue.GetStops();
	std::vector<Transport::Geo::Coordinates> points;
	points.reserve(stops.size());

	std::for_each(stops.begin(), stops.end(), [&catalogue, &points](const std::pair< std::string_view, domain::Stop*> stop) {
		if (catalogue.IsStopWithBuses(stop.first)) {
			points.push_back(stop.second->position);
		}
		});

	Transport::Geo::SphereProjector projector(points.begin(), points.end(), m_width, m_height, m_padding);
	
	auto& buses = catalogue.GetBusesNames();

	RenderLines(buses, catalogue, projector, doc);
	RenderBusesNames(buses, catalogue, projector, doc);
	
	
	std::vector<Transport::domain::Stop*> tmp_stops;
	tmp_stops.reserve(stops.size());

	std::for_each(stops.begin(), stops.end(), [&catalogue, &tmp_stops](const std::pair< std::string_view, domain::Stop*> stop) {
		if (catalogue.IsStopWithBuses(stop.first)) {
			tmp_stops.push_back(stop.second);
		}
		});
	std::sort(/*std::execution::par, */tmp_stops.begin(), tmp_stops.end(),
		[](const Transport::domain::Stop* lhs, const Transport::domain::Stop* rhs) {
			return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(), rhs->name.end());
		});
	
	RenderCircles(tmp_stops, projector, doc);
	RenderStopsNames(tmp_stops, projector, doc);

	return doc;
}

json::Node Transport::Renderer::MapRenderer::GetRenderedNode(const Transport::TransportCatalogue& catalogue)
{
	std::stringstream ss;
	auto doc = RenderBuses(catalogue);
	doc.Render(ss);
	return json::Node(ss.str());
}
