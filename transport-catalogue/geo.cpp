#include "geo.h"

namespace Transport {
    namespace Geo {
        bool Coordinates::operator == (const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool Coordinates::operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
        svg::Point SphereProjector::operator()(Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    }
}