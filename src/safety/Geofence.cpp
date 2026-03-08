#include "safety/Geofence.hpp"

namespace nanohawk::safety {

Geofence::Geofence(double xMinM, double xMaxM, double yMinM, double yMaxM)
    : xMinM_(xMinM), xMaxM_(xMaxM), yMinM_(yMinM), yMaxM_(yMaxM) {}

bool Geofence::contains(double xM, double yM) const {
    return xM >= xMinM_ && xM <= xMaxM_ && yM >= yMinM_ && yM <= yMaxM_;
}

} // namespace nanohawk::safety

