#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <iomanip>
#include <fstream>
#include <string_view>


#include "transport_catalogue.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std;

using namespace Transport;

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        Transport::JsonReader::JSONReader json_reader;
        const auto document = json::Load(std::cin);
        json_reader.FillTransportCatalogue(document.GetRoot().AsDict());
        const auto& t = document.GetRoot().AsDict();
        auto filePath = t.at("serialization_settings").AsDict().at("file").AsString();
        ofstream output(filePath, std::ios::binary);
        Transport::Serialization::Serializator s;
        s.AddTransportCatalogue(json_reader.GetTransportCatalogue());
        s.AddMapRenderer(json_reader.ParseRenderSettings(t.at("render_settings")));
        s.AddRouter(Router::TransporRouter(json_reader.GetTransportCatalogue()));
        s.Serialize(output);
        // make base here

    } else if (mode == "process_requests"sv) {
        Transport::JsonReader::JSONReader json_reader;
        const auto document = json::Load(std::cin);
        const auto& t = document.GetRoot().AsDict();
        auto filePath = t.at("serialization_settings").AsDict().at("file").AsString();
        ifstream in_file(filePath, ios::binary);
        assert(in_file);
        Transport::Serialization::Deserializator ds(in_file);

        json_reader.FillTransportCatalogue(ds);
        json::Print(json_reader.FillOutputRequests(t.at("stat_requests"), ds), std::cout);// , json_reader.ParseRenderSettings(t.at("render_settings")));
        //// process requests here

    } else {
        PrintUsage();
        return 1;
    }
    return 0;
}