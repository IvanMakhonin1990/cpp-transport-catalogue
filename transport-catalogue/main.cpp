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

using namespace Transport;

TransportCatalogue FillTransportCatalogue() {
  int request_count = 0;
  cin >> request_count;
  string line;
  vector<Transport::InputReader::Request> requests;
  requests.reserve(request_count);

  while (request_count >= 0 && getline(cin, line)) {
    --request_count;
    if (!line.empty()) {
      requests.push_back(move(line));
    }
  }

  sort(/*execution::par, */ requests.begin(), requests.end(),
       [](const Transport::InputReader::Request &request1,
          const Transport::InputReader::Request &request2) {
         return request1.request_type < request2.request_type;
       });

  bool distances_processed = false;
  TransportCatalogue transport_catalogue;
  std::vector<std::tuple<std::string, std::string, double>> distances;
  for (auto request : requests) {
    switch (request.request_type) {
    case Transport::InputReader::RequestType::AddStop: {
      auto [name, coordinates] = ParseStop(request, distances);
      transport_catalogue.AddStop(name, coordinates);
      break;
    }
    case Transport::InputReader::RequestType::AddBus: {
      if (!distances_processed) {
        transport_catalogue.AddStopsDistances(distances);
        distances_processed = true;
      }
      auto [name, stops] = ParseBus(request);
      transport_catalogue.AddBus(name, stops);
      break;
    }
    }
  }

  return transport_catalogue;
}

void ExecuteRequestsToTransportCatalogue(
    const TransportCatalogue &transport_catalogue) {
  double request_count = 0;
  string line;
  cin >> request_count;

  while (request_count > 0 && getline(cin, line)) {
    if (line.empty()) {
      continue;
    }
    auto [request_type, name] = StatReader::ParseInfoRequest(line);
    switch (request_type) {
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
}

int main() {

    ExecuteRequestsToTransportCatalogue(FillTransportCatalogue());
    return 0;
}