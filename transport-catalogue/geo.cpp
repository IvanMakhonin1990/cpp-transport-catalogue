#include "geo.h"

namespace Transport {
namespace Geo {
bool Coordinates::operator == (const Coordinates &other) const { 
    return lat == other.lat && lng == other.lng; 
}
bool Coordinates::operator!=(const Coordinates &other) const {
  return !(*this == other);
}
}
}