#include <cassert>
#include <iostream>

#include "stat_reader.h"

using namespace std;

using namespace Transport::detail;

namespace Transport {
namespace StatReader {
std::pair<RequestType, std::string> ParseInfoRequest(string_view request) {
  auto [part1, part2] = Split(request, ' ');
  RequestType request_type = RequestType::Undefined;
  if ("Bus" == part1) {
    request_type = RequestType::Bus;
  }
  if (("Stop" == part1)) {
    request_type = RequestType::Stop;
  }
  return {request_type, string(part2)};
}

void ExecuteRequestsToTransportCatalogue(
    const TransportCatalogue &transport_catalogue, istream &input_stream,
    std::ostream &output_stream) {
  double request_count = 0;
  string line;
  input_stream >> request_count;

  while (request_count > 0 && getline(input_stream, line)) {
    if (line.empty()) {
      continue;
    }
    auto [request_type, name] = StatReader::ParseInfoRequest(line);
    switch (request_type) {
    case StatReader::RequestType::Bus:
      output_stream << transport_catalogue.GetBusInfo(name) << endl;
      break;
    case StatReader::RequestType::Stop:
      output_stream << transport_catalogue.GetStopInfo(name) << endl;
      break;
    default:
      assert(false);
      break;
    }
  }
}
} // namespace StatReader
} // namespace Transport