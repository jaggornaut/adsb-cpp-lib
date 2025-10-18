# adsb-cpp-lib

A simple, modern C++ library for decoding ADS-B (Automatic Dependent Surveillance-Broadcast) messages.

This library is designed to process raw ADS-B data, such as data from SDR (Software Defined Radio) projects.

The implementation is made by following [this guide](https://airmetar.main.jp/radio/ADS-B%20Decoding%20Guide.pdf)

## Current Features

Currently, the library can decode the following message types (Mode-S Downlink Formats):

- **Identification** (TC=1-4): Flight ID (callsign) and aircraft category.
- **Airborne Position** (TC=9-18): Barometric altitude and the raw data needed for position calculation.
- **Airborne Velocity** (TC=19): Ground speed, heading, and vertical rate.

It also includes:
- CPR Position Calculation: An algorithm to calculate the exact geographic coordinates.
- Error Correction: Logic to fix single-bit errors using a CRC check.

## Requirements

- C++17
- CMake 3.10+

## Build
```shell
git clone https://github.com/jaggornaut/adsb-cpp-lib.git
cd adsb-cpp-lib

mkdir build
cd build

cmake ..
make
```

## Usage

### Basic decoding

Here is a short example that shows how to use the library to decode a raw message.

```cpp
#include <iostream>
#include <vector>
#include <memory>

#include "adsb/decoder.hpp"

int main() {
    // A raw ADS-B message (112 bits) from some source
    std::vector<int> raw_bits = { /* ... 112 bits here ... */ };

    // Use the 'decode' function to get a message object
    if (auto message = decoder::decode(raw_bits)) {
        std::cout << "Info: " << message->to_string() << std::endl;
    } else {
        std::cout << "Could not decode message: data is invalid." << std::endl;
    }
    return 0;
}
```

### Decoding a Position (CPR)

To calculate a position, you need two recent position messages (one "even" and one "odd") from the same aircraft. The application that uses this library needs to store the first message while it waits for the second.

```cpp
#include "adsb/decoder.hpp"
#include "adsb/message/AirbornePositionMessage.hpp"
#include <map>

// Map to store the last position message from each aircraft
std::map<std::string, AirbornePositionMessage> last_positions;

// Your receiver's location is needed
GlobalPosition my_receiver_location = {51.5, 10.12}; // Example: Lat 51.5, Lon 10.12

void process_new_message(const std::vector<int>& raw_bits) {
    auto message = decoder::decode(raw_bits);
    if (!message) return;

    // Check if it's a position message
    if (auto pos_msg = dynamic_cast<AirbornePositionMessage*>(message.get())) {
        std::string icao = pos_msg->get_icao();

        // Check if you have a stored message from this aircraft
        if (last_positions.count(icao)) {
            auto& last_msg = last_positions.at(icao);

            // Make sure the frames are different (one even, one odd)
            if (last_msg.is_odd_frame() != pos_msg->is_odd_frame()) {
                
                // Calculate the global position
                PositionResult result = decoder::calculate_global_position(*pos_msg, last_msg, my_receiver_location);

                if (result.is_valid) {
                    std::cout << "Position for " << icao << ": Lat "
                              << result.position.latitude << ", Lon "
                              << result.position.longitude << std::endl;
                }
            }
        }
        // Store the current message for the next cycle
        last_positions[icao] = *pos_msg;
    }
}
```