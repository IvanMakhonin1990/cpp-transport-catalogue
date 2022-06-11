#include "map_renderer.h"

#include <cassert>
#include <algorithm>
#include <execution>
#include <cassert>
#include <mutex>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

using namespace Transport::Geo;

template<typename T>
T add_color(const json::Node& node) {
	if (node.IsString()) {
		return node.AsString();
	}
	else if (node.IsArray()) {
		auto& arr = node.AsArray();
		if (arr.size() == 3) {
			return svg::Rgb(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
		}
		if (arr.size() == 4) {
			return  svg::Rgba(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
		}
	}
	assert(false);
}

Transport::renderer::MapRenderer::MapRenderer(const json::Node& node)
{
	assert(node.IsMap());
	auto& dict = node.AsMap();
	for (const auto& item : dict) {
		if ("width" == item.first) {
			m_width = item.second.AsDouble();
		}
		else if ("height" == item.first) {
			m_height = item.second.AsDouble();
		}
		else if ("padding" == item.first) {
			m_padding = item.second.AsDouble();
		}
		else if ("line_width" == item.first) {
			m_line_width = item.second.AsDouble();
		}
		else if ("stop_radius" == item.first) {
			m_stop_radius = item.second.AsDouble();
		}
		else if ("bus_label_font_size" == item.first) {
			m_bus_label_font_size = item.second.AsInt();
		}
		else if ("bus_label_offset" == item.first) {
			assert(item.second.IsArray());
			auto& arr = item.second.AsArray();
			assert(2 == arr.size());
			m_bus_label_offset_dx = arr.at(0).AsDouble();
			m_bus_label_offset_dy = arr.at(1).AsDouble();
		}
		else if ("stop_label_font_size" == item.first) {
			m_stop_label_font_size = item.second.AsInt();
		}
		else if ("stop_label_offset" == item.first) {
			assert(item.second.IsArray());
			auto& arr = item.second.AsArray();
			assert(2 == arr.size());
			m_stop_label_offset_dx = arr.at(0).AsDouble();
			m_stop_label_offset_dy = arr.at(1).AsDouble();
		}
		else if ("underlayer_color" == item.first) {
			m_underlayer_color = add_color<svg::Color>(item.second);
		}
		else if ("underlayer_width" == item.first) {
			m_underlayer_width = item.second.AsDouble();
		}
		else if ("color_palette" == item.first) {
			auto& arr = item.second.AsArray();
			m_color_palette = std::vector<svg::Color>(arr.size());
			std::transform(arr.begin(), arr.end(), m_color_palette.begin(), [](const json::Node& n) {return add_color<svg::Color>(n); });
		}
	}
	assert(m_width >= 0.0 && m_width <= 100000.0);
	assert(m_height >= 0.0 && m_height <= 100000.0);
	assert(m_padding >= 0.0 && m_padding < std::min(m_width, m_height) / 2.0);
	assert(m_line_width >= 0.0 && m_line_width <= 100000.0);
	assert(m_stop_radius >= 0.0 && m_stop_radius <= 100000.0);
	assert(m_bus_label_font_size >= 0.0 && m_bus_label_font_size <= 100000.0);
	assert(m_bus_label_offset_dx >= -100000.0 && m_bus_label_offset_dx <= 100000.0);
	assert(m_bus_label_offset_dy >= -100000.0 && m_bus_label_offset_dy <= 100000.0);
	assert(m_stop_label_font_size >= 0.0 && m_stop_label_font_size <= 100000.0);
	assert(m_stop_label_offset_dx >= -100000.0 && m_stop_label_offset_dx <= 100000.0);
	assert(m_stop_label_offset_dy >= -100000.0 && m_stop_label_offset_dy <= 100000.0);
	assert(m_underlayer_width >= 0.0 && m_underlayer_width <= 100000.0);
}

svg::Document Transport::renderer::MapRenderer::RenderBuses(const Transport::TransportCatalogue& catalogue) const
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

	i = 0;
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
	i = 0;
	std::vector<Transport::domain::Stop*> tmp_stops;
	tmp_stops.reserve(stops.size());

	std::for_each(stops.begin(), stops.end(), [&catalogue, &tmp_stops](const std::pair< std::string_view, domain::Stop*> stop) {
		if (catalogue.IsStopWithBuses(stop.first)) {
			tmp_stops.push_back(stop.second);
		}
		});
	/*std::sort(std::execution::par, tmp_stops.begin(), tmp_stops.end(),
		[](const Transport::domain::Stop* lhs, const Transport::domain::Stop* rhs) {
			return std::lexicographical_compare(rhs->name.begin(), rhs->name.end(), lhs->name.begin(), lhs->name.end());
		});*/
	std::sort(std::execution::par, tmp_stops.begin(), tmp_stops.end(),
		[](const Transport::domain::Stop* lhs, const Transport::domain::Stop* rhs) {
			return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(), rhs->name.end());
		});
	for (auto stop : tmp_stops) {
		svg::Circle circle;
		circle.SetCenter(projector(stop->position));
		circle.SetRadius(m_stop_radius);
		circle.SetFillColor("white");
		doc.Add(circle);
	}
	//std::sort(std::execution::par, tmp_stops.begin(), tmp_stops.end(),
	//	[](const Transport::domain::Stop* lhs, const Transport::domain::Stop* rhs) {
	//		return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(), rhs->name.end());
	//	});
	for (auto stop : tmp_stops) {
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
	return doc;
}

json::Node Transport::renderer::MapRenderer::GetRenderedNode(const Transport::TransportCatalogue& catalogue)
{
	std::stringstream ss;
	auto doc = RenderBuses(catalogue);
	doc.Render(ss);
	return json::Node(ss.str());
}
