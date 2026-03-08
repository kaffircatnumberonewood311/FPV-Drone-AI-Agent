#include "msp/MspClient.hpp"

#include <array>
#include <cstdint>
#include <cstring>  // std::memcpy
#include <string>
#include <vector>

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

namespace nanohawk::msp {

// ─────────────────────────────────────────────────────────────────────────────
//  Impl: platform-specific state
// ─────────────────────────────────────────────────────────────────────────────

struct MspClient::Impl {
#if defined(_WIN32)
    HANDLE hPort{INVALID_HANDLE_VALUE};
#endif
    bool connected{false};
};

// ─────────────────────────────────────────────────────────────────────────────
//  Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

MspClient::MspClient(std::string port, int baud)
    : port_(std::move(port)), baud_(baud), impl_(std::make_unique<Impl>()) {}

MspClient::~MspClient() { disconnect(); }

MspClient::MspClient(MspClient&&) noexcept            = default;
MspClient& MspClient::operator=(MspClient&&) noexcept = default;

// ─────────────────────────────────────────────────────────────────────────────
//  Connection management
// ─────────────────────────────────────────────────────────────────────────────

#if defined(_WIN32)

bool MspClient::connect(std::string& error) {
    if (impl_->connected) { return true; }

    // Use \\.\COMx syntax so port numbers above 9 work correctly.
    const std::string path = "\\\\.\\" + port_;
    HANDLE h = CreateFileA(path.c_str(),
                           GENERIC_READ | GENERIC_WRITE,
                           0,       // exclusive access
                           nullptr, // default security
                           OPEN_EXISTING,
                           0,       // synchronous I/O
                           nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        error = "MspClient: failed to open " + port_
              + " (Win32 error " + std::to_string(GetLastError()) + ")";
        return false;
    }

    // Configure baud rate and 8N1 framing.
    DCB dcb{};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(h, &dcb)) {
        CloseHandle(h);
        error = "MspClient: GetCommState failed on " + port_;
        return false;
    }
    dcb.BaudRate    = static_cast<DWORD>(baud_);
    dcb.ByteSize    = 8;
    dcb.Parity      = NOPARITY;
    dcb.StopBits    = ONESTOPBIT;
    dcb.fBinary     = TRUE;
    dcb.fParity     = FALSE;
    // DTR_CONTROL_ENABLE is needed by some FC boards that use DTR for reset.
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    if (!SetCommState(h, &dcb)) {
        CloseHandle(h);
        error = "MspClient: SetCommState failed on " + port_;
        return false;
    }

    // Read timeout: 1 000 ms total per ReadFile call.
    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout        = 10;   // ms between chars
    to.ReadTotalTimeoutMultiplier = 1;
    to.ReadTotalTimeoutConstant   = 1000; // ms total
    to.WriteTotalTimeoutConstant  = 500;
    SetCommTimeouts(h, &to);

    // Flush any stale bytes left over from Betaflight Configurator etc.
    PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);

    impl_->hPort    = h;
    impl_->connected = true;
    return true;
}

void MspClient::disconnect() {
    if (!impl_->connected) { return; }
    CloseHandle(impl_->hPort);
    impl_->hPort    = INVALID_HANDLE_VALUE;
    impl_->connected = false;
}

bool MspClient::isConnected() const { return impl_->connected; }

// ─────────────────────────────────────────────────────────────────────────────
//  Low-level MSP V1 framing
// ─────────────────────────────────────────────────────────────────────────────

// Build and send a MSP V1 request frame:
//   '$' 'M' '<' <size> <cmd> [payload…] <checksum>
// Checksum = XOR of (size ^ cmd ^ each payload byte).
bool MspClient::sendRequest(uint8_t cmd,
                             const std::vector<uint8_t>& payload,
                             std::string& error) {
    const uint8_t size = static_cast<uint8_t>(payload.size());

    // Pre-allocate the full frame in one buffer (avoids multiple WriteFile calls).
    std::vector<uint8_t> frame;
    frame.reserve(6 + payload.size());
    frame.push_back('$');
    frame.push_back('M');
    frame.push_back('<');
    frame.push_back(size);
    frame.push_back(cmd);
    for (auto b : payload) { frame.push_back(b); }

    uint8_t checksum = size ^ cmd;
    for (auto b : payload) { checksum ^= b; }
    frame.push_back(checksum);

    DWORD written = 0;
    if (!WriteFile(impl_->hPort,
                   frame.data(), static_cast<DWORD>(frame.size()),
                   &written, nullptr)
        || written != static_cast<DWORD>(frame.size())) {
        error = "MspClient: write failed for cmd " + std::to_string(cmd)
              + " (Win32 error " + std::to_string(GetLastError()) + ")";
        return false;
    }
    return true;
}

