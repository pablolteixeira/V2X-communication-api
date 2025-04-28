#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <vector>
#include <map>
#include <functional>
#include <cmath>
#include <iomanip>

// Message types for our autonomous vehicle system
enum MessageType {
    POSITION_UPDATE = 1,
    SENSOR_DATA = 2,
    CONTROL_COMMAND = 3,
    TELEMETRY = 4
};

// Message structures
struct PositionMessage {
    MessageType type;
    double x;
    double y;
    double heading; // in radians
    double speed;   // in m/s
};

struct SensorMessage {
    MessageType type;
    enum SensorType { LIDAR, CAMERA, RADAR, ULTRASONIC };
    SensorType sensor_type;
    double readings[16];  // sensor readings
};

struct ControlMessage {
    MessageType type;
    double steering;  // -1.0 to 1.0
    double throttle;  // 0.0 to 1.0
    double brake;     // 0.0 to 1.0
};

uint64_t get_timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}