#pragma once

#include <list>

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace vl53l0x_m {

class VL53L0XMSensor : public sensor::VL53L0XSensor {
 public:
  void update() override;
};

}  // namespace vl53l0x
}  // namespace esphome
