#pragma once

namespace nanohawk::safety {

class Geofence {
public:
    Geofence(double xMinM, double xMaxM, double yMinM, double yMaxM);

    [[nodiscard]] bool contains(double xM, double yM) const;

private:
    double xMinM_;
    double xMaxM_;
    double yMinM_;
    double yMaxM_;
};

} // namespace nanohawk::safety

