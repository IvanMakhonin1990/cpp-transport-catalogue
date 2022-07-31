#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "json.h"
#include "serialization.h"


#include <vector>
#include <list>
#include <algorithm>
#include <cassert>

using namespace std;

//

using namespace Transport;

using namespace Transport::domain;

using namespace Router;

namespace Transport::JsonReader {


  json::Document JSONReader::ReadAndProcessDocument(istream& input) {
    const auto document = json::Load(input);
    const auto& t = document.GetRoot().AsDict();
    FillTransportCatalogue(t.at("base_requests"), t.at("routing_settings"));

    return document;//FillOutputRequests(t.at("stat_requests"), ParseRenderSettings(t.at("render_settings")));
  }

  svg::Document JSONReader::ReadAndProcessSvgDocument(std::istream& input)
  {
    const auto document = json::Load(input);
    const auto& t = document.GetRoot().AsDict();
    FillTransportCatalogue(t.at("base_requests"), t.at("routing_settings"));
    Transport::Renderer::MapRenderer mr;
    return mr.RenderBuses(m_transport_catalogue);
  }

  json::Document JSONReader::FillOutputRequests(const json::Node& requests, Transport::Serialization::Deserializator& ds) {
    using namespace json;
    auto map_renderer = ds.GetMapRenderer();
    router_ = ds.GetRouter(m_transport_catalogue);
    RequestHandler request_handler(m_transport_catalogue, map_renderer);

    auto& arrayRequest = requests.AsArray();

    json::Array arr;
    arr.reserve(arrayRequest.size());
    json::Builder builder;
    auto arr_context = builder.StartArray();
    for (const auto& request : arrayRequest) {
      auto& items = request.AsDict();
      auto& type = items.at("type").AsString();
      auto& id = items.at("id");
      if ("Bus" == type) {

        auto bus_stat = request_handler.GetBusStat(items.at("name").AsString());
        if (bus_stat.has_value()) {
          auto& v = bus_stat.value();
          arr_context.StartDict()
            .Key("curvature").Value(v.curvature)
            .Key("request_id").Value(id.AsInt())
            .Key("route_length").Value(v.route_length)
            .Key("stop_count").Value(v.stop_count)
            .Key("unique_stop_count").Value(v.unique_stop_count)
            .EndDict();
        }
        else {
          arr_context.StartDict()
            .Key("request_id"s).Value(id.AsInt())
            .Key("error_message"s).Value("not found"s)
            .EndDict();
        }
      }
      if ("Stop" == type) {
        auto stops = request_handler.GetBusesByStop(items.at("name").AsString());
        if (!stops.has_value()) {
          arr_context.StartDict()
            .Key("request_id").Value(id.AsInt())
            .Key("error_message").Value("not found"s)
            .EndDict();
        }
        else {
          json::Array stops_array;
          stops_array.reserve(stops.value().size());
          for (const auto& s : stops.value()) {
            stops_array.push_back(std::move(s));
          }
          std::sort(stops_array.begin(), stops_array.end(), [](const json::Node& lhs, const json::Node& rhs) {
            return std::lexicographical_compare(lhs.AsString().begin(), lhs.AsString().end(),
              rhs.AsString().begin(), rhs.AsString().end()); });
          auto stops_context = arr_context.StartDict()
            .Key("buses").StartArray();
          for (auto& s : stops_array) {
            stops_context.Value(s.AsString());
          }
          stops_context.EndArray().Key("request_id").Value(id.AsInt()).EndDict();

        }
      }
      if ("Map" == type) {
        std::stringstream ss;
        auto doc = request_handler.RenderMap();
        doc.Render(ss);
        arr_context.StartDict()
          .Key("map").Value(ss.str())
          .Key("request_id").Value(id.AsInt())
          .EndDict();
      }
      if ("Route" == type) {
        RouteStat(request, builder);
      }
    }
    return Document(arr_context.EndArray().Build());
  }
  template<typename T>
  T AddColor(const json::Node& node) {
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
    return  svg::Rgba();
  }

