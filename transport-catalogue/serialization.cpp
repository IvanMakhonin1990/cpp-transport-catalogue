
#include <transport_catalogue.pb.h>
#include "serialization.h"
#include "domain.h"


namespace Transport {
	namespace Serialization {
		void Serialize(const TransportCatalogue& transportCatalogue, std::ofstream& out) {
			transport_catalogue_serialize::TransportCatalogueInfo transportCatalogueInfo;

			//Stops
			std::unordered_map<std::string_view, int> stop_to_id;
			auto& stops = transportCatalogue.GetAllStops();
			int i = 0;
			for (auto& stop : stops)
			{
				auto stopPtr = transportCatalogueInfo.add_stops();
				stopPtr->set_id(i);
				stop_to_id.emplace(stop.name, i);
				stopPtr->set_name(stop.name);
				stopPtr->mutable_position()->set_lat(stop.position.lat);
				stopPtr->mutable_position()->set_lng(stop.position.lng);
				////
				++i;
			}

			auto& distances = transportCatalogue.GetDistances();
			for (auto& distance : distances) {
				auto distancePtr = transportCatalogueInfo.add_distances();
				auto it1 = stop_to_id.find(distance.first.first->name);
				assert(stop_to_id.end() != it1);
				distancePtr->set_stop1id(it1->second);
				auto it2 = stop_to_id.find(distance.first.second->name);
				assert(stop_to_id.end() != it2);
				distancePtr->set_stop2id(it2->second);
				distancePtr->set_distance(distance.second);
			}

			auto& buses = transportCatalogue.GetAllBuses();
			for (auto& bus : buses) {
				auto busPtr = transportCatalogueInfo.add_buses();
				busPtr->set_name(bus.second->name);
				busPtr->set_is_roundtrip(bus.second->is_roundtrip);
				for (auto& stop : bus.second->stops) {
					auto it = stop_to_id.find(stop->name);
					assert(stop_to_id.end() != it);
					busPtr->add_stops(it->second);
				}
			}
			assert(transportCatalogueInfo.SerializePartialToOstream(&out));
		}


		Transport::TransportCatalogue DeserializeTransportCatalogue(std::istream& in) {
			transport_catalogue_serialize::TransportCatalogueInfo transportCatalogueInfo;
			assert(transportCatalogueInfo.ParseFromIstream(&in));
			
			Transport::TransportCatalogue transportCatalogue;

			std::list<std::tuple<std::string, std::string, double>> distances;

			std::unordered_map<int, std::string_view> id_to_name;
			for (auto& item : transportCatalogueInfo.stops()) {
				Geo::Coordinates coordinates;
				coordinates.lat = std::move(item.position().lat());
				coordinates.lng = std::move(item.position().lng());
				transportCatalogue.AddStop(item.name(), coordinates);
				id_to_name.emplace(item.id(), item.name());
			}
			for (auto& item : transportCatalogueInfo.distances()) {
				auto it1 = id_to_name.find(item.stop1id());
				assert(id_to_name.end() != it1);
				auto it2 = id_to_name.find(item.stop2id());
				assert(id_to_name.end() != it2);
				transportCatalogue.SetStopsDistance(it1->second, it2->second, item.distance());
			}
			for (auto& item : transportCatalogueInfo.buses()) {
				std::vector<std::string_view> stops_view;
				stops_view.reserve(item.stops().size());
				for (auto& stops_item : item.stops()) {
					auto it1 = id_to_name.find(stops_item);
					assert(id_to_name.end() != it1);
					stops_view.push_back(it1->second);
				}
				transportCatalogue.AddBus(item.name(), stops_view, item.is_roundtrip());
			}
			return transportCatalogue;
		}
	}
}



	