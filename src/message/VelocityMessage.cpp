#include "adsb/message/VelocityMessage.hpp"
#include "adsb/utils.hpp"

#include <sstream>
#include <iomanip>
#include <cmath>


namespace {
    namespace FieldIndex {
        constexpr int SUBTYPE_START    = 5;
        constexpr int SUBTYPE_END      = 8;

        constexpr int EW_SIGN_BIT      = 13;
        constexpr int EW_VEL_START     = 14;
        constexpr int EW_VEL_END       = 24;

        constexpr int NS_SIGN_BIT      = 24;
        constexpr int NS_VEL_START     = 25;
        constexpr int NS_VEL_END       = 35;

        constexpr int VR_SIGN_BIT      = 36;
        constexpr int VR_START         = 37;
        constexpr int VR_END           = 46;
    }
}

using namespace adsb::message;

VelocityMessage::VelocityMessage(const std::string& icao, int type_code, const std::vector<int>& payload)
    : ADSBMessage(icao, type_code, payload) {
    decode_payload();
}

void VelocityMessage::decode_payload() {
    int subtype = utils::bits_to_int(m_payload.cbegin() + FieldIndex::SUBTYPE_START,
                                     m_payload.cbegin() + FieldIndex::SUBTYPE_END);

    if (subtype == 1 || subtype == 2) {

        const int s_ew = m_payload[FieldIndex::EW_SIGN_BIT];
        const int v_ew_raw = utils::bits_to_int(m_payload.cbegin() + FieldIndex::EW_VEL_START,
                                                m_payload.cbegin() + FieldIndex::EW_VEL_END);

        const int s_ns = m_payload[FieldIndex::NS_SIGN_BIT];
        const int v_ns_raw = utils::bits_to_int(m_payload.cbegin() + FieldIndex::NS_VEL_START,
                                                m_payload.cbegin() + FieldIndex::NS_VEL_END);

        const int s_vr = m_payload[FieldIndex::VR_SIGN_BIT];
        const int vr_raw = utils::bits_to_int(m_payload.cbegin() + FieldIndex::VR_START,
                                              m_payload.cbegin() + FieldIndex::VR_END);

        double vel_ew = (v_ew_raw == 0) ? 0.0 : (v_ew_raw - 1.0);
        if (s_ew == 1) vel_ew = -vel_ew;

        double vel_ns = (v_ns_raw == 0) ? 0.0 : (v_ns_raw - 1.0);
        if (s_ns == 1) vel_ns = -vel_ns;

        m_vertical_rate = (vr_raw == 0) ? 0 : (vr_raw - 1) * 64;
        if (s_vr == 1) m_vertical_rate = -m_vertical_rate;

        m_speed = std::sqrt(vel_ew * vel_ew + vel_ns * vel_ns);

        double heading_rad = std::atan2(vel_ew, vel_ns);
        m_heading = (heading_rad * 180.0) / M_PI;
        if (m_heading < 0) {
            m_heading += 360.0;
        }
    } else {
        m_speed = 0.0;
        m_heading = 0.0;
        m_vertical_rate = 0;
    }
}

std::string VelocityMessage::to_string() const {
    std::stringstream ss;
    ss << ADSBMessage::to_string();
    ss << " | Speed: "   << std::fixed << std::setprecision(1) << m_speed << " kn"
       << " | Hdg: "     << std::fixed << std::setprecision(1) << m_heading << " deg"
       << " | VRate: "   << m_vertical_rate << " ft/min";
    return ss.str();
}