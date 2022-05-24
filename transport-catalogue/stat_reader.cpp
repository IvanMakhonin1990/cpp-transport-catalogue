#include <cassert>

#include "stat_reader.h"

using namespace std;

using namespace Transport::detail;

namespace StatReader {
	std::pair<RequestType, std::string> ParseInfoRequest(string_view request) {
		auto [part1, part2] = Split(request, ' ');
		RequestType request_type = RequestType::Undefined;
		if ("Bus" == part1) {
			request_type = RequestType::Bus;
		}
		if(("Stop" == part1)) {
			request_type = RequestType::Stop;
		}
		return { request_type, string(part2) };
	}
}