#include "ld2450.h"

#include <utility>

#define highbyte(val) (uint8_t)((val) >> 8)
#define lowbyte(val) (uint8_t)((val) &0xff)

namespace esphome {
  namespace ld2450 {

    static const char *const TAG = "ld2450";

    LD2450Component::LD2450Component() : zone_coordinate_numbers_(NUM_ZONES, std::vector<number::Number *>(4)) {}

    void LD2450Component::dump_config() {
      ESP_LOGCONFIG(TAG, "LD2450:");

#ifdef USE_TEXT_SENSOR
      LOG_TEXT_SENSOR(TAG, "  VersionTextSensor", this->version_text_sensor_);
      LOG_TEXT_SENSOR(TAG, "  MacTextSensor", this->mac_text_sensor_);
#endif

#ifdef USE_SELECT
      LOG_SELECT(TAG, "  BaudRateSelect", this->baud_rate_select_);
#endif

#ifdef USE_SWITCH
      LOG_SWITCH(TAG, "  BluetoothSwitch", this->bluetooth_switch_);
      LOG_SWITCH(TAG, "  MultiTargetSwitch", this->multi_target_switch_);
#endif

#ifdef USE_BUTTON
      LOG_BUTTON(TAG, "  ResetButton", this->reset_button_);
      LOG_BUTTON(TAG, "  RestartButton", this->restart_button_);
      LOG_BUTTON(TAG, "  QueryButton", this->query_button_);
#endif
      ESP_LOGCONFIG(TAG, "  Zones:");

#ifdef USE_SELECT
      LOG_SELECT(TAG, "    ZoneFilterSelect", this->zone_filter_select_);
#endif
      for (int i = 0; i < this->zone_coordinate_numbers_.size(); i++) {
        ESP_LOGCONFIG(TAG, "    %s:", std::to_string(i+1));

#ifdef USE_NUMBER
        for (int j = 0; j < this->zone_coordinate_numbers_[i].size(); j++) {
          LOG_NUMBER(TAG, "      ZoneCoordinateNumber", this->zone_coordinate_numbers_[i][j]);
        }
#endif
      }
      ESP_LOGCONFIG(TAG, "  Targets:");

      for (int i = 0; i < NUM_TARGETS; i++) {
        ESP_LOGCONFIG(TAG, "    %s:", std::to_string(i+1));

#ifdef USE_SENSOR
        LOG_SENSOR(TAG, "      TargetXSensor", this->target_x_sensors_[i]);
        LOG_SENSOR(TAG, "      TargetYSensor", this->target_y_sensors_[i]);
        LOG_SENSOR(TAG, "      TargetSpeedSensor", this->target_speed_sensors_[i]);
        LOG_SENSOR(TAG, "      TargetResolutionSensor", this->target_resolution_sensors_[i]);
#endif
      }
    }

    void LD2450Component::setup() {
      ESP_LOGCONFIG(TAG, "Setting up LD2450...");
      this->read_all_info();
      ESP_LOGCONFIG(TAG, "Mac Address : %s", const_cast<char*>(this->mac_.c_str()));
      ESP_LOGCONFIG(TAG, "Firmware Version : %s", const_cast<char*>(this->version_.c_str()));
      ESP_LOGCONFIG(TAG, "LD2450 setup complete.");
    }

    void LD2450Component::read_all_info() {
      this->set_config_mode_(true);
      this->get_version_();
      this->get_mac_();
      this->get_zone_filter_();
      this->set_config_mode_(false);

#ifdef USE_SELECT
      const auto baud_rate = std::to_string(this->parent_->get_baud_rate());
      if (this->baud_rate_select_ != nullptr && this->baud_rate_select_->state != baud_rate) {
        this->baud_rate_select_->publish_state(baud_rate);
      }
#endif
    }

    void LD2450Component::restart_and_read_all_info() {
      this->set_config_mode_(true);
      this->restart_();
      this->set_timeout(1000, [this]() {
        this->read_all_info();
      });
    }