  Transport::Renderer::MapRenderer JSONReader::ParseRenderSettings(const json::Node& node)
  {
    Transport::Renderer::MapRenderer map_renderer;
    assert(node.IsDict());
    auto& dict = node.AsDict();
    for (const auto& item : dict) {
      if ("width" == item.first) {
        map_renderer.m_width = item.second.AsDouble();
      }
      else if ("height" == item.first) {
        map_renderer.m_height = item.second.AsDouble();
      }
      else if ("padding" == item.first) {
        map_renderer.m_padding = item.second.AsDouble();
      }
      else if ("line_width" == item.first) {
        map_renderer.m_line_width = item.second.AsDouble();
      }
      else if ("stop_radius" == item.first) {
        map_renderer.m_stop_radius = item.second.AsDouble();
      }
      else if ("bus_label_font_size" == item.first) {
        map_renderer.m_bus_label_font_size = item.second.AsInt();
      }
      else if ("bus_label_offset" == item.first) {
        assert(item.second.IsArray());
        auto& arr = item.second.AsArray();
        assert(2 == arr.size());
        map_renderer.m_bus_label_offset_dx = arr.at(0).AsDouble();
        map_renderer.m_bus_label_offset_dy = arr.at(1).AsDouble();
      }
      else if ("stop_label_font_size" == item.first) {
        map_renderer.m_stop_label_font_size = item.second.AsInt();
      }
      else if ("stop_label_offset" == item.first) {
        assert(item.second.IsArray());
        auto& arr = item.second.AsArray();
        assert(2 == arr.size());
        map_renderer.m_stop_label_offset_dx = arr.at(0).AsDouble();
        map_renderer.m_stop_label_offset_dy = arr.at(1).AsDouble();
      }
      else if ("underlayer_color" == item.first) {
        map_renderer.m_underlayer_color = AddColor<svg::Color>(item.second);
      }
      else if ("underlayer_width" == item.first) {
        map_renderer.m_underlayer_width = item.second.AsDouble();
      }
      else if ("color_palette" == item.first) {
        auto& arr = item.second.AsArray();
        map_renderer.m_color_palette = std::vector<svg::Color>(arr.size());
        std::transform(arr.begin(), arr.end(), map_renderer.m_color_palette.begin(), [](const json::Node& n) {return AddColor<svg::Color>(n); });
      }
    }
    assert(map_renderer.m_width >= 0.0 && map_renderer.m_width <= 100000.0);
    assert(map_renderer.m_height >= 0.0 && map_renderer.m_height <= 100000.0);
    assert(map_renderer.m_padding >= 0.0 && map_renderer.m_padding < std::min(map_renderer.m_width, map_renderer.m_height) / 2.0);
    assert(map_renderer.m_line_width >= 0.0 && map_renderer.m_line_width <= 100000.0);
    assert(map_renderer.m_stop_radius >= 0.0 && map_renderer.m_stop_radius <= 100000.0);
    assert(map_renderer.m_bus_label_font_size >= 0.0 && map_renderer.m_bus_label_font_size <= 100000.0);
    assert(map_renderer.m_bus_label_offset_dx >= -100000.0 && map_renderer.m_bus_label_offset_dx <= 100000.0);
    assert(map_renderer.m_bus_label_offset_dy >= -100000.0 && map_renderer.m_bus_label_offset_dy <= 100000.0);
    assert(map_renderer.m_stop_label_font_size >= 0.0 && map_renderer.m_stop_label_font_size <= 100000.0);
    assert(map_renderer.m_stop_label_offset_dx >= -100000.0 && map_renderer.m_stop_label_offset_dx <= 100000.0);
    assert(map_renderer.m_stop_label_offset_dy >= -100000.0 && map_renderer.m_stop_label_offset_dy <= 100000.0);
    assert(map_renderer.m_underlayer_width >= 0.0 && map_renderer.m_underlayer_width <= 100000.0);
    return map_renderer;
  }

  const Transport::TransportCatalogue& JSONReader::GetTransportCatalogue() const
  {
    return m_transport_catalogue;
  }

  void JSONReader::FillTransportCatalogue(const json::Node& requests, const json::Node& routing_settings) {
    m_transport_catalogue.SetBusWaitTime(routing_settings.AsDict().at("bus_wait_time").AsInt());
    m_transport_catalogue.SetBusVelocity(routing_settings.AsDict().at("bus_velocity").AsInt());
    auto& arrayRequest = requests.AsArray();
    std::list<std::tuple<std::string, std::string, double>> distances;
    for (auto& item : arrayRequest) {
      Geo::Coordinates coordinates;
      const auto& attributes = item.AsDict();
      if ("Stop" != attributes.at("type").AsString()) {
        continue;
      }
      const auto& stop_name = attributes.at("name").AsString();
      for (auto& attr : attributes)
      {
        if ("latitude" == attr.first) {
          coordinates.lat = move(attr.second.AsDouble());
        }
        else if ("longitude" == attr.first) {
          coordinates.lng = move(attr.second.AsDouble());
        }
        else if ("road_distances" == attr.first) {
          for (auto& dist : attr.second.AsDict()) {
            distances.push_back({ stop_name, move(dist.first), move(dist.second.AsDouble()) });
          }
        }
      }
      m_transport_catalogue.AddStop(stop_name, coordinates);
    }
    m_transport_catalogue.AddStopsDistances<std::list<std::tuple<std::string, std::string, double>>>(distances);

    for (auto& item : arrayRequest) {
      const auto& attributes = item.AsDict();
      if ("Bus" != attributes.at("type").AsString()) {
        continue;
      }
      auto stops = attributes.at("stops").AsArray();
      std::vector<std::string_view> stops_view;
      stops_view.reserve(stops.size());
      for (auto& stops_item : attributes.at("stops").AsArray()) {
        stops_view.push_back(stops_item.AsString());
      }
      if (attributes.at("is_roundtrip").AsBool()) {
        //stops_view.push_back(stops_view.front());
      }
      else {
        stops_view.insert(stops_view.end(), stops_view.rbegin() + 1,
          stops_view.rend());

      }
      m_transport_catalogue.AddBus(attributes.at("name").AsString(), stops_view, attributes.at("is_roundtrip").AsBool());
    }

  }

