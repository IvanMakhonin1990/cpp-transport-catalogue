#pragma once

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <set>

#include "geo.h"


namespace Transport {
struct Stop {
  std::string name;
  Transport::Geo::Coordinates position;
};

struct StopsPairHash {
  size_t operator()(const std::pair<Stop *, Stop *> &stops_pair) const;
};

struct Bus {
  std::string name;
  std::vector<Stop *> stops;
};

class TransportCatalogue {
public:
  void AddStop(const std::string &name,
               const Transport::Geo::Coordinates &coordinates);

  void AddBus(const std::string &name,
              const std::vector<std::string_view> &stops);

  void AddStopsDistances(
      const std::vector<std::tuple<std::string, std::string, double>>
          &distances);

  const Bus &FindBus(std::string_view name) const;

  const Stop &FindStop(std::string_view name) const;

  std::string GetBusInfo(std::string_view name) const;

  std::string GetStopInfo(std::string_view name) const;

  void SetStopsDistance(std::string_view stop_name1,
                        std::string_view stop_name2, double distance);

  double GetStopsDistance(std::string_view stop_name1,
                          std::string_view stop_name2) const;

private:
  std::deque<Stop> stops;
  std::unordered_map<std::string_view, Stop *, std::hash<std::string_view>>
      stopname_to_stop;
  std::deque<Bus> buses;
  std::unordered_map<std::string_view, Bus *, std::hash<std::string_view>>
      busname_to_bus;
  std::unordered_map<std::string_view, std::set<std::string_view, std::less<>>,
                     std::hash<std::string_view>>
      stopname_to_buses;
  std::unordered_map<std::pair<Stop *, Stop *>, double, StopsPairHash>
      stops_ptr_pair;
};
} // namespace Transport
