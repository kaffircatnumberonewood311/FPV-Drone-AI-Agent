cmake_minimum_required(VERSION 3.26)
project(nanohawk_agent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NANOHAWK_BUILD_GUI "Build Qt/OpenCV desktop GUI" ON)
option(NANOHAWK_BUILD_TESTS "Build test targets" ON)

include(GNUInstallDirs)

find_package(OpenCV QUIET)
find_package(CURL QUIET)
find_package(MAVSDK QUIET CONFIG)

# Winsock is required by DeviceWatcher (UDP broadcast WiFi discovery).
# On MinGW/GCC builds, also link the C++ and GCC runtimes statically so the
# executables are self-contained (no libstdc++-6.dll / libgcc_s_seh-1.dll needed).
if(WIN32)
    set(NANOHAWK_PLATFORM_LIBS ws2_32)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_link_options(-static-libgcc -static-libstdc++)
    endif()
else()
    set(NANOHAWK_PLATFORM_LIBS "")
endif()

# Betaflight MSP protocol headers (standalone, no firmware deps).
# Copied from D:\DroneAI\betaflight-master\src\main\msp\ (GPL-3.0).
# Used by MspClient for reference; not included in our C++ code directly
# (we define our own typed wrappers in include/msp/MspClient.hpp).
set(BETAFLIGHT_MSP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/betaflight/msp)

set(NANOHAWK_CORE_HEADERS
    include/app/Bootstrap.hpp
    include/msp/MspClient.hpp
    include/app/ServiceLocator.hpp
    include/app/DeviceWatcher.hpp
    include/telemetry/MavlinkTransport.hpp
    include/telemetry/HeartbeatMonitor.hpp
    include/telemetry/VehicleStateStore.hpp
    include/video/VideoSource.hpp
    include/video/RtspSource.hpp
    include/video/UvcSource.hpp
    include/video/FrameBus.hpp
    include/llm/LlmClient.hpp
    include/llm/PromptCompiler.hpp
    include/llm/JsonPlanParser.hpp
    include/planning/MissionTypes.hpp
    include/planning/MissionPlanner.hpp
    include/planning/TaskGraph.hpp
    include/planning/MissionExecutor.hpp
    include/safety/RuleEngine.hpp
    include/safety/Geofence.hpp
    include/safety/BatteryGuard.hpp
    include/safety/LinkLossGuard.hpp
    include/safety/AbortController.hpp
    include/flight/ArduPilotGuidedAdapter.hpp
    include/flight/VelocityController.hpp
    include/flight/TakeoffLand.hpp
    include/flight/CommandArbiter.hpp
)

set(NANOHAWK_CORE_SOURCES
    src/app/Bootstrap.cpp
    src/app/ServiceLocator.cpp
    src/app/DeviceWatcher.cpp
    src/telemetry/MavlinkTransport.cpp
    src/telemetry/HeartbeatMonitor.cpp
    src/telemetry/VehicleStateStore.cpp
    src/video/VideoSource.cpp
    src/video/RtspSource.cpp
    src/video/UvcSource.cpp
    src/video/FrameBus.cpp
    src/llm/LlmClient.cpp
    src/llm/PromptCompiler.cpp
    src/llm/JsonPlanParser.cpp
    src/planning/MissionPlanner.cpp
    src/planning/TaskGraph.cpp
    src/planning/MissionExecutor.cpp
    src/safety/RuleEngine.cpp
    src/safety/Geofence.cpp
    src/safety/BatteryGuard.cpp
    src/safety/LinkLossGuard.cpp
    src/safety/AbortController.cpp
    src/flight/ArduPilotGuidedAdapter.cpp
    src/flight/VelocityController.cpp
    src/flight/TakeoffLand.cpp
    src/flight/CommandArbiter.cpp
    src/msp/MspClient.cpp
)

