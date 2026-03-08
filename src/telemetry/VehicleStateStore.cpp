#include "telemetry/VehicleStateStore.hpp"

namespace nanohawk::telemetry {

void VehicleStateStore::update(const planning::VehicleSnapshot& snapshot) {
    snapshot_ = snapshot;
}

planning::VehicleSnapshot VehicleStateStore::current() const {
    return snapshot_;
}

} // namespace nanohawk::telemetry

