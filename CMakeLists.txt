cmake_minimum_required(VERSION 3.10)
project(cpp-adsb-lib VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(adsb-lib
        src/decoder.cpp
        src/utils.cpp
        src/message/ADSBMessage.cpp
        src/message/AirbornePositionMessage.cpp
        src/message/IdentificationMessage.cpp
        src/message/VelocityMessage.cpp
)

target_include_directories(adsb-lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
