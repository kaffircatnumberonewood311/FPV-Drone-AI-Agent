# nanohawk-agent
![img_1.png](img_1.png)

**AI-powered autonomous control agent for the EMAX Nanohawk 1S FPV Drone.**

A modular C++20 desktop application that uses a local LLM to interpret natural-language commands and fly the Nanohawk via the Betaflight MSP serial protocol. No FPV goggles required. Live camera feed streams to your desktop. You type a prompt; the agent executes it on the drone.

EMAX Nanohawk 1S -- https://emax-usa.com/products/nanohawk-1s-ultralight-brushless-fpv-drone

---

## Hardware

| Component | Spec |
|---|---|
| Frame | 65 mm brushless micro FPV |
| Flight Controller | STM32 running **Betaflight 4.2.0** |
| USB Identity | VID `0483` / PID `5740` (STM32 USB CDC) |
| Serial Protocol | **MSP V1** at 115200 baud |
| Motors | 0802 brushless, 4-in-1 ESC |
| FPV Camera | Analog + 5.8 GHz VTX (desktop capture via USB dongle) |
| Connection | USB-C to laptop (COM3) -- confirmed working |


# Workflow Diagram
<img width="2547" height="1422" alt="nanohawk_nl_agent_workflow" src="https://github.com/user-attachments/assets/43b41436-651b-4aa8-ac1d-ae5ac9cc0a6d" />


## Device Detection

`DeviceWatcher` automatically finds the drone at startup using two methods:

### 1. WiFi UDP Broadcast (Companion MCU)

Sends `NHAWK_DISCOVER` (14 bytes) as a UDP broadcast on port 14560.
If a companion MCU replies with `NHAWK_FOUND` (10 bytes), its IP is used as the endpoint.

### 2. USB Serial (Betaflight FC -- Active)

Scans the Windows registry for known USB device VID/PID combinations:

| VID | PID | Device | Baud |
|---|---|---|---|
| `0483` | `5740` | **STM32 Betaflight FC** <- EMAX Nanohawk | 115200 |
| `2E8A` | `000A` | RP2040 companion MCU | 500000 |
| `10C4` | `EA60` | CP210x USB-Serial | 115200 |
| `1A86` | `7523` | CH340 USB-Serial | 115200 |
| `0403` | `6001` | FTDI FT232R | 115200 |

If no VID/PID match is found, falls back to sending an `NHAWK?` handshake on every open COM port.


## Safety Architecture

| Layer | Role | Failsafe |
|---|---|---|
| **LLM** | Natural-language -> strict JSON | Returns idle JSON if unavailable |
| **JsonPlanParser** | Schema validation | Rejects malformed mission plans |
| **SafetyEngine** | Hard-limit veto | Blocks execution on any breach |
| **AbortController** | Operator emergency stop | Disarms immediately |
| **MspClient.disarm()** | Final hardware failsafe | ch[4]=1000, throttle=1000 |
| **Manual TX** | Physical radio override | Always available; takes precedence |

**Key principle:** The LLM never directly controls motors. It outputs JSON. JSON goes through safety validation. Only validated, operator-authorized commands reach the MSP serial layer.

---

## Build and Run

### Prerequisites

- Windows 10+ (Linux/macOS stubs compile but lack Windows serial/WiFi backends)
- CMake 3.26+, C++20 compiler (MinGW-w64 or MSVC)
- Optional for GUI: Qt 6.5+, OpenCV 4.8+, libcurl 7.85+


## Troubleshooting

**Drone not detected**
- Check Device Manager for `STM32 Virtual COM Port`.
- Use a data-capable USB cable (not charge-only).
- If COM port differs, update `endpoints.yaml` -> `serial_port`.

**RC override has no effect**
- Open Betaflight Configurator -> Receiver -> set type to **MSP** -> save + reboot.

**LLM outputs prose instead of JSON**
- Use an instruction-tuned GGUF model.
- Confirm `PromptCompiler` is injecting the mission schema.

**Video shows no feed**
- Check Device Manager for the FPV capture dongle (Imaging Devices).
- Try `uvc_index: 1` or `2` in `endpoints.yaml`.

**0xC0000139 on launch (MinGW)**
- Rebuild via a CMake preset; `-static-libgcc -static-libstdc++` is applied automatically.

---

## Design Decisions

**Why MSP instead of MAVLink?**
The Nanohawk ships with Betaflight, which does not speak MAVLink. MSP is the native protocol -- lower overhead, simpler framing, direct RC override without firmware replacement.

**Why local LLM?**
Privacy (prompts stay on-machine), low latency (~100 ms), no API cost, works offline.

**Why not LLM -> motors directly?**
LLMs hallucinate. Enforcing `Prompt -> JSON -> Safety Veto -> MSP` makes every command auditable, testable, and hard-limited before it reaches the hardware.

**Why C++20?**
Zero overhead on the serial framing path. Direct Win32 API access. No JVM or GIL. `std::jthread` keeps the pipeline clean.

---

## Next Steps

1. Set Betaflight receiver type to **MSP** in Betaflight Configurator.
2. Download a GGUF model into `models\llm\` and update `endpoints.yaml`.
3. Run `nanohawk_device_detection.exe` to confirm the full MSP session.
4. Run the CLI agent with a simple prompt to validate JSON -> MSP flow.
5. Build the GUI (Qt6 + OpenCV) for live video and telemetry.
6. First flight: `"Takeoff to 0.5 meters, hover 3 seconds, land"` -- indoors, clear area.

---

## License

Currently "Closed License".

This GitHub Repository is not currently sponsored by Emax USA - https://emax-usa.com/

## References

- [Betaflight MSP Protocol](https://github.com/betaflight/betaflight/blob/master/src/main/msp/msp_protocol.h)
- [EMAX Nanohawk 1S](https://emax-usa.com/products/nanohawk-1s-ultralight-brushless-fpv-drone)
- [llama.cpp Server](https://github.com/ggerganov/llama.cpp/blob/master/examples/server/README.md)
- [STM32 USB CDC driver](https://www.st.com/en/development-tools/stsw-stm32102.html)
- [Qt 6 CMake](https://doc.qt.io/qt-6/cmake-get-started.html)

- [OpenCV](https://docs.opencv.org/)








