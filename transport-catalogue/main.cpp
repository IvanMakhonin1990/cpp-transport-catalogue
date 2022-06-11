#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <execution>
#include <cassert>
#include <iomanip>

#include "transport_catalogue.h"
#include "json_reader.h"

using namespace std;

using namespace Transport;

int main() {



    auto document = Transport::json_reader::ReadAndProcessDocument(std::cin);
    json::Print(document, std::cout);
    return 0;
}