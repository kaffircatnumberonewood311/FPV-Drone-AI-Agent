#pragma once

#include "planning/MissionTypes.hpp"

namespace nanohawk::telemetry {

class VehicleStateStore {
public:
    void update(const planning::VehicleSnapshot& snapshot);
    [[nodiscard]] planning::VehicleSnapshot current() const;

private:
    planning::VehicleSnapshot snapshot_{};
};

} // namespace nanohawk::telemetry

