#include "telemetry/MavlinkTransport.hpp"

#include <chrono>
#include <future>
#include <regex>
#include <string>
#include <thread>

#ifdef NANOHAWK_WITH_MAVSDK
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#endif

namespace nanohawk::telemetry {

namespace {

std::string normalizeEndpoint(const std::string& endpoint) {
    if (endpoint.rfind("udp://", 0) == 0) {
        return "udpin://" + endpoint.substr(6);
    }
    if (endpoint.rfind("serial://", 0) == 0) {
        return endpoint;
    }
    if (endpoint.rfind("COM", 0) == 0 || endpoint.rfind("com", 0) == 0) {
        return "serial://" + endpoint + ":57600";
    }
    return endpoint;
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

} // namespace

struct MavlinkTransport::Impl {
#ifdef NANOHAWK_WITH_MAVSDK
    mavsdk::Mavsdk mavsdk{};
    std::shared_ptr<mavsdk::System> system;
    std::unique_ptr<mavsdk::Action> action;
    std::unique_ptr<mavsdk::Offboard> offboard;
#endif
    bool connected{false};
};

MavlinkTransport::MavlinkTransport(std::string endpoint)
    : endpoint_(std::move(endpoint)), impl_(std::make_unique<Impl>()) {}

MavlinkTransport::~MavlinkTransport() = default;
MavlinkTransport::MavlinkTransport(MavlinkTransport&&) noexcept = default;
MavlinkTransport& MavlinkTransport::operator=(MavlinkTransport&&) noexcept = default;

bool MavlinkTransport::connect(std::string& error) {
    if (endpoint_.empty()) {
        error = "empty MAVLink endpoint";
        return false;
    }

#ifdef NANOHAWK_WITH_MAVSDK
    const std::string connectionUrl = normalizeEndpoint(endpoint_);
    const auto addResult = impl_->mavsdk.add_any_connection(connectionUrl);
    if (addResult != mavsdk::ConnectionResult::Success) {
        error = "MAVSDK add_any_connection failed for " + connectionUrl;
        return false;
    }

    std::promise<std::shared_ptr<mavsdk::System>> promise;
    auto future = promise.get_future();

    impl_->mavsdk.subscribe_on_new_system([this, &promise]() mutable {
        auto systems = impl_->mavsdk.systems();
        for (const auto& system : systems) {
            if (system && system->is_connected()) {
                try {
                    promise.set_value(system);
                } catch (...) {
                }
                break;
            }
        }
    });

    if (future.wait_for(std::chrono::seconds(10)) != std::future_status::ready) {
        error = "timeout waiting for MAVSDK system discovery";
        return false;
    }

    impl_->system = future.get();
    if (!impl_->system) {
        error = "discovered MAVSDK system is null";
        return false;
    }

    impl_->action = std::make_unique<mavsdk::Action>(impl_->system);
    impl_->offboard = std::make_unique<mavsdk::Offboard>(impl_->system);
    impl_->connected = true;
    return true;
#else
    error = "MAVSDK support not enabled at build time";
    return false;
#endif
}

bool MavlinkTransport::sendGuidedCommand(const std::string& command, std::string& error) const {
    if (command.empty()) {
        error = "empty command";
        return false;
    }

#ifdef NANOHAWK_WITH_MAVSDK
    if (!impl_->connected || !impl_->system || !impl_->action || !impl_->offboard) {
        error = "transport is not connected";
        return false;
    }

    if (startsWith(command, "takeoff")) {
        const auto result = impl_->action->arm();
        if (result != mavsdk::Action::Result::Success) {
            error = "arm failed";
            return false;
        }
        const auto takeoff = impl_->action->takeoff();
        if (takeoff != mavsdk::Action::Result::Success) {
            error = "takeoff failed";
            return false;
        }
        return true;
    }

    if (startsWith(command, "land")) {
        const auto result = impl_->action->land();
        if (result != mavsdk::Action::Result::Success) {
            error = "land failed";
            return false;
        }
        return true;
    }

    if (startsWith(command, "move_body:")) {
        std::smatch match;
        const std::regex parse(R"(^move_body:([\-0-9\.]+),([\-0-9\.]+),([\-0-9\.]+)$)");
        if (!std::regex_match(command, match, parse)) {
            error = "move_body format must be move_body:forward,right,yaw_deg_s";
            return false;
        }

        mavsdk::Offboard::VelocityBodyYawspeed body{};
        body.forward_m_s = std::stof(match[1].str());
        body.right_m_s = std::stof(match[2].str());
        body.down_m_s = 0.0f;
        body.yawspeed_deg_s = std::stof(match[3].str());

        const auto setpoint = impl_->offboard->set_velocity_body(body);
        if (setpoint != mavsdk::Offboard::Result::Success) {
            error = "failed to set offboard velocity setpoint";
            return false;
        }

        const auto start = impl_->offboard->start();
        if (start != mavsdk::Offboard::Result::Success) {
            error = "failed to start offboard mode";
            return false;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        mavsdk::Offboard::VelocityBodyYawspeed zero{};
        zero.forward_m_s = 0.0f;
        zero.right_m_s = 0.0f;
        zero.down_m_s = 0.0f;
        zero.yawspeed_deg_s = 0.0f;
        (void)impl_->offboard->set_velocity_body(zero);
        (void)impl_->offboard->stop();
        return true;
    }

    if (startsWith(command, "hover")) {
        mavsdk::Offboard::VelocityBodyYawspeed zero{};
        zero.forward_m_s = 0.0f;
        zero.right_m_s = 0.0f;
        zero.down_m_s = 0.0f;
        zero.yawspeed_deg_s = 0.0f;
        if (impl_->offboard->set_velocity_body(zero) != mavsdk::Offboard::Result::Success) {
            error = "failed to set hover setpoint";
            return false;
        }
        return true;
    }

    error = "unsupported guided command: " + command;
    return false;
#else
    error = "MAVSDK support not enabled at build time";
    return false;
#endif
}

} // namespace nanohawk::telemetry

