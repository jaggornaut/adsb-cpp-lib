#include "adsb/decoder.hpp"

#include "adsb/message/ADSBMessage.hpp"
#include "adsb/message/IdentificationMessage.hpp"
#include "adsb/message/AirbornePositionMessage.hpp"
#include "adsb/message/VelocityMessage.hpp"

#include <chrono>
#include <cmath>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace {

    // CRC checksum polynomial for ADS-B messages
    constexpr uint32_t ADS_B_CRC_POLY = 0xFFF409;

    /**
     * @brief Calculates the number of longitude zones (NL) for a given latitude.
     * @param lat The aircraft's latitude in degrees.
     * @return The number of longitude zones.
     */
    int NL(double lat) {
        if (std::abs(lat) >= 87.0) return 1;
        if (lat == 0) return 59;
        double term = 1.0 - std::cos(M_PI / 30.0);
        double lat_rad = lat * M_PI / 180.0;
        double bottom = std::cos(lat_rad) * std::cos(lat_rad);
        double nl_val = 2.0 * M_PI / std::acos(1.0 - (term / bottom));
        return static_cast<int>(std::floor(nl_val));
    }

    /**
     * @brief Performs a CRC check on a 112-bit raw message.
     */
    bool check_crc(const std::vector<int>& message) {
        if (message.size() != 112) return false;

        uint8_t bytes[14] = {0};
        for (size_t i = 0; i < 112; ++i) {
            if (message[i] == 1) {
                bytes[i / 8] |= (1 << (7 - (i % 8)));
            }
        }

        uint32_t crc = 0;
        for (int i = 0; i < 11; i++) {
            crc ^= static_cast<uint32_t>(bytes[i]) << 16;
            for (int j = 0; j < 8; j++) {
                crc <<= 1;
                if (crc & 0x1000000) {
                    crc ^= ADS_B_CRC_POLY;
                }
            }
        }

        crc ^= static_cast<uint32_t>(bytes[11]) << 16;
        crc ^= static_cast<uint32_t>(bytes[12]) << 8;
        crc ^= static_cast<uint32_t>(bytes[13]);

        return (crc & 0xFFFFFF) == 0;
    }

}

namespace adsb::decoder {
    namespace FieldIndex {
        constexpr int ICAO_START    = 8;
        constexpr int ICAO_END      = 31;
        constexpr int PAYLOAD_START = 32;
        constexpr int TYPE_CODE_END = 36;
        constexpr int PAYLOAD_END   = 88;
    }

    std::unique_ptr<adsb::message::ADSBMessage> decode(const std::vector<int>& raw_bits) {
        if (raw_bits.size() != 112) return nullptr;

        std::vector<int> corrected_bits = raw_bits;
        bool is_valid = check_crc(corrected_bits);

        // Attempt single-bit error correction if initial CRC fails
        if (!is_valid) {
            for (int i = 0; i < 112; ++i) {
                corrected_bits[i] = 1 - corrected_bits[i]; // Flip bit
                if (check_crc(corrected_bits)) {
                    is_valid = true;
                    break;
                }
                corrected_bits[i] = raw_bits[i]; // Flip back
            }
        }

        if (!is_valid) return nullptr;

        int df = 0;
        for (int i = 0; i < 5; ++i)
            df = (df << 1) | corrected_bits[i];
        if (df != 17) return nullptr;

        unsigned int icao_val = 0;
        for (int i = FieldIndex::ICAO_START; i <= FieldIndex::ICAO_END; ++i) {
            icao_val = (icao_val << 1) | corrected_bits[i];
        }

        std::stringstream icao_ss;
        icao_ss << std::hex << std::setfill('0') << std::setw(6) << icao_val;
        std::string icao = icao_ss.str();

        int type_code = 0;
        for (int i = FieldIndex::PAYLOAD_START; i <= FieldIndex::TYPE_CODE_END; ++i) {
            type_code = (type_code << 1) | corrected_bits[i];
        }

        std::vector<int> payload(corrected_bits.cbegin() + FieldIndex::PAYLOAD_START,
                                 corrected_bits.cbegin() + FieldIndex::PAYLOAD_END);

        switch (type_code) {
            case 1: case 2: case 3: case 4:
                return std::make_unique<adsb::message::IdentificationMessage>(icao, type_code, payload);
            case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18:
                return std::make_unique<adsb::message::AirbornePositionMessage>(icao, type_code, payload);
            case 19:
                return std::make_unique<adsb::message::VelocityMessage>(icao, type_code, payload);
            default:
                return nullptr;
        }
    }

    adsb::types::PositionResult calculate_global_position(
        const adsb::message::AirbornePositionMessage& msg_a,
        const adsb::message::AirbornePositionMessage& msg_b,
        const adsb::types::GlobalPosition& ref_pos) {

        if (msg_a.is_odd_frame() == msg_b.is_odd_frame()) {
            return {{0.0, 0.0}, false};
        }

        const adsb::message::AirbornePositionMessage& even_msg = msg_a.is_odd_frame() ? msg_b : msg_a;
        const adsb::message::AirbornePositionMessage& odd_msg = msg_a.is_odd_frame() ? msg_a : msg_b;
        auto t_even = even_msg.get_timestamp();
        auto t_odd = odd_msg.get_timestamp();

        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(t_even - t_odd);
        if (std::abs(time_diff.count()) > 10000) {
            return {{0.0, 0.0}, false};
        }

        const double CPR_MAX = 131072.0;
        double cpr_lat_even = even_msg.get_cpr_latitude_raw() / CPR_MAX;
        double cpr_lon_even = even_msg.get_cpr_longitude_raw() / CPR_MAX;
        double cpr_lat_odd = odd_msg.get_cpr_latitude_raw() / CPR_MAX;
        double cpr_lon_odd = odd_msg.get_cpr_longitude_raw() / CPR_MAX;

        const int NZ = 15;
        double d_lat_even = 360.0 / (4.0 * NZ);
        double d_lat_odd  = 360.0 / (4.0 * NZ - 1.0);

        double j = std::floor(59.0 * cpr_lat_even - 60.0 * cpr_lat_odd + 0.5);

        double r_lat_even = d_lat_even * (std::fmod(j, 60.0) + cpr_lat_even);
        double r_lat_odd  = d_lat_odd  * (std::fmod(j, 59.0) + cpr_lat_odd);

        if (r_lat_even >= 270.0) r_lat_even -= 360.0;
        if (r_lat_odd  >= 270.0) r_lat_odd  -= 360.0;

        bool is_even_newer = t_even > t_odd;
        double latitude = is_even_newer ? r_lat_even : r_lat_odd;

        int nl = NL(latitude);
        if (nl <= 0) {
            return {{0.0, 0.0}, false};
        }

        double m = std::floor(cpr_lon_even * (nl - 1.0) - cpr_lon_odd * nl + 0.5);
        double nl_mod = std::max(static_cast<double>(nl - (is_even_newer ? 0 : 1)), 1.0);
        double d_lon = 360.0 / nl_mod;

        double longitude_base = is_even_newer ? cpr_lon_even : cpr_lon_odd;
        double longitude = d_lon * (std::fmod(m, nl_mod) + longitude_base);

        longitude += d_lon * std::round((ref_pos.longitude - longitude) / d_lon);

        if (longitude > 180.0) longitude -= 360.0;

        return {{latitude, longitude}, true};
    }

}