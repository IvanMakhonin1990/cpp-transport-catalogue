#pragma once
#include <unordered_map>
#include <string_view>
#include <fstream>
#include <cassert>

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>

namespace Transport {
	namespace Serialization {
		class Serializator {
		public:
			void Serialize(std::ofstream& out);
			void AddTransportCatalogue(const TransportCatalogue& transportCatalogue);
			void SetMapRenderer(const Renderer::MapRenderer& mapRenderer);

			//private:
			//	void Serialize(const TransportCatalogue& transportCatalogue, std::ofstream& out);
			//	void Serialize(const Renderer::MapRenderer& mapRenderer, std::ofstream& out);

		private:
			//TransportCatalogue* m_transportCatalogue;
			//Renderer::MapRenderer* m_mapRenderer;
			transport_catalogue_serialize::DataInfo dataInfo;
		};

		class Deserializator {
		public:
			Deserializator(std::istream& in);
			TransportCatalogue GetTransportCatalogue() const;
			Renderer::MapRenderer GetMapRenderer() const;

			//private:
			//	void Serialize(const TransportCatalogue& transportCatalogue, std::ofstream& out);
			//	void Serialize(const Renderer::MapRenderer& mapRenderer, std::ofstream& out);

		private:
			//TransportCatalogue* m_transportCatalogue;
			//Renderer::MapRenderer* m_mapRenderer;
			transport_catalogue_serialize::DataInfo dataInfo;
		};

	}
	
}