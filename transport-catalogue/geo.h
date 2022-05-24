#pragma once

#include <cmath>

namespace Transport {
namespace Geo {

static const double earth_radius = 6371000.0;

struct Coordinates {
  double lat;
  double lng;
  bool operator==(const Coordinates &other) const;
  bool operator!=(const Coordinates &other) const;
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
  using namespace std;
  if (from == to) {
    return 0;
  }
  static const double dr = 3.1415926535 / 180.;
  return acos(sin(from.lat * dr) * sin(to.lat * dr) +
              cos(from.lat * dr) * cos(to.lat * dr) *
                  cos(abs(from.lng - to.lng) * dr)) *
         earth_radius;
}
}
}
