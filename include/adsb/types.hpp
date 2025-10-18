#pragma once

/**
 * @struct GlobalPosition
 * @brief Represents a geographical position with latitude and longitude.
 */
struct GlobalPosition {
    double latitude;
    double longitude;
};

/**
 * @struct PositionResult
 * @brief Holds the result of a position calculation.
 */
struct PositionResult {
    GlobalPosition position;
    bool is_valid;
};