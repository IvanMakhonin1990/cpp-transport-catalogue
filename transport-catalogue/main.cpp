#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <execution>
#include <cassert>

#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

using namespace std;

int main() {

    int request_count = 0;
    cin >> request_count;
    string line;
    vector<Request> requests;
    requests.reserve(request_count);

    while (request_count >= 0 && getline(cin, line)) {
        --request_count;
        if (!line.empty()) {
            requests.push_back(move(line));
        }
    }
    //assert(requests.size() == request_count);

    sort(/*execution::par, */requests.begin(), requests.end(),
        [](const Request& request1, const Request& request2) {
            return request1.request_type < request2.request_type;
        });

    TransportCatalogue transport_catalogue;
    std::vector<std::tuple<std::string, std::string, double>> distances;
    for (auto request : requests) {
        switch (request.request_type) {
        case RequestType::AddStop:
        {
            auto [name, coordinates] = ParseStop(request, distances);
            transport_catalogue.AddStop(name, coordinates);
            break;
        }
        case RequestType::AddBus:
        {
            auto [name, stops] = ParseBus(request);
            transport_catalogue.AddBus(name, stops);
            break;
        }
        }
    }

    transport_catalogue.AddStopsDistances(distances);

    request_count = 0;
    cin >> request_count;

    while (request_count > 0 && getline(cin, line)) {
        if (line.empty()) {
            continue;
        }
        auto [request_type, name] = StatReader::ParseInfoRequest(line);
        switch (request_type)
        {
        case StatReader::RequestType::Bus:
            cout << transport_catalogue.GetBusInfo(name) << endl;
            break;
        case StatReader::RequestType::Stop:
            cout << transport_catalogue.GetStopInfo(name) << endl;
            break;
        default:
            assert(false);
            break;
        }
    }
    return 0;
}