// Read and parse a MSP V1 response frame using a byte-by-byte state machine.
// Tolerates leading garbage bytes (e.g. stale debug output from Betaflight).
//
// Expected response frame:
//   '$' 'M' '>' <size> <cmd> [payload…] <checksum>
bool MspClient::readResponse(uint8_t               expectedCmd,
                              std::vector<uint8_t>& payload,
                              std::string&          error) {
    enum class St { Header1, Header2, Dir, Size, Cmd, Payload, Checksum };

    St      state      = St::Header1;
    uint8_t size       = 0;
    uint8_t cmd        = 0;
    uint8_t checksum   = 0;
    size_t  payloadIdx = 0;

    // kMaxBytes: maximum number of bytes we'll consume before giving up.
    // Betaflight can emit debug strings before the MSP frame, so be generous.
    constexpr int kMaxBytes = 1024;

    for (int i = 0; i < kMaxBytes; ++i) {
        uint8_t b    = 0;
        DWORD   read = 0;
        if (!ReadFile(impl_->hPort, &b, 1, &read, nullptr) || read != 1) {
            error = "MspClient: response timeout waiting for cmd "
                  + std::to_string(expectedCmd);
            return false;
        }

        switch (state) {
        case St::Header1:
            if (b == '$') { state = St::Header2; }
            break;

        case St::Header2:
            state = (b == 'M') ? St::Dir : St::Header1;
            break;

        case St::Dir:
            if (b == '>') {
                state = St::Size;
            } else if (b == '!') {
                // FC returned an MSP error response for this command.
                error = "MspClient: FC error response for cmd "
                      + std::to_string(expectedCmd);
                return false;
            } else {
                state = St::Header1;
            }
            break;

        case St::Size:
            size     = b;
            checksum = b;    // checksum starts with size
            state    = St::Cmd;
            break;

        case St::Cmd:
            cmd       = b;
            checksum ^= b;
            if (size == 0) {
                state = St::Checksum;
            } else {
                payload.resize(size);
                payloadIdx = 0;
                state      = St::Payload;
            }
            break;

        case St::Payload:
            payload[payloadIdx] = b;
            checksum ^= b;
            if (++payloadIdx == size) { state = St::Checksum; }
            break;

        case St::Checksum:
            if (b != checksum) {
                error = "MspClient: checksum mismatch (got 0x"
                      + std::to_string(b) + ", expected 0x"
                      + std::to_string(checksum) + ") for cmd "
                      + std::to_string(expectedCmd);
                return false;
            }
            if (cmd != expectedCmd) {
                // Got a different command's response — skip and keep parsing.
                // This can happen when Betaflight sends unsolicited frames.
                state      = St::Header1;
                size       = 0;
                checksum   = 0;
                payloadIdx = 0;
                payload.clear();
                break;
            }
            return true;
        }
    }

    error = "MspClient: exceeded " + std::to_string(kMaxBytes)
          + " bytes waiting for cmd " + std::to_string(expectedCmd);
    return false;
}

#else  // ── Non-Windows stubs ─────────────────────────────────────────────────

bool MspClient::connect(std::string& error) {
    error = "MspClient: not implemented on this platform";
    return false;
}
void MspClient::disconnect() {}
bool MspClient::isConnected() const { return false; }

bool MspClient::sendRequest(uint8_t, const std::vector<uint8_t>&, std::string& error) {
    error = "not implemented"; return false;
}
bool MspClient::readResponse(uint8_t, std::vector<uint8_t>&, std::string& error) {
    error = "not implemented"; return false;
}

#endif // _WIN32

// ─────────────────────────────────────────────────────────────────────────────
//  transaction() — flush Rx, send request, wait for response
// ─────────────────────────────────────────────────────────────────────────────

