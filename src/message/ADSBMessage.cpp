#include "adsb/message/ADSBMessage.hpp"

#include <iomanip>
#include <sstream>
#include <utility>


ADSBMessage::ADSBMessage(std::string icao, int type_code, std::vector<int> payload)
    : m_payload(std::move(payload)),
      m_icao(std::move(icao)),
      m_type_code(type_code),
      m_timestamp(std::chrono::steady_clock::now())
{}

std::string ADSBMessage::to_string() const {
    std::stringstream ss;
    ss << "[ADSB] ICAO: " << m_icao
       << " | TC: " << std::setw(2) << m_type_code;
    return ss.str();
}