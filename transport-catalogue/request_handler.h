#pragma once

#include <unordered_set>
#include <optional>

#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"


class RequestHandler {
public:
    RequestHandler(const Transport::TransportCatalogue& db, const Transport::renderer::MapRenderer& renderer);

    std::optional<Transport::domain::BusStat> GetBusStat(const std::string_view& bus_name) const;

    const std::optional<std::unordered_set<std::string>> GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

private:
    const Transport::TransportCatalogue& db_;
    const Transport::renderer::MapRenderer& renderer_;
};
