#include <cassert>
#include <charconv>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <system_error>

#include "input_reader.h"

using namespace std;

Request::Request(std::string text) { 
    auto [p1, p2] = Split(text, ' ');
    if ("Stop" == p1) {
    request_type = RequestType::AddStop;
    } else if ("Bus" == p1) {
      request_type = RequestType::AddBus;
    } else {
      assert(false);
    }
    request_text = std::move(text);
}

pair<string_view, string_view> Split(string_view line, char by) {
  size_t pos = line.find(by);
  string_view left = line.substr(0, pos);

  if (pos < line.size() && pos + 1 < line.size()) {
    return {left, line.substr(pos + 1)};
  } else {
    return {left, string_view()};
  }
}

pair<string, Coordinates>
ParseStop(const Request &request,
          std::vector<std::tuple<std::string, std::string, double>>& distances) {
  assert(RequestType::AddStop == request.request_type);
  auto [part1, part2] = Split(request.request_text, ':');
  part1 = part1.substr(5);
  auto ws = SplitIntoWords(part2, ',');
  assert(2 <= ws.size());
  for (size_t i = 2; i < ws.size(); ++i) {
    auto pos = ws[i].find(' ');
    assert(string::npos != pos);
    auto dist = ws[i].substr(0, pos - 1);
    pos = ws[i].find(' ', pos + 1);
    assert(string::npos != pos);
    auto dist1 = ws[i].substr(pos + 1);
    distances.push_back(tuple<string, string, double>(string(part1), dist1, ConvertStringValue<double>(dist)));
  }
  return {
      string(part1),
      {ConvertStringValue<double>(ws[0]), ConvertStringValue<double>(ws[1])}};
}

pair<string, vector<string_view>> ParseBus(const Request& request) {
  assert(RequestType::AddBus == request.request_type);
  auto [part1, part2] = Split(request.request_text, ':');
  part1 = part1.substr(4);

  vector<string_view> stops_names;

  auto begin = part2.begin();
  auto it = part2.begin();
  size_t count = 0;
  bool is_circle_route = false;
  bool is_serial_route = false;
  while (part2.end() != it) {
      if (!is_circle_route && *it == '-') {
          is_circle_route = true;
      }
      if (!is_serial_route && *it == '>') {
          is_serial_route = true;
      }
      assert(!(is_serial_route && is_circle_route));

      if (*it == '-' || *it == '>') {
          if (count > 0) {
              stops_names.push_back(trim(part2.substr(distance(part2.begin(), begin), count), " "));
          }
          count = 0;
          begin = it + 1;

      }
      else {
          ++count;
          if (it + 1 == part2.end()) {
              stops_names.push_back(trim(part2.substr(distance(part2.begin(), begin), count), " "));
          }
      }
      ++it;
  }
  if (is_circle_route) {
      stops_names.insert(stops_names.begin(), stops_names.rbegin(), stops_names.rend() - 1);
  }
  return {string(part1), stops_names};
}

vector<string_view> SplitIntoWords(string_view text, const char& symbol) {
  vector<string_view> words;

  auto begin = text.begin();
  auto it = text.begin();
  size_t count = 0;
  while (text.end() != it) {
    if (symbol == *it) {
      if (count > 0) {
        words.push_back(
            trim(text.substr(distance(text.begin(), begin), count), " "));
      }
      count = 0;
      begin = it + 1;

    } else {
      ++count;
      if (it + 1 == text.end()) {
        words.push_back(
            trim(text.substr(distance(text.begin(), begin), count), " "));
      }
    }
    ++it;
  }
  return words;
}

std::string_view trim(const std::string_view &str, const std::string_view &whitespace) {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos)
    return ""; // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}
