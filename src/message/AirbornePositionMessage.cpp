#include "adsb/message/AirbornePositionMessage.hpp"
#include "adsb/utils.hpp"

#include <stdexcept>
#include <sstream>

using namespace adsb::message;

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

int AirbornePositionMessage::decode_altitude(std::vector<int>::const_iterator begin,
                                              std::vector<int>::const_iterator end) {
    if (std::distance(begin, end) != 12) {
        throw std::invalid_argument("Altitude data must be 12 bits long.");
    }

    bool q_bit = (*(begin + 4) == 1);

    if (q_bit) {
        int n = utils::bits_to_int(begin, begin + 4);
        for (auto it = begin + 5; it != end; ++it) {
            n = (n << 1) | *it;
        }
        return (n * 25) - 1000;
    }

    std::vector<int> bits(begin, end);

    int C1 = bits[0];
    int A1 = bits[1];
    int C2 = bits[2];
    int A2 = bits[3];
    int C4 = bits[5];
    int A4 = bits[6];
    int B1 = bits[7];
    int B2 = bits[8];
    int D2 = bits[9];
    int B4 = bits[10];
    int D4 = bits[11];

    auto gray3_to_bin = [](int g2, int g1, int g0) -> int {
        int b2 = g2;
        int b1 = b2 ^ g1;
        int b0 = b1 ^ g0;
        return b2 * 4 + b1 * 2 + b0;
    };

    int C = gray3_to_bin(C1, C2, C4); //  500ft
    int D = gray3_to_bin(0,  D2, D4);
    int A = gray3_to_bin(A1, A2, A4);
    int B = gray3_to_bin(B1, B2, B4);

    if (C == 0 && A == 0) return 0;

    int five_hundred = C * 10 + D;
    int one_hundred  = A * 5 + B;

    static const int gillham_100ft[] = {-9999, 0, 100, 200, 300, -200, -100, -9999};

    if (one_hundred < 1 || one_hundred > 6) return 0;

    int offset_100 = gillham_100ft[one_hundred];
    int base_500   = (five_hundred * 500) - 1200;

    return base_500 + offset_100;
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