#pragma once

#include <string>
#include <iostream>

#include "input_reader.h"
#include "transport_catalogue.h"

namespace Transport {
namespace StatReader {
enum class RequestType { Undefined = 0, Bus = 1, Stop = 2 };
std::pair<RequestType, std::string> ParseInfoRequest(std::string_view request);

void ExecuteRequestsToTransportCatalogue(
    const TransportCatalogue &transport_catalogue, std::istream& input_stream = std::cin, std::ostream& output_stream = std::cout);
} // namespace StatReader
} // namespace Transport
