#pragma once
#include <unordered_map>
#include <string_view>
#include <fstream>
#include <cassert>

//#include "transport_catalogue.h"
#include "map_renderer.h"
//#include "graph.h"
//#include "router.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>


namespace Transport {
	namespace Serialization {
		class Serializator {
		public:
			void Serialize(std::ofstream& out);
			void AddTransportCatalogue(const TransportCatalogue& transportCatalogue);
			void AddMapRenderer(const Renderer::MapRenderer& mapRenderer);
			void AddRouter(const Router::TransporRouter& router);

			//private:
			//	void Serialize(const TransportCatalogue& transportCatalogue, std::ofstream& out);
			//	void Serialize(const Renderer::MapRenderer& mapRenderer, std::ofstream& out);

		private:
			//TransportCatalogue* m_transportCatalogue;
			//Renderer::MapRenderer* m_mapRenderer;
			transport_catalogue_serialize::DataInfo dataInfo;
			std::unordered_map<std::string_view, int> stop_to_id;
			std::unordered_map<std::string_view, int> bus_to_id;
		};

		class Deserializator {
		public:
			Deserializator(std::istream& in);
			TransportCatalogue GetTransportCatalogue() const;
			Renderer::MapRenderer GetMapRenderer() const;
			std::unique_ptr<Router::TransporRouter > GetRouter(const TransportCatalogue& transportCatalogue);

			//private:
			//	void Serialize(const TransportCatalogue& transportCatalogue, std::ofstream& out);
			//	void Serialize(const Renderer::MapRenderer& mapRenderer, std::ofstream& out);

		private:
			//TransportCatalogue* m_transportCatalogue;
			//Renderer::MapRenderer* m_mapRenderer;
			transport_catalogue_serialize::DataInfo dataInfo;
			mutable std::unordered_map<int, std::string> id_to_name;
			mutable std::unordered_map<int, std::string> bus_id_to_name;
	    };

	}
	
}