#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include "geo.h"

namespace Transport {
namespace InputReader {
enum class RequestType { AddStop = 0, AddBus = 1 };

struct Request {
  RequestType request_type;
  std::string request_text;

  Request(std::string text);
};

std::pair<std::string, Transport::Geo::Coordinates>
ParseStop(const Request &request,
          std::vector<std::tuple<std::string, std::string, double>> &distances);

std::pair<std::string, std::vector<std::string_view>>
ParseBus(const Request &request);
}

namespace detail {
std::pair<std::string_view, std::string_view> Split(std::string_view line,
                                                    char by);

std::vector<std::string_view> SplitIntoWords(std::string_view text,
                                             const char &symbol = ' ');

std::string_view trim(const std::string_view &str,
                      const std::string_view &whitespace);

template <typename T>
inline T ConvertStringValue(std::string_view string_value) {
  std::istringstream attr_input(string_value.data());
  T result;
  attr_input >> result;
  return result;
}
}

}
