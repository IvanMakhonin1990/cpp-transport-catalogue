#include "transport_router.h"

#include <functional>
#include <cassert>
#include <stdexcept>
#include <numeric>
#include <iterator>

namespace Router {

    using namespace Transport;
    using namespace Transport::domain;
    using namespace std;
       
    TransportRouter::TransportRouter(const TransportCatalogue& transport_catalogue)
      : m_transport_catalogue(transport_catalogue) {
      graph::VertexId vertex_id = 0;
      double bus_velocity = m_transport_catalogue.GetBusVelocity() * 1000.0 / 60.0;
      double bus_wait_time = m_transport_catalogue.GetBusWaitTime();
      auto& stops = m_transport_catalogue.GetStops();
      for (auto it = stops.begin(); it != stops.end(); ++it) {
        stop_vertices_.insert({ &*(it->second), vertex_id++ });
      }
      m_graph = std::make_unique<graph::DirectedWeightedGraph<double>>(stop_vertices_.size());
      auto buses = m_transport_catalogue.GetAllBuses();
      for (auto it = buses.begin(); it != buses.end(); ++it) {
        AddBus(&*(it->second), bus_wait_time, bus_velocity);
      }
      assert(m_graph);
      m_router = std::make_unique<graph::Router<double>>(*m_graph);
    }

    optional<TransportRouter::Result> TransportRouter::Route(const Stop * from, const Stop * to) {
      
      auto route = m_router->BuildRoute(GetVertex(from),
        GetVertex(to));
      if (!route.has_value()) {
        return nullopt;
      }
      Result result;
      result.m_total_time = route->weight;

      for (const auto edge_id : route->edges) {
        const auto& edge_data = edges_[edge_id];
        const auto& edge = m_graph->GetEdge(edge_id);
        result.m_atctions.push_back(WaitBus{ edge_data.m_source, edge_data.m_is_wait });
        result.m_atctions.push_back(MoveBus({ edge_data.m_bus, edge_data.m_source,
                                                 edge_data.m_span, edge.weight - edge_data.m_is_wait }));
      }
      return result;
    }

    void TransportRouter::AddBus(const Transport::domain::Bus* bus, double bus_wait_m_time, double bus_velocity) {
      auto& stops = bus->stops;
      auto e1 = prev(stops.end());
      for (auto it1 = stops.begin(); it1 != e1; ++it1) {
        graph::VertexId vertex1 = GetVertex(*it1);
        uint32_t distance = 0;
        int span = 1;
        for (auto it2 = it1; it2 != e1; ++it2) {
          graph::VertexId vertex2 = GetVertex(*next(it2));
          distance += m_transport_catalogue.GetStopsDistance((*it2)->name, (*next(it2))->name);
          m_graph->AddEdge({ vertex1, vertex2, distance / bus_velocity + bus_wait_m_time });
          edges_.push_back({ bus_wait_m_time, *it1, *it2, span, bus });
          ++span;
        }
      }

      if (!bus->is_roundtrip) {
        auto e2 = prev(stops.rend());
        for (auto it1 = stops.rbegin(); it1 != e2; ++it1) {
          graph::VertexId vertex1 = GetVertex(*it1);
          uint32_t distance = 0;
          int span = 1;
          for (auto it2 = it1; it2 != e2; ++it2) {
            graph::VertexId vertex2 = GetVertex(*next(it2));
            distance += m_transport_catalogue.GetStopsDistance((*it2)->name, (*next(it2))->name);
            m_graph->AddEdge({ vertex1, vertex2, distance / bus_velocity + bus_wait_m_time });
            edges_.push_back({ bus_wait_m_time, *it1, *it2, span, bus });
            ++span;
          }
        }
      }
    }

    graph::VertexId TransportRouter::GetVertex(const Transport::domain::Stop* stop) const {
      auto it = stop_vertices_.find(stop);
      assert(it != stop_vertices_.end());
      return it->second;
    }

  } // namespace Router