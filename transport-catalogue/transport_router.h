#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

#include <optional>
#include <variant>
#include <memory>

namespace Router {

  
  class TransportRouter {

  public:

    TransportRouter(const Transport::TransportCatalogue& tc);
    struct WaitBus {
      const Transport::domain::Stop* stop;
      double m_time;
    };

    struct MoveBus {
      const Transport::domain::Bus* m_bus;
      const Transport::domain::Stop* m_from;
      int m_span;
      double m_time;
    };

    using ActionType = std::variant<WaitBus, MoveBus>;

    struct Result {
      std::vector<ActionType> m_atctions;
      double m_total_time;
    };

    std::optional<Result> Route(const Transport::domain::Stop* from, const Transport::domain::Stop* to);

  private:
    void AddBus(const Transport::domain::Bus* bus, double bus_wait_m_time, double bus_velocity);

    graph::VertexId GetVertex(const Transport::domain::Stop* stop) const;

    struct EdgeInfo {
      double m_is_wait;
      const Transport::domain::Stop* m_source;
      const Transport::domain::Stop* m_target;
      int m_span;
      const Transport::domain::Bus* m_bus;
    };

  private:
    const Transport::TransportCatalogue& m_transport_catalogue;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> m_graph;
    std::unique_ptr <graph::Router<double>> m_router;
    std::unordered_map<const Transport::domain::Stop*, graph::VertexId> stop_vertices_;
    std::vector<EdgeInfo> edges_;
  };


} // namespace Router