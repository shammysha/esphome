#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif

#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif

#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif

#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif

#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

#include <map>

namespace esphome {
  namespace ld2450 {

#define CHECK_BIT(var, pos) (((var) >> (pos)) & 1)
#define STATE_SIZE 8
#define NUM_ZONES 3
#define NUM_TARGETS 3
#define POS_LEFT 0
#define POS_TOP 1
#define POS_RIGHT 2
#define POS_BOTTOM 3

// Commands
    static const uint8_t CMD_ENABLE_CONF = 0x00FF;
    static const uint8_t CMD_DISABLE_CONF = 0x00FE;
    static const uint8_t CMD_SINGLE_TARGET = 0x0080;
    static const uint8_t CMD_MULTI_TARGET = 0x0090;
    static const uint8_t CMD_VERSION = 0x00A0;
    static const uint8_t CMD_SET_BAUD_RATE = 0x00A1;
    static const uint8_t CMD_RESET = 0x00A2;
    static const uint8_t CMD_RESTART = 0x00A3;
    static const uint8_t CMD_BLUETOOTH = 0x00A4;
    static const uint8_t CMD_MAC = 0x00A5;
    static const uint8_t CMD_QUERY_ZONE_FILTER = 0x00C1;
    static const uint8_t CMD_SET_ZONE_FILTER = 0x00C2;

    static const char VERSION_FMT[] = "%u.%02X.%02X%02X%02X%02X";
    static const char MAC_FMT[] = "%02X:%02X:%02X:%02X:%02X:%02X";
    static const std::string UNKNOWN_MAC("unknown");
    static const std::string NO_MAC("08:05:04:03:02:01");

    enum BaudRateStructure : uint8_t {
      BAUD_RATE_9600 = 1,
      BAUD_RATE_19200 = 2,
      BAUD_RATE_38400 = 3,
      BAUD_RATE_57600 = 4,
      BAUD_RATE_115200 = 5,
      BAUD_RATE_230400 = 6,
      BAUD_RATE_256000 = 7,
      BAUD_RATE_460800 = 8
    };

    static const std::map<std::string, uint8_t> BAUD_RATE_ENUM_TO_INT {
      { "9600", BAUD_RATE_9600 },
      { "19200", BAUD_RATE_19200 },
      { "38400", BAUD_RATE_38400 },
      { "57600", BAUD_RATE_57600 },
      { "115200", BAUD_RATE_115200 },
      { "230400", BAUD_RATE_230400 },
      { "256000", BAUD_RATE_256000 },
      { "460800", BAUD_RATE_460800 }
    };

    enum ZoneFilter : uint8_t {
      FILTER_DISABLED = 0x00,
      FILTER_DETECT = 0x01,
      FILTER_IGNORE = 0x02
    };

    static const std::map<std::string, uint8_t> ZONE_FILTER_ENUM_TO_INT {
      { "disabled", FILTER_DISABLED },
      { "detect", FILTER_DETECT },
      { "ignore", FILTER_IGNORE }
    };

    static const std::map<uint8_t, std::string> ZONE_FILTER_INT_TO_ENUM {
      { FILTER_DISABLED, "disabled" },
      { FILTER_DETECT, "detect" },
      { FILTER_IGNORE, "ignore" }
    };

// Command Header & Footer
    static const uint8_t CMD_FRAME_HEADER[4] = { 0xFD, 0xFC, 0xFB, 0xFA };
    static const uint8_t CMD_FRAME_END[4] = { 0x04, 0x03, 0x02, 0x01 };

// Data Header & Footer
    static const uint8_t DATA_FRAME_HEADER[4] = { 0xAA, 0xFF, 0x03, 0x00 };
    static const uint8_t DATA_FRAME_END[2] = { 0x55, 0xCC };

    enum PeriodicDataStructure : uint8_t {
      TARGET1_X = 0,
      TARGET1_Y = 2,
      TARGET1_SPEED = 4,
      TARGET1_RESOLUTION = 6
    };

