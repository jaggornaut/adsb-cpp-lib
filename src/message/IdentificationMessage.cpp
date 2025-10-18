#include "adsb/message/IdentificationMessage.hpp"
#include <sstream>

static const std::string FLIGHT_NAME_CHARS = "?ABCDEFGHIJKLMNOPQRSTUVWXYZ????? ???????????????0123456789??????";

IdentificationMessage::IdentificationMessage(const std::string& icao, int type_code, const std::vector<int>& payload)
    : ADSBMessage(icao, type_code, payload) {
    decode_payload();
}

void IdentificationMessage::decode_payload() {
    m_category = decode_category(m_payload.cbegin() + 5, m_payload.cbegin() + 8);
    m_flight_name = decode_flight_name(m_payload.cbegin() + 8, m_payload.cbegin() + 56);
}

IdentificationMessage::EmitterCategory
IdentificationMessage::decode_category(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end) {
    if (std::distance(begin, end) != 3) return EmitterCategory::UNKNOWN;

    int category_code = 0;
    for (auto it = begin; it != end; ++it) {
        category_code = (category_code << 1) | *it;
    }

    if (m_type_code == 4) {
        category_code += 8;
    } else if (m_type_code == 3) {
        category_code += 16;
    }

    return static_cast<EmitterCategory>(category_code);
}

std::string
IdentificationMessage::decode_flight_name(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end) {
    if (std::distance(begin, end) != 48) return "N/A";

    std::string flight_name = "";
    flight_name.reserve(8);

    for (int i = 0; i < 8; ++i) {
        int char_code = 0;
        for (int j = 0; j < 6; ++j) {
            char_code = (char_code << 1) | *(begin + (i * 6) + j);
        }

        if (char_code < FLIGHT_NAME_CHARS.length()) {
            flight_name += FLIGHT_NAME_CHARS[char_code];
        } else {
            flight_name += '?';
        }
    }

    size_t last = flight_name.find_last_not_of(' ');
    if (last == std::string::npos) return "";
    return flight_name.substr(0, last + 1);
}


std::string IdentificationMessage::to_string() const {
    std::stringstream ss;
    ss << ADSBMessage::to_string();
    ss << " | Flight Name: " << m_flight_name
       << " | Category: " << ::to_string(m_category);
    return ss.str();
}

std::string to_string(IdentificationMessage::EmitterCategory category) {
    switch (category) {
        case IdentificationMessage::EmitterCategory::LIGHT:               return "Light";
        case IdentificationMessage::EmitterCategory::SMALL:               return "Small";
        case IdentificationMessage::EmitterCategory::LARGE:               return "Large";
        case IdentificationMessage::EmitterCategory::HIGH_VORTEX_LARGE:   return "High Vortex";
        case IdentificationMessage::EmitterCategory::HEAVY:               return "Heavy";
        case IdentificationMessage::EmitterCategory::HIGH_PERFORMANCE:    return "High Performance";
        case IdentificationMessage::EmitterCategory::ROTORCRAFT:          return "Rotorcraft";
        case IdentificationMessage::EmitterCategory::GLIDER_SAILPLANE:    return "Glider";
        case IdentificationMessage::EmitterCategory::LIGHTER_THAN_AIR:    return "Lighter-than-air";
        case IdentificationMessage::EmitterCategory::UNMANNED_AERIAL_VEHICLE: return "UAV";
        case IdentificationMessage::EmitterCategory::SURFACE_EMERGENCY_VEHICLE: return "Surface Emergency Vehicle";
        case IdentificationMessage::EmitterCategory::SURFACE_SERVICE_VEHICLE: return "Surface Service Vehicle";
        default:                                                          return "Unknown";
    }
}