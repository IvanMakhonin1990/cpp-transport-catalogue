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

using namespace Transport::domain;

namespace Transport {
void TransportCatalogue::AddStop(const string &name,
                                 const Coordinates &coordinates) {
  stops.push_back({name, coordinates});
  stopname_to_stop[stops.back().name] = &stops.back();
  stopname_to_buses[stops.back().name];
}

void TransportCatalogue::AddBus(const string &name,
                                const vector<std::string_view> &stops, const bool& is_roundtrip) {
  Bus bus;
  bus.name = name;
  bus.is_roundtrip = is_roundtrip;
  for (auto stop : stops) {
    auto it = stopname_to_stop.find(stop);
    assert(stopname_to_stop.end() != it);
    bus.stops.push_back(it->second);
  }
  set<string_view, less<>> uniq_stops;
  vector<string_view> names(stops.begin(), stops.end());
  for (auto stop = stops.begin(); stop != stops.end() - 1;) {
    uniq_stops.emplace(*stop);
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
  uniq_stops.emplace(stops.back());
  bus.unique_stops = uniq_stops.size();
  assert(bus.distance > 1.0e-6);

  buses.push_back(move(bus));
  
  for (auto stop : buses.back().stops) {
    stopname_to_buses[stop->name].emplace(buses.back().name);
  }
  busname_to_bus[buses.back().name] = &buses.back();
}

const Bus &TransportCatalogue::FindBus(std::string_view name) const {
  static Bus bus;
  bus.curvature = -1.0;
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
  auto bus = FindBus(name);
  if (bus.name.empty()) {
    return "Bus " + string(name) + ": not found";
  }
  assert(!bus.stops.empty() && "Stop's list of bus is empty");

  ss << "Bus " << name << ": " << bus.stops.size() << " stops on route, ";
  ss << bus.unique_stops << " unique stops, " << setprecision(6) << bus.distance << " route length, "
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

bool TransportCatalogue::IsStopWithBuses(std::string_view name) const
{
    return stopname_to_buses.count(name)>0 && stopname_to_buses.at(name).size() > 0;
    
}

std::optional<std::unordered_set<std::string>> TransportCatalogue::GetBuses(std::string_view stop_name) const
{
    std::unordered_set<std::string> result;
    auto it = stopname_to_buses.find(stop_name);
    if (stopname_to_buses.end() != it) {
        result.reserve(it->second.size());
        for (const auto& b : it->second) {
            result.insert(std::string(b));
        }
        return result;
    }
    return std::optional<std::unordered_set<std::string>>();
}

const std::vector<std::string_view> TransportCatalogue::GetBusesNames() const
{
    std::vector<std::string_view> result(busname_to_bus.size());
    std::transform(execution::par, busname_to_bus.begin(), busname_to_bus.end(), result.begin(), [](const auto& arg) {return arg.first; });
    std::sort(execution::par, result.begin(), result.end(), [](std::string_view lhs, std::string_view rhs) { return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); });
    return result;
}

const std::unordered_map<std::string_view, domain::Stop*,
    std::hash<std::string_view>>& TransportCatalogue::GetStops() const
{
    return stopname_to_stop;
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

//void TransportCatalogue::AddStopsDistances(
//    const list<tuple<string, string, double>> &distances) {
//  for (auto [stop_name1, stop_name2, distance] : distances) {
//    SetStopsDistance(stop_name1, stop_name2, distance);
//  }
//}

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
void TransportCatalogue::SetBusWaitTime(int bus_wait_time) {
  m_bus_wait_time = std::move(bus_wait_time);
}

void TransportCatalogue::SetBusVelocity(int bus_velocity) {
  m_bus_velocity = std::move(bus_velocity);
}

int TransportCatalogue::GetBusWaitTime() const {
  return m_bus_wait_time;
}
int TransportCatalogue::GetBusVelocity() const {
  return m_bus_velocity;
}

uint32_t Transport::TransportCatalogue::RouteLength(const Bus* bus) const {
  auto stops = bus->stops;
  
  uint32_t distance = transform_reduce(next(stops.begin()), stops.end(), stops.begin(), static_cast<uint32_t>(0), 
    plus<>(), [this](const Stop* stop1, const Stop* stop2) {
      return GetStopsDistance(stop2->name, stop1->name); 
    }
  );
  if (!bus->is_roundtrip) {
    distance += transform_reduce( next(stops.rbegin()), stops.rend(), stops.rbegin(), static_cast<uint32_t>(0),
      plus<>(),[this](const Stop* stop1, const Stop* stop2) {
        return GetStopsDistance(stop2->name, stop1->name); 
      }
    );
  }
  return distance;
}
const std::unordered_map<std::string_view, domain::Bus*, std::hash<std::string_view>>& Transport::TransportCatalogue::GetAllBuses() const
{
  return busname_to_bus;
}
} // namespace Transport
