#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"


#include <vector>
#include <list>
#include <algorithm>

using namespace std;

//

using namespace Transport;

using namespace Transport::domain;

namespace Transport::json_reader {

   
    json::Document ReadAndProcessDocument(istream& input) {
        const auto document = json::Load(input);
        const auto& t = document.GetRoot().AsMap();
        auto catalogue = FillTransportCatalogue(t.at("base_requests"));

        return FillOutputRequests(catalogue, t.at("stat_requests"), t.at("render_settings"));
    }

    svg::Document ReadAndProcessSvgDocument(std::istream& input)
    {
        const auto document = json::Load(input);
        const auto& t = document.GetRoot().AsMap();
        auto catalogue = FillTransportCatalogue(t.at("base_requests"));
        Transport::renderer::MapRenderer mr(t.at("render_settings"));
        return mr.RenderBuses(catalogue);
    }

    json::Document FillOutputRequests(const TransportCatalogue& transport, const json::Node& requests, const json::Node& map_settings) {
        using namespace json;
        Transport::renderer::MapRenderer mr(map_settings);
        RequestHandler request_handler(transport, mr);
        
        auto& arrayRequest = requests.AsArray();
        
        json::Array arr;
        arr.reserve(arrayRequest.size());

        for (const auto& request : arrayRequest) {
            auto& items = request.AsMap();
            auto& type = items.at("type").AsString();
            auto& id = items.at("id");
            if ("Bus" == type) {
                
                auto bus_stat = request_handler.GetBusStat(items.at("name").AsString());
                if (bus_stat.has_value()) {
                    auto& v = bus_stat.value();
                    Dict d = { 
                        {"curvature", v.curvature}, 
                        {"request_id", id}, 
                        {"route_length", v.route_length}, 
                        {"stop_count", v.stop_count},
                        {"unique_stop_count", v.unique_stop_count} 
                    };
                    arr.push_back(std::move(d));
                } else {
                    Dict d = {
                        {"request_id", id},
                        {"error_message", "not found"s}
                    };
                    arr.push_back(std::move(d));
                }
            }
            if ("Stop" == type) {
                auto stops = request_handler.GetBusesByStop(items.at("name").AsString());
                if (!stops.has_value()) {
                    Dict d = {
                        {"request_id", id},
                        {"error_message", "not found"s}
                    };
                    arr.push_back(std::move(d));
                } else {
                    json::Array stops_array;
                    stops_array.reserve(stops.value().size());
                    for (const auto& s : stops.value()) {
                        stops_array.push_back(std::move(s));
                    }
                    std::sort(stops_array.begin(), stops_array.end(), [](const json::Node& lhs, const json::Node& rhs) {
                        return std::lexicographical_compare(lhs.AsString().begin(), lhs.AsString().end(),
                            rhs.AsString().begin(), rhs.AsString().end()); });
                    Dict d = {
                        {"buses", std::move(stops_array)},
                        {"request_id", id}
                    };
                    arr.push_back(std::move(d));
                }
            }
            if ("Map" == type) {
                std::stringstream ss;
                auto doc = request_handler.RenderMap();
                doc.Render(ss);
                Dict d = {
                        {"map", json::Node(ss.str())},
                        {"request_id", id}
                };
                arr.push_back(std::move(d));
            }
        }
        return Document(std::move(arr));
    }

    TransportCatalogue FillTransportCatalogue(const json::Node& requests) {
        TransportCatalogue transport_catalogue;
        auto& arrayRequest = requests.AsArray();
        std::list<std::tuple<std::string, std::string, double>> distances;
        for (auto& item : arrayRequest) {
            Geo::Coordinates coordinates;
            const auto& attributes = item.AsMap();
            if ("Stop" != attributes.at("type").AsString()) {
                continue;
            }
            const auto & stop_name = attributes.at("name").AsString();
            for (auto& attr : attributes)
            {
                if ("latitude" == attr.first) {
                    coordinates.lat = move(attr.second.AsDouble());
                }
                else if ("longitude" == attr.first) {
                    coordinates.lng = move(attr.second.AsDouble());
                }
                else if ("road_distances" == attr.first) {
                    for (auto& dist : attr.second.AsMap()) {
                        distances.push_back({ stop_name, move(dist.first), move(dist.second.AsDouble()) });
                    }
                }
            }
            transport_catalogue.AddStop(stop_name, coordinates);
        }
        transport_catalogue.AddStopsDistances<std::list<std::tuple<std::string, std::string, double>>>(distances);

        for (auto& item : arrayRequest) {
            const auto& attributes = item.AsMap();
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
                 stops_view.insert(stops_view.end(), stops_view.rbegin()+1,
                    stops_view.rend());

            }
            transport_catalogue.AddBus(attributes.at("name").AsString(), stops_view, attributes.at("is_roundtrip").AsBool());
        }

        return transport_catalogue;
    }
}