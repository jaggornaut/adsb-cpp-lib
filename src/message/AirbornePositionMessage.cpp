#include "adsb/message/AirbornePositionMessage.hpp"
#include "adsb/utils.hpp"

#include <stdexcept>
#include <sstream>

AirbornePositionMessage::AirbornePositionMessage(const std::string& icao, int type_code, const std::vector<int>& payload)
    : ADSBMessage(icao, type_code, payload) {
    decode_payload();
}

void AirbornePositionMessage::decode_payload() {
    m_surveillance_status = utils::bits_to_int(m_payload.cbegin() + 5, m_payload.cbegin() + 7);
    m_nic_supplement_b = m_payload[7];
    m_altitude = decode_altitude(m_payload.cbegin() + 8, m_payload.cbegin() + 20);
    m_time_bit = (m_payload[20] == 1);
    m_is_odd = (m_payload[21] == 1);
    m_cpr_lat = utils::bits_to_int(m_payload.cbegin() + 22, m_payload.cbegin() + 39);
    m_cpr_lon = utils::bits_to_int(m_payload.cbegin() + 39, m_payload.cbegin() + 56);
}

int AirbornePositionMessage::decode_altitude(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end) {
    if (std::distance(begin, end) != 12) {
        throw std::invalid_argument("Altitude data must be 12 bits long.");
    }

    bool q_bit = (*(begin + 4) == 1);

    int n = 0;
    n = utils::bits_to_int(begin, begin + 4);
    for (auto it = begin + 5; it != end; ++it) {
        n = (n << 1) | *it;
    }

    int increment = q_bit ? 25 : 100;
    return (n * increment) - 1000;
}

std::string AirbornePositionMessage::to_string() const {
    std::stringstream ss;
    ss << ADSBMessage::to_string()
       << " | Alt: " << m_altitude << " ft"
       << " | Frame: " << (m_is_odd ? "Odd" : "Even")
       << " | CPR Lat: " << m_cpr_lat
       << " | CPR Lon: " << m_cpr_lon;
    return ss.str();
}