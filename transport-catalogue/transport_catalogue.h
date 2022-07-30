#pragma once

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <list>
#include <optional>

#include "geo.h"
#include "domain.h"


namespace Transport {

    struct StopsPairHash {
        size_t  operator()(const std::pair<domain::Stop*, domain::Stop*>& stops_pair) const;
    };

    class TransportCatalogue {
    public:
          void AddStop(const std::string& name,
            const Transport::Geo::Coordinates& coordinates);

        void AddBus(const std::string& name,
            const std::vector<std::string_view>& stops, const bool& is_roundtrip);
        template<typename T>
        void AddStopsDistances( const T& distances) {
            for (auto [stop_name1, stop_name2, distance] : distances) {
                SetStopsDistance(stop_name1, stop_name2, distance);
            }
        }

        const domain::Bus& FindBus(std::string_view name) const;

        const domain::Stop& FindStop(std::string_view name) const;

        std::string GetBusInfo(std::string_view name) const;

        std::string GetStopInfo(std::string_view name) const;

        bool IsStopWithBuses(std::string_view name) const;

        std::optional<std::unordered_set<std::string>> GetBuses(std::string_view stop_name) const;

        const std::vector<std::string_view> GetBusesNames() const;
        
        const std::unordered_map<std::string_view, domain::Stop*,
            std::hash<std::string_view>>& GetStops() const;

        void SetStopsDistance(std::string_view stop_name1,
            std::string_view stop_name2, double distance);

        double GetStopsDistance(std::string_view stop_name1,
            std::string_view stop_name2) const;
    
        void SetBusWaitTime(int bus_wait_time);

        void SetBusVelocity(int bus_velocity);

        int GetBusWaitTime() const;

        int GetBusVelocity() const;

        uint32_t RouteLength(const domain::Bus* bus) const;

        const std::unordered_map<std::string_view, domain::Bus*, std::hash<std::string_view>>& GetAllBuses() const;
        const std::deque<domain::Stop>& GetAllStops() const;
        const std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, double,
            StopsPairHash>& GetDistances() const;
                
    private:
        std::deque<domain::Stop> stops;
        std::unordered_map<std::string_view, domain::Stop*,
            std::hash<std::string_view>>
            stopname_to_stop;
        std::deque<domain::Bus> buses;
        std::unordered_map<std::string_view, domain::Bus*,std::hash<std::string_view>> busname_to_bus;
        std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>,
            std::hash<std::string_view>>
            stopname_to_buses;
        std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, double,
            StopsPairHash>
            stops_ptr_pair;

    private:
      int m_bus_wait_time = 0;
      int m_bus_velocity = 0;
    };
} // namespace Transport