    void LD2450Component::loop() {
      const int max_line_length = 160;
      static uint8_t buffer[max_line_length];

      while (available()) {
        this->readline_(read(), buffer, max_line_length);
      }
    }

    void LD2450Component::send_command_(uint8_t command, const uint8_t *command_value, int command_value_len) {
      ESP_LOGV(TAG, "Sending COMMAND %02X", command);
      // frame start bytes
      this->write_array(CMD_FRAME_HEADER, 4);
      // length bytes
      int len = 2;
      if (command_value != nullptr) len += command_value_len;
      this->write_byte(lowbyte(len));
      this->write_byte(highbyte(len));

      // command
      this->write_byte(lowbyte(command));
      this->write_byte(highbyte(command));

      // command value bytes
      if (command_value != nullptr) {
        for (int i = 0; i < command_value_len; i++) {
          this->write_byte(command_value[i]);
        }
      }
      // frame end bytes
      this->write_array(CMD_FRAME_END, 4);

      delay(50); // NOLINT
    }

    void LD2450Component::handle_periodic_data_(uint8_t *buffer, int len) {
      if (len < 30) {
        return;  // 4 frame start bytes + 8 bytes * 3 (per each target) + 2 frame end bytes
      }
      if (buffer[0] != 0xAA || buffer[1] != 0xFF || buffer[2] != 0x03 || buffer[3] != 0x00) {  // check 4 frame start bytes
        return;
      }
      if (buffer[len - 2] != 0x55 || buffer[len - 1] != 0xCC) { //  data end=0x55, 0xCC
        return;
      }

      /*
       Reduce data update rate to prevent home assistant database size grow fast
       */
      int32_t current_millis = millis();
      if (current_millis - this->last_periodic_millis_ < this->throttle_) {
        return;
      }
      this->last_periodic_millis_ = current_millis;

      bool activity = false;

      for (int t = 0; t < NUM_TARGETS; t++) {
        int offset = 4 + t*8;

        int target_x = this->two_byte_to_int_(buffer[offset], buffer[offset + 1] & 0x7F);
        target_x = (buffer[offset + 1] >> 7 ? 0 - target_x : target_x) / 10;

        int target_y = this->two_byte_to_int_(buffer[offset + 2], buffer[offset + 3] & 0x7F);
        target_y = (buffer[offset + 3] >> 7 ? 0 - target_y : target_y) / 10;

        int target_speed = this->two_byte_to_int_(buffer[offset + 4], buffer[offset + 5] & 0x7F);
        target_speed = (buffer[offset + 5] >> 7 ? 0 - target_speed : target_speed) / 10;

        int target_resolution = this->two_byte_to_int_(buffer[offset + 6], buffer[offset + 7]) / 10;

        if (target_x == 0 && target_y == 0 && target_speed == 0 && target_resolution == 0) {
          continue;
        }

#ifdef USE_SENSOR
        if (this->target_x_sensors_[t] != nullptr && this->target_x_sensors_[t]->state != target_x) {
          this->target_x_sensors_[t]->publish_state(target_x);
        }
        if (this->target_y_sensors_[t] != nullptr && this->target_y_sensors_[t]->state != target_y) {
          this->target_y_sensors_[t]->publish_state(target_y);
        }
        if (this->target_speed_sensors_[t] != nullptr && this->target_speed_sensors_[t]->state != target_speed) {
          this->target_speed_sensors_[t]->publish_state(target_speed);
        }
        if (this->target_resolution_sensors_[t] != nullptr && this->target_resolution_sensors_[t]->state != target_resolution) {
          this->target_resolution_sensors_[t]->publish_state(target_resolution);
        }
#endif
        activity = true;
      }

#ifdef USE_BINARY_SENSOR
      if (this->activity_binary_sensor_ != nullptr && this->activity_binary_sensor_->state != activity) {
        this->activity_binary_sensor_->publish_state(activity);
      }
#endif
    }

