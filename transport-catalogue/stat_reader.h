#pragma once

#include <string>

#include "input_reader.h"

namespace StatReader {
	enum class RequestType {
		Undefined = 0,
		Bus = 1,
		Stop = 2
	};
	std::pair<RequestType, std::string> ParseInfoRequest(std::string_view request);
}