    enum PeriodicDataValue : uint8_t {
      HEAD = 0XAA, END = 0x55, CHECK = 0x00
    };

    enum AckDataStructure : uint8_t {
      COMMAND = 6, COMMAND_STATUS = 7
    };



    class LD2450Component: public Component, public uart::UARTDevice {
#ifdef USE_SENSOR
      SUB_SENSOR(target_x)
      SUB_SENSOR(target_y)
      SUB_SENSOR(target_speed)
      SUB_SENSOR(target_resolution)
#endif

#ifdef USE_BINARY_SENSOR
      SUB_BINARY_SENSOR(activity)
#endif
#ifdef USE_TEXT_SENSOR
      SUB_TEXT_SENSOR(version)
      SUB_TEXT_SENSOR(mac)
#endif
#ifdef USE_SELECT
      SUB_SELECT(baud_rate)
      SUB_SELECT(zone_filter)
#endif
#ifdef USE_SWITCH
      SUB_SWITCH(bluetooth)
      SUB_SWITCH(multi_target)
#endif
#ifdef USE_BUTTON
      SUB_BUTTON(reset)
      SUB_BUTTON(restart)
      SUB_BUTTON(query)
#endif
#ifdef USE_NUMBER
      SUB_NUMBER(zone_coordinate)
#endif
      public:
        LD2450Component();
        void setup() override;
        void dump_config() override;
        void loop() override;

#ifdef USE_SELECT
        void set_zone_filter(const std::string &value);
#endif

#ifdef USE_NUMBER
        void set_zone_coordinate_number(int zone, int pos, number::Number *n);
        void set_zone_coordinate();
#endif

#ifdef USE_SENSOR
        void set_target_x_sensor(int target, sensor::Sensor *s);
        void set_target_y_sensor(int target, sensor::Sensor *s);
        void set_target_speed_sensor(int target, sensor::Sensor *s);
        void set_target_resolution_sensor(int target, sensor::Sensor *s);
#endif

        void set_throttle(uint16_t value) {
          this->throttle_ = value;
        };
        void read_all_info();
        void restart_and_read_all_info();
        void set_bluetooth(bool enable);
        void set_multi_target(bool enable);
        void set_baud_rate(const std::string &state);
        void factory_reset();

      protected:
        uint16_t two_byte_to_int_(int firstbyte, int secondbyte) {
          return (uint16_t)(secondbyte << 8) + firstbyte;
        }
        std::string format_version(uint8_t *buffer);
        std::string format_mac(uint8_t *buffer);
        std::function<void(void)> set_number_value(number::Number *n, float value);

        void send_command_(uint8_t command_str, const uint8_t *command_value, int command_value_len);
        void set_config_mode_(bool enable);
        void handle_periodic_data_(uint8_t *buffer, int len);
        bool handle_ack_data_(uint8_t *buffer, int len);
        void readline_(char readch, uint8_t *buffer, int len);
        void get_version_();
        void get_mac_();
        void get_zone_filter_();
        void restart_();

        int32_t last_periodic_millis_ = millis();
        uint16_t throttle_;
        std::string version_;
        std::string mac_;
        int zone_filter_ = -1;

#ifdef USE_NUMBER
        std::vector<std::vector<number::Number *> > zone_coordinate_numbers_;
#endif

#ifdef USE_SENSOR
        std::vector<sensor::Sensor *> target_x_sensors_ = std::vector<sensor::Sensor *>(NUM_TARGETS);
        std::vector<sensor::Sensor *> target_y_sensors_ = std::vector<sensor::Sensor *>(NUM_TARGETS);
        std::vector<sensor::Sensor *> target_speed_sensors_ = std::vector<sensor::Sensor *>(NUM_TARGETS);
        std::vector<sensor::Sensor *> target_resolution_sensors_ = std::vector<sensor::Sensor *>(NUM_TARGETS);
#endif
    };

  }  // namespace ld2450
}  // namespace esphome
