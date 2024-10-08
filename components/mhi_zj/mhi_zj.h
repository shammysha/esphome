#pragma once

#include "esphome/components/climate_ir/climate_ir.h"
#include <cinttypes>

namespace esphome {
namespace mhi_zj {
// Temperature
const uint8_t MHI_TEMP_MIN = 18;  // Celsius
const uint8_t MHI_TEMP_MAX = 30;  // Celsius

// Enum to represent horizontal directios
enum HorizontalDirection {
  HORIZONTAL_DIRECTION_MIDDLE = 0x48,
  HORIZONTAL_DIRECTION_LEFT = 0xC8,
  HORIZONTAL_DIRECTION_MIDDLE_LEFT = 0x88,
  HORIZONTAL_DIRECTION_MIDDLE_RIGHT = 0x08,
  HORIZONTAL_DIRECTION_RIGHT = 0xC4,
};

// Enum to represent vertical directions
enum VerticalDirection {
  VERTICAL_DIRECTION_UP = 0x02,
  VERTICAL_DIRECTION_MIDDLE_UP = 0x18,
  VERTICAL_DIRECTION_MIDDLE = 0x10,
  VERTICAL_DIRECTION_MIDDLE_DOWN = 0x08,
  VERTICAL_DIRECTION_DOWN = 0x00,
};


class MhiClimate : public climate_ir::ClimateIR {
 public:
  MhiClimate()
      : climate_ir::ClimateIR(
            MHI_TEMP_MIN, MHI_TEMP_MAX, 1.0f, true, true,
            std::set<climate::ClimateFanMode>{climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
                                              climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH,
                                              climate::CLIMATE_FAN_MIDDLE, climate::CLIMATE_FAN_FOCUS,
                                              climate::CLIMATE_FAN_DIFFUSE},
            std::set<climate::ClimateSwingMode>{climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL,
                                                climate::CLIMATE_SWING_HORIZONTAL, climate::CLIMATE_SWING_BOTH}) {}

  void set_horizontal_default(HorizontalDirection horizontal_direction) {
    this->default_horizontal_direction_ = horizontal_direction;
  }
  void set_vertical_default(VerticalDirection vertical_direction) {
    this->default_vertical_direction_ = vertical_direction;
  }

 protected:
  void transmit_state() override;
  /// Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;

  HorizontalDirection default_horizontal_direction_;
  VerticalDirection default_vertical_direction_;

};
}  // namespace mhi_zj
}  // namespace esphome
