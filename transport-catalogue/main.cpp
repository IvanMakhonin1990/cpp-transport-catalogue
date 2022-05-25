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

int main() {

    Transport::StatReader::ExecuteRequestsToTransportCatalogue(Transport::InputReader::FillTransportCatalogue());
    return 0;
}