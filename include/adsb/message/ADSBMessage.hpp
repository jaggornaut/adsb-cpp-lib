#pragma once

#include <chrono>
#include <string>
#include <vector>


    namespace adsb::message {
        /**
         * @class ADSBMessage
         * @brief Abstract base class for all ADS-B message types.
         *
         * This class provides the common interface and data for all messages,
         * such as the ICAO address, type code, and reception timestamp.
         */
        class ADSBMessage {
        public:
            /**
             * @brief Constructs an ADSBMessage object.
             * @param icao The 24-bit ICAO address of the aircraft as a hex string.
             * @param type_code The message Type Code (a value between 1 and 31).
             * @param payload The 56-bit message payload.
             */
            ADSBMessage(std::string icao, int type_code, std::vector<int> payload);

            virtual ~ADSBMessage() = default;

            const std::string& get_icao() const { return m_icao; }
            int get_type_code() const { return m_type_code; }
            const std::chrono::steady_clock::time_point& get_timestamp() const { return m_timestamp; }

            /**
             * @brief Returns a string representation of the message.
             * @return A formatted string with the main message data.
             */
            virtual std::string to_string() const;

        protected:
            const std::vector<int> m_payload;
            const int m_type_code;

        private:
            const std::string m_icao;
            const std::chrono::steady_clock::time_point m_timestamp;
        };
    }