    std::string LD2450Component::format_version(uint8_t *buffer) {
      std::string::size_type version_size = 256;
      std::string version;
      do {
        version.resize(version_size + 1);
        version_size = std::snprintf(&version[0], version.size(), VERSION_FMT, buffer[13], buffer[12], buffer[17], buffer[16], buffer[15], buffer[14]);
      } while (version_size + 1 > version.size());
      version.resize(version_size);
      return version;
    }

    std::string LD2450Component::format_mac(uint8_t *buffer) {
      std::string::size_type mac_size = 256;
      std::string mac;
      do {
        mac.resize(mac_size + 1);
        mac_size = std::snprintf(&mac[0], mac.size(), MAC_FMT, buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
      } while (mac_size + 1 > mac.size());
      mac.resize(mac_size);
      if (mac == NO_MAC) {
        return UNKNOWN_MAC;
      }
      return mac;
    }

#ifdef USE_NUMBER
    std::function<void(void)> LD2450Component::set_number_value(number::Number *n, float value) {
      float normalized_value = value * 1.0;
      if (n != nullptr && (!n->has_state() || n->state != normalized_value)) {
        n->state = normalized_value;
        return [n, normalized_value]() { n->publish_state(normalized_value); };
      }
      return []() {};
    }
#endif

    bool LD2450Component::handle_ack_data_(uint8_t *buffer, int len) {
      ESP_LOGV(TAG, "Handling ACK DATA for COMMAND %02X", buffer[COMMAND]);

      if (len < 10) {
        ESP_LOGE(TAG, "Error with last command : incorrect length");
        return true;
      }

      if (buffer[0] != 0xFD || buffer[1] != 0xFC || buffer[2] != 0xFB || buffer[3] != 0xFA) {  // check 4 frame start bytes
        ESP_LOGE(TAG, "Error with last command : incorrect Header");
        return true;
      }

      if (buffer[COMMAND_STATUS] != 0x01) {
        ESP_LOGE(TAG, "Error with last command : status != 0x01");
        return true;
      }

      if (this->two_byte_to_int_(buffer[8], buffer[9]) != 0x00) {
        ESP_LOGE(TAG, "Error with last command , last buffer was: %u , %u", buffer[8], buffer[9]);
        return true;
      }

      switch (buffer[COMMAND]) {

        case lowbyte(CMD_ENABLE_CONF):
          ESP_LOGV(TAG, "Handled Enable conf command");
          break;

        case lowbyte(CMD_DISABLE_CONF):
          ESP_LOGV(TAG, "Handled Disabled conf command");
          break;

        case lowbyte(CMD_SET_BAUD_RATE):
          ESP_LOGV(TAG, "Handled baud rate change command");
#ifdef USE_SELECT
          if (this->baud_rate_select_ != nullptr) {
            ESP_LOGE(TAG, "Change uart baud rate in config to %s and reinstall", this->baud_rate_select_->state.c_str());
          }
#endif
          break;

        case lowbyte(CMD_VERSION):
          this->version_ = this->format_version(buffer);
          ESP_LOGV(TAG, "FW Version is: %s", const_cast<char*>(this->version_.c_str()));
#ifdef USE_TEXT_SENSOR
          if (this->version_text_sensor_ != nullptr) {
            this->version_text_sensor_->publish_state(this->version_);
          }
#endif
          break;

        case lowbyte(CMD_QUERY_ZONE_FILTER): {
            this->zone_filter_ = static_cast<int>(this->two_byte_to_int_(buffer[10], buffer[11]));

            std::string zone_filter = ZONE_FILTER_INT_TO_ENUM.at( this->zone_filter_ );
            ESP_LOGV(TAG, "Zone FIlter is: %s", const_cast<char*>(zone_filter.c_str()));


  #ifdef USE_SELECT
            if (this->zone_filter_select_ != nullptr
                && this->zone_filter_select_->state != zone_filter) {
              this->zone_filter_select_->publish_state(zone_filter);
            }
  #endif

            for (int t = 0; t < this->zone_coordinate_numbers_.size(); t++) {
              for (int p = 0; p < this->zone_coordinate_numbers_[t].size(); p++) {
                int pos = this->two_byte_to_int_(buffer[12 + p*2], buffer[12 + p*2 + 1]);

  #ifdef USE_NUMBER
                if (this->zone_coordinate_numbers_[t][p] == nullptr
                    || this->zone_coordinate_numbers_[t][p]->has_state() == false
                    || this->zone_coordinate_numbers_[t][p]->state != pos / 10
                ) {
                  this->zone_coordinate_numbers_[t][p]->publish_state(pos / 10);
                }
  #endif
              }
            }
          }
          break;

        case lowbyte(CMD_MAC):
          if (len < 20) {
            return false;
          }
          this->mac_ = this->format_mac(buffer);
          ESP_LOGV(TAG, "MAC Address is: %s", const_cast<char*>(this->mac_.c_str()));

#ifdef USE_TEXT_SENSOR
          if (this->mac_text_sensor_ != nullptr) {
            this->mac_text_sensor_->publish_state(this->mac_);
          }
#endif

#ifdef USE_SWITCH
          if (this->bluetooth_switch_ != nullptr) {
            this->bluetooth_switch_->publish_state(this->mac_ != UNKNOWN_MAC);
          }
#endif

          break;

        case lowbyte(CMD_SET_ZONE_FILTER):
          ESP_LOGV(TAG, "Handled zone filter command");
          break;

        case lowbyte(CMD_BLUETOOTH):
          ESP_LOGV(TAG, "Handled bluetooth command");
          break;

        default:
          break;
      }

      return true;
    }

    void LD2450Component::readline_(char readch, uint8_t *buffer, int len) {
      static int pos = 0;

      if (readch >= 0) {
        if (pos < len - 1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        } else {
          pos = 0;
        }

        if (pos >= 2 && buffer[pos - 2] == 0x55 && buffer[pos - 1] == 0xCC) {
          ESP_LOGV(TAG, "Will handle Periodic Data");
          this->handle_periodic_data_(buffer, pos);
          pos = 0;  // Reset position index ready for next time

        } else if (pos >= 4 && buffer[pos - 4] == 0x04 && buffer[pos - 3] == 0x03 && buffer[pos - 2] == 0x02 && buffer[pos - 1] == 0x01) {
          ESP_LOGV(TAG, "Will handle ACK Data");

          if (this->handle_ack_data_(buffer, pos)) {
            pos = 0;  // Reset position index ready for next time
          } else {
            ESP_LOGV(TAG, "ACK Data incomplete");
          }
        }
      }
    }

    void LD2450Component::set_config_mode_(bool enable) {
      uint8_t cmd = enable ? CMD_ENABLE_CONF : CMD_DISABLE_CONF;
      uint8_t cmd_value[2] = { 0x01, 0x00 };
      this->send_command_(cmd, enable ? cmd_value : nullptr, 2);
    }

    void LD2450Component::set_bluetooth(bool enable) {
      this->set_config_mode_(true);
      uint8_t enable_cmd_value[2] = { 0x01, 0x00 };
      uint8_t disable_cmd_value[2] = { 0x00, 0x00 };
      this->send_command_(CMD_BLUETOOTH, enable ? enable_cmd_value : disable_cmd_value, 2);
      this->set_timeout(200, [this]() {
        this->restart_and_read_all_info();
      });
    }

    void LD2450Component::set_baud_rate(const std::string &state) {
      this->set_config_mode_(true);
      uint8_t cmd_value[2] = { BAUD_RATE_ENUM_TO_INT.at(state), 0x00 };
      this->send_command_(CMD_SET_BAUD_RATE, cmd_value, 2);
      this->set_timeout(200, [this]() {
        this->restart_();
      });
    }

    void LD2450Component::set_multi_target(bool enable) {
      this->set_config_mode_(true);
      uint8_t enable_cmd_value[2] = { 0x01, 0x00 };
      uint8_t disable_cmd_value[2] = { 0x00, 0x00 };
      this->send_command_(CMD_MULTI_TARGET, enable ? enable_cmd_value : disable_cmd_value, 2);
      this->set_timeout(200, [this]() {
        this->restart_and_read_all_info();
      });
    }

    void LD2450Component::factory_reset() {
      this->set_config_mode_(true);
      this->send_command_(CMD_RESET, nullptr, 0);
      this->set_timeout(200, [this]() {
        this->restart_and_read_all_info();
      });
    }

    void LD2450Component::restart_() {
      this->send_command_(CMD_RESTART, nullptr, 0);
    }

    void LD2450Component::get_version_() {
      this->send_command_(CMD_VERSION, nullptr, 0);
    }

    void LD2450Component::get_mac_() {
      uint8_t cmd_value[2] = { 0x01, 0x00 };
      this->send_command_(CMD_MAC, cmd_value, 2);
    }

    void LD2450Component::get_zone_filter_() {
      this->send_command_(CMD_QUERY_ZONE_FILTER, nullptr, 0);
    }



#ifdef USE_NUMBER
    void LD2450Component::set_zone_coordinate_number(int zone, int pos, number::Number *n) {

      this->zone_coordinate_numbers_[zone][pos] = n;

    }

    void LD2450Component::set_zone_coordinate() {

      if (this->zone_filter_ < 0) {
        return;
      }

      uint8_t zones[28] = {
          0x00, 0x00,
          lowbyte(this->zone_filter_), highbyte(this->zone_filter_),
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
      };

      for (int i = 0; i < this->zone_coordinate_numbers_.size(); i++) {
        number::Number *left_num = this->zone_coordinate_numbers_[i][POS_LEFT];
        number::Number *top_num = this->zone_coordinate_numbers_[i][POS_TOP];
        number::Number *right_num = this->zone_coordinate_numbers_[i][POS_RIGHT];
        number::Number *bottom_num = this->zone_coordinate_numbers_[i][POS_BOTTOM];

        if (!(left_num->has_state() && top_num->has_state() && right_num->has_state() && bottom_num->has_state())) {
          continue;
        }

        int left = static_cast<int>(left_num->state) * 10;
        int top = static_cast<int>(top_num->state) * 10;
        int right = static_cast<int>(right_num->state) * 10;
        int bottom = static_cast<int>(bottom_num->state) * 10;

        if (left == 0 && top == 0 && right == 0 && bottom == 0) {
          continue;
        }

        int pos = 4 + i*8;
        zones[pos] = lowbyte(left);
        zones[pos+1] = highbyte(left);
        zones[pos+2] = lowbyte(top);
        zones[pos+3] = highbyte(top);
        zones[pos+4] = lowbyte(right);
        zones[pos+5] = highbyte(right);
        zones[pos+6] = lowbyte(bottom);
        zones[pos+7] = highbyte(bottom);

      }
      this->set_config_mode_(true);
      this->send_command_(CMD_SET_ZONE_FILTER, zones, 28);
      delay(50);  // NOLINT
      this->get_zone_filter_();
      this->set_config_mode_(false);

    }
#endif

#ifdef USE_SELECT
    void LD2450Component::set_zone_filter(const std::string &value) {
      this->zone_filter_ = ZONE_FILTER_ENUM_TO_INT.at( value );
      this->set_zone_coordinate();
    }

#endif

#ifdef USE_SENSOR
    void LD2450Component::set_target_x_sensor(int target, sensor::Sensor *s) {
      this->target_x_sensors_[target] = s;
    }
    void LD2450Component::set_target_y_sensor(int target, sensor::Sensor *s) {
      this->target_y_sensors_[target] = s;
    }
    void LD2450Component::set_target_speed_sensor(int target, sensor::Sensor *s) {
      this->target_speed_sensors_[target] = s;
    }
    void LD2450Component::set_target_resolution_sensor(int target, sensor::Sensor *s) {
      this->target_resolution_sensors_[target] = s;
    }
#endif

  }  // namespace ld2450
}  // namespace esphome