add_library(nanohawk_core STATIC ${NANOHAWK_CORE_HEADERS} ${NANOHAWK_CORE_SOURCES})
target_include_directories(nanohawk_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(nanohawk_core PRIVATE ${BETAFLIGHT_MSP_INCLUDE_DIR})
target_compile_features(nanohawk_core PUBLIC cxx_std_20)
target_link_libraries(nanohawk_core PRIVATE ${NANOHAWK_PLATFORM_LIBS})

if(OpenCV_FOUND)
    target_include_directories(nanohawk_core PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(nanohawk_core PRIVATE ${OpenCV_LIBS})
    target_compile_definitions(nanohawk_core PRIVATE NANOHAWK_WITH_OPENCV=1)
endif()

if(CURL_FOUND)
    target_link_libraries(nanohawk_core PRIVATE CURL::libcurl)
    target_compile_definitions(nanohawk_core PRIVATE NANOHAWK_WITH_CURL=1)
endif()

if(TARGET mavsdk::mavsdk)
    target_link_libraries(nanohawk_core PRIVATE mavsdk::mavsdk)
    target_compile_definitions(nanohawk_core PRIVATE NANOHAWK_WITH_MAVSDK=1)
elseif(TARGET MAVSDK::mavsdk)
    target_link_libraries(nanohawk_core PRIVATE MAVSDK::mavsdk)
    target_compile_definitions(nanohawk_core PRIVATE NANOHAWK_WITH_MAVSDK=1)
endif()

add_executable(nanohawk_agent_cli src/main.cpp)
target_include_directories(nanohawk_agent_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nanohawk_agent_cli PRIVATE nanohawk_core)

add_executable(launcher tools/launcher/main.cpp)
target_compile_features(launcher PRIVATE cxx_std_20)
target_link_libraries(launcher PRIVATE ${NANOHAWK_PLATFORM_LIBS})

if(NANOHAWK_BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets Network Concurrent QUIET)
    find_package(OpenCV QUIET)

    if(Qt6_FOUND AND OpenCV_FOUND)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)

        add_executable(nanohawk_agent
            src/gui/gui_main.cpp
            src/gui/MainWindow.cpp
            src/gui/VideoPane.cpp
            src/gui/PromptPane.cpp
            src/gui/TelemetryPane.cpp
            src/gui/MissionPane.cpp
            src/gui/SafetyPane.cpp
        )
        target_include_directories(nanohawk_agent PRIVATE include)
        target_link_libraries(nanohawk_agent PRIVATE nanohawk_core Qt6::Widgets Qt6::Network Qt6::Concurrent ${OpenCV_LIBS})
        target_compile_definitions(nanohawk_agent PRIVATE NANOHAWK_WITH_GUI=1 NANOHAWK_WITH_OPENCV=1)
    else()
        message(WARNING "Qt6/OpenCV not found; building CLI target only")
    endif()
endif()

if(NANOHAWK_BUILD_TESTS)
    enable_testing()
    add_executable(nanohawk_unit_tests test/unit/test_pipeline.cpp)
    target_link_libraries(nanohawk_unit_tests PRIVATE nanohawk_core)
    add_test(NAME nanohawk_unit_pipeline COMMAND nanohawk_unit_tests)

    add_executable(nanohawk_transport_tests test/unit/test_transports.cpp)
    target_link_libraries(nanohawk_transport_tests PRIVATE nanohawk_core)
    add_test(NAME nanohawk_transport_checks COMMAND nanohawk_transport_tests)

    add_executable(nanohawk_config_tests test/unit/test_config_wiring.cpp)
    target_link_libraries(nanohawk_config_tests PRIVATE nanohawk_core)
    add_test(NAME nanohawk_config_wiring COMMAND nanohawk_config_tests)

    add_executable(nanohawk_device_detection test/unit/test_device_detection.cpp)
    target_link_libraries(nanohawk_device_detection PRIVATE nanohawk_core)
    add_test(NAME nanohawk_device_detection COMMAND nanohawk_device_detection)
endif()
