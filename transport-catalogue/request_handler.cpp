#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const Transport::TransportCatalogue& db, const Transport::Renderer::MapRenderer& renderer):db_(db), renderer_(renderer)
{
}

std::optional<Transport::domain::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const
{
    auto bus = db_.FindBus(bus_name);
    if (-1.0 != bus.curvature) {
        Transport::domain::BusStat bus_stat{ bus.distance/bus.curvature , bus.distance, static_cast<int>(bus.stops.size()), static_cast<int>(bus.unique_stops) };
        return std::optional<Transport::domain::BusStat>(std::move(bus_stat));
    }
    return std::optional<Transport::domain::BusStat>();
}

const std::optional<std::unordered_set<std::string>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
    return db_.GetBuses(stop_name);
}

svg::Document RequestHandler::RenderMap() const
{
    return renderer_.RenderBuses(db_);
}