  void JSONReader::FillTransportCatalogue(const std::map<std::string, json::Node>& requests) {
      if (requests.end() != requests.find("routing_settings")) {
          m_transport_catalogue.SetBusWaitTime(requests.at("routing_settings").AsDict().at("bus_wait_time").AsInt());
          m_transport_catalogue.SetBusVelocity(requests.at("routing_settings").AsDict().at("bus_velocity").AsInt());
      }
      
      auto& arrayRequest = requests.at("base_requests").AsArray();
      auto mp = ParseRenderSettings(requests.at("render_settings"));
      std::list<std::tuple<std::string, std::string, double>> distances;

      for (auto& item : arrayRequest) {
          Geo::Coordinates coordinates;
          const auto& attributes = item.AsDict();
          if ("Stop" != attributes.at("type").AsString()) {
              continue;
          }
          const auto& stop_name = attributes.at("name").AsString();
          for (auto& attr : attributes)
          {
              if ("latitude" == attr.first) {
                  coordinates.lat = move(attr.second.AsDouble());
              }
              else if ("longitude" == attr.first) {
                  coordinates.lng = move(attr.second.AsDouble());
              }
              else if ("road_distances" == attr.first) {
                  for (auto& dist : attr.second.AsDict()) {
                      distances.push_back({ stop_name, move(dist.first), move(dist.second.AsDouble()) });
                  }
              }
          }
          m_transport_catalogue.AddStop(stop_name, coordinates);
      }
      m_transport_catalogue.AddStopsDistances<std::list<std::tuple<std::string, std::string, double>>>(distances);

      for (auto& item : arrayRequest) {
          const auto& attributes = item.AsDict();
          if ("Bus" != attributes.at("type").AsString()) {
              continue;
          }
          auto stops = attributes.at("stops").AsArray();
          std::vector<std::string_view> stops_view;
          stops_view.reserve(stops.size());
          for (auto& stops_item : attributes.at("stops").AsArray()) {
              stops_view.push_back(stops_item.AsString());
          }
          if (attributes.at("is_roundtrip").AsBool()) {
              //stops_view.push_back(stops_view.front());
          }
          else {
              stops_view.insert(stops_view.end(), stops_view.rbegin() + 1,
                  stops_view.rend());

          }
          m_transport_catalogue.AddBus(attributes.at("name").AsString(), stops_view, attributes.at("is_roundtrip").AsBool());
      }

  }

  
  void JSONReader::GetJsonRoute(const Router::TransporRouter::Result& route, json::Builder& builder)
  {
    builder.StartArray();
    for (auto& act : route.m_atctions) {
      visit(JsonFormer{ builder }, act);
    }
    builder.EndArray();
  }

  void JSONReader::RouteStat(const json::Node& route_request, json::Builder& builder) {

    const auto& items = route_request.AsDict();

    const int id = items.at("id"s).AsInt();
    const Stop* from = &m_transport_catalogue.FindStop(items.at("from"s).AsString());
    const Stop* to = &m_transport_catalogue.FindStop(items.at("to"s).AsString());

    if (from && to) {
      if (!router_) {
        router_ = std::make_unique<TransporRouter>(m_transport_catalogue);
      }
      auto router = router_->Route(from, to);
      if (router) {
        builder.StartDict()
          .Key("request_id"s).Value(id)
          .Key("total_time"s).Value(router->m_total_time)
          .Key("items"s);
        GetJsonRoute(*router, builder);
        builder.EndDict();
        return;
      }
    }

    builder
      .StartDict()
      .Key("request_id"s).Value(id)
      .Key("error_message"s).Value("not found"s)
      .EndDict();
  }
}