#include <algorithm>
#include <sstream>
#include <execution>
#include <cassert>
#include <iomanip>
#include <charconv>
#include <mutex>

#include "transport_catalogue.h"

using namespace std;

using namespace Transport::Geo;

namespace Transport {
void TransportCatalogue::AddStop(const string &name,
                                 const Coordinates &coordinates) {
  stops.push_back({name, coordinates});
  stopname_to_stop[stops.back().name] = &stops.back();
  auto tmp = stopname_to_buses[stops.back().name];
}

void TransportCatalogue::AddBus(const string &name,
                                const vector<std::string_view> &stops) {
  Bus bus;
  bus.name = name;
  for (auto stop : stops) {
    auto it = stopname_to_stop.find(stop);
    assert(stopname_to_stop.end() != it);
    bus.stops.push_back(it->second);
  }

  vector<string_view> names(stops.begin(), stops.end());
  for (auto stop = stops.begin(); stop != stops.end() - 1;) {
    auto stop1 = stopname_to_stop.find(*stop);
    assert(stopname_to_stop.end() != stop1);
    auto stop2 = stopname_to_stop.find(*++stop);
    assert(stopname_to_stop.end() != stop2);

    bus.curvature +=
        ComputeDistance(stop1->second->position, stop2->second->position);
    auto it =
        stops_ptr_pair.find(pair<Stop *, Stop *>(stop1->second, stop2->second));
    if (stops_ptr_pair.end() == it) {
      it = stops_ptr_pair.find(
          pair<Stop *, Stop *>(stop2->second, stop1->second));
    }
    assert(stops_ptr_pair.end() != it);
    bus.distance += it->second;
  }
  
  assert(bus.distance > 1.0e-6);

  buses.push_back(move(bus));
  
  for (auto stop : buses.back().stops) {
    stopname_to_buses[stop->name].emplace(buses.back().name);
  }
  busname_to_bus[buses.back().name] = &buses.back();
}

const Bus &TransportCatalogue::FindBus(std::string_view name) const {
  static Bus bus;
  auto it = busname_to_bus.find(name);
  if (busname_to_bus.end() != it) {
    return *(it->second);
  }
  return bus;
}

const Stop &TransportCatalogue::FindStop(std::string_view name) const {
  static Stop stop;
  auto it = stopname_to_stop.find(name);
  if (stopname_to_stop.end() != it) {
    return *(it->second);
  }
  return stop;
}

string TransportCatalogue::GetBusInfo(std::string_view name) const {
  stringstream ss;
  double dist = 0;
  double curvature = 0;
  auto bus = FindBus(name);
  if (bus.name.empty()) {
    return "Bus " + string(name) + ": not found";
  }
  assert(!bus.stops.empty() && "Stop's list of bus is empty");

  ss << "Bus " << name << ": " << bus.stops.size() << " stops on route, ";
  vector<string> names(bus.stops.size());
  transform(execution::par, bus.stops.begin(), bus.stops.end(), names.begin(),
            [&](Stop *stop1) { return stop1->name; });
  sort(execution::par, names.begin(), names.end());
  ss << distance(names.begin(),
                 unique(execution::par, names.begin(), names.end()));
  ss << " unique stops, " << setprecision(6) << bus.distance << " route length, "
     << bus.distance/bus.curvature << " curvature";

  return ss.str();
}

string TransportCatalogue::GetStopInfo(std::string_view name) const {
  stringstream ss;
  auto it = stopname_to_buses.find(name);
  if (stopname_to_buses.end() == it) {
    ss << "Stop " << name << ": not found";
  } else if (it->second.empty()) {
    ss << "Stop " << name << ": no buses";
  } else {
    ss << "Stop " << name << ": buses";
    for (auto bus : it->second) {
      ss << " " << bus;
    }
  }
  return ss.str();
}

template <class T> inline void hash_combine(std::size_t &seed, const T &v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t
StopsPairHash::operator()(const std::pair<Stop *, Stop *> &stops_pair) const {
  size_t result = 0;

  hash_combine(result, stops_pair.first->name);
  hash_combine(result, stops_pair.first->position.lat);
  hash_combine(result, stops_pair.first->position.lng);

  hash_combine(result, stops_pair.second->name);
  hash_combine(result, stops_pair.second->position.lat);
  hash_combine(result, stops_pair.second->position.lng);

  return result;
}

void TransportCatalogue::AddStopsDistances(
    const vector<tuple<string, string, double>> &distances) {
  for (auto [stop_name1, stop_name2, distance] : distances) {
    SetStopsDistance(stop_name1, stop_name2, distance);
  }
}

void TransportCatalogue::SetStopsDistance(string_view stop_name1,
                                          string_view stop_name2,
                                          double distance) {
  auto stop1 = stopname_to_stop.find(stop_name1);
  assert(stopname_to_stop.end() != stop1);
  auto stop2 = stopname_to_stop.find(stop_name2);
  assert(stopname_to_stop.end() != stop2);
  stops_ptr_pair[pair<Stop *, Stop *>(stop1->second, stop2->second)] = distance;
}

double TransportCatalogue::GetStopsDistance(string_view stop_name1,
                                            string_view stop_name2) const {
  auto stop1 = stopname_to_stop.find(stop_name1);
  assert(stopname_to_stop.end() != stop1);
  auto stop2 = stopname_to_stop.find(stop_name2);
  assert(stopname_to_stop.end() != stop2);
  auto it =
      stops_ptr_pair.find(pair<Stop *, Stop *>(stop1->second, stop2->second));
  if (stops_ptr_pair.end() == it) {
    it =
        stops_ptr_pair.find(pair<Stop *, Stop *>(stop2->second, stop1->second));
  }
  assert(stops_ptr_pair.end() != it);
  return it->second;
}
} // namespace Transport