bool MspClient::transaction(uint8_t                     cmd,
                             const std::vector<uint8_t>& req,
                             std::vector<uint8_t>&       resp,
                             std::string&                error) {
    if (!impl_->connected) {
        error = "MspClient: not connected";
        return false;
    }
#if defined(_WIN32)
    PurgeComm(impl_->hPort, PURGE_RXCLEAR);
#endif
    if (!sendRequest(cmd, req, error)) { return false; }
    resp.clear();
    return readResponse(cmd, resp, error);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Identification
// ─────────────────────────────────────────────────────────────────────────────

// MSP_FC_VARIANT (cmd 2) — response: 4 ASCII chars, e.g. "BTFL"
bool MspClient::identify(std::string& fcVariant, std::string& error) {
    std::vector<uint8_t> resp;
    if (!transaction(static_cast<uint8_t>(Cmd::FcVariant), {}, resp, error)) {
        return false;
    }
    if (resp.size() < 4) {
        error = "MspClient: MSP_FC_VARIANT response too short ("
              + std::to_string(resp.size()) + " bytes)";
        return false;
    }
    fcVariant.assign(reinterpret_cast<const char*>(resp.data()), 4);
    return true;
}

// MSP_FC_VERSION (cmd 3) — response: major, minor, patch (3 × uint8)
bool MspClient::version(uint8_t& major, uint8_t& minor,
                         uint8_t& patch, std::string& error) {
    std::vector<uint8_t> resp;
    if (!transaction(static_cast<uint8_t>(Cmd::FcVersion), {}, resp, error)) {
        return false;
    }
    if (resp.size() < 3) {
        error = "MspClient: MSP_FC_VERSION response too short";
        return false;
    }
    major = resp[0];
    minor = resp[1];
    patch = resp[2];
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Telemetry
// ─────────────────────────────────────────────────────────────────────────────

// MSP_ATTITUDE (cmd 108)
// Payload: int16 roll (decidegrees), int16 pitch (decidegrees), int16 yaw (degrees)
bool MspClient::readAttitude(Attitude& out, std::string& error) {
    std::vector<uint8_t> resp;
    if (!transaction(static_cast<uint8_t>(Cmd::Attitude), {}, resp, error)) {
        return false;
    }
    if (resp.size() < 6) {
        error = "MspClient: MSP_ATTITUDE response too short ("
              + std::to_string(resp.size()) + " bytes, need 6)";
        return false;
    }
    int16_t raw[3];
    std::memcpy(raw, resp.data(), 6);
    out.rollDeg  = raw[0] / 10.0f;  // decidegrees → degrees
    out.pitchDeg = raw[1] / 10.0f;
    out.yawDeg   = static_cast<float>(raw[2]);  // already in degrees
    return true;
}

// MSP_ANALOG (cmd 110)
// Payload: uint8 vbat (0.1 V), uint16 mAh, uint16 rssi, int16 amperage (0.01 A)
bool MspClient::readAnalog(Analog& out, std::string& error) {
    std::vector<uint8_t> resp;
    if (!transaction(static_cast<uint8_t>(Cmd::Analog), {}, resp, error)) {
        return false;
    }
    if (resp.size() < 7) {
        error = "MspClient: MSP_ANALOG response too short ("
              + std::to_string(resp.size()) + " bytes, need 7)";
        return false;
    }
    out.batteryVolts = resp[0] / 10.0f;

    uint16_t mAh  = 0;
    uint16_t rssi = 0;
    int16_t  amp  = 0;
    std::memcpy(&mAh,  resp.data() + 1, 2);
    std::memcpy(&rssi, resp.data() + 3, 2);
    std::memcpy(&amp,  resp.data() + 5, 2);

    out.mAhDrawn = mAh;
    out.rssi     = rssi;
    out.amperage = amp / 100.0f;  // centl-amps → amps
    return true;
}

// MSP_RC (cmd 105)
// Payload: up to 18 × uint16 channel values (µs, 1000–2000)
bool MspClient::readRc(RcChannels& out, std::string& error) {
    std::vector<uint8_t> resp;
    if (!transaction(static_cast<uint8_t>(Cmd::Rc), {}, resp, error)) {
        return false;
    }
    if (resp.size() < 16) {
        error = "MspClient: MSP_RC response too short ("
              + std::to_string(resp.size()) + " bytes, need 16)";
        return false;
    }
    for (int i = 0; i < 8; ++i) {
        std::memcpy(&out.ch[i], resp.data() + i * 2, 2);
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RC override
// ─────────────────────────────────────────────────────────────────────────────

// MSP_SET_RAW_RC (cmd 200)
// Payload: 8 × uint16 channel values (µs, 1000–2000), little-endian.
// FC returns an empty ACK frame (size=0) on success.
bool MspClient::setRawRc(const RcChannels& channels, std::string& error) {
    std::vector<uint8_t> payload(16);
    for (int i = 0; i < 8; ++i) {
        payload[i * 2]     = static_cast<uint8_t>(channels.ch[i] & 0xFF);
        payload[i * 2 + 1] = static_cast<uint8_t>(channels.ch[i] >> 8);
    }
    std::vector<uint8_t> resp;
    return transaction(static_cast<uint8_t>(Cmd::SetRawRc), payload, resp, error);
}

// Arm: throttle=1000 (minimum), arm switch=1900 (high), everything else centred.
bool MspClient::arm(std::string& error) {
    RcChannels ch;
    ch.ch[0] = 1500;  // roll  — centre
    ch.ch[1] = 1500;  // pitch — centre
    ch.ch[2] = 1000;  // throttle — minimum (required before arming)
    ch.ch[3] = 1500;  // yaw   — centre
    ch.ch[4] = 1900;  // arm switch — high
    ch.ch[5] = 1000;
    ch.ch[6] = 1000;
    ch.ch[7] = 1000;
    return setRawRc(ch, error);
}

// Disarm: arm switch=1000 (low), throttle minimum, everything else centred.
bool MspClient::disarm(std::string& error) {
    RcChannels ch;
    ch.ch[0] = 1500;
    ch.ch[1] = 1500;
    ch.ch[2] = 1000;
    ch.ch[3] = 1500;
    ch.ch[4] = 1000;  // arm switch — low
    ch.ch[5] = 1000;
    ch.ch[6] = 1000;
    ch.ch[7] = 1000;
    return setRawRc(ch, error);
}

} // namespace nanohawk::msp
