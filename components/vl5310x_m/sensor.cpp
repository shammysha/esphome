#include "sensor.h"
#include "esphome/core/log.h"

/*
 * Most of the code in this integration is based on the VL53L0x library
 * by Pololu (Pololu Corporation), which in turn is based on the VL53L0X
 * API from ST.
 *
 * For more information about licensing, please view the included LICENSE.txt file
 * in the vl53l0x integration directory.
 */

namespace esphome {
namespace vl53l0x_m {

void VL53L0XMSensor::update() {
  if (this->initiated_read_ || this->waiting_for_interrupt_) {
    this->publish_state(NAN);
    // this->status_momentary_warning("update", 5000);
    ESP_LOGW(TAG, "%s - update called before prior reading complete - initiated:%d waiting_for_interrupt:%d. Restarting",
             this->name_.c_str(), this->initiated_read_, this->waiting_for_interrupt_);
    this->enable_pin_->digital_write(false);
    delayMicroseconds(100);
    this->enable_pin_->digital_write(true);
  }

  // initiate single shot measurement
  reg(0x80) = 0x01;
  reg(0xFF) = 0x01;

  reg(0x00) = 0x00;
  reg(0x91) = this->stop_variable_;
  reg(0x00) = 0x01;
  reg(0xFF) = 0x00;
  reg(0x80) = 0x00;

  reg(0x00) = 0x01;
  this->waiting_for_interrupt_ = false;
  this->initiated_read_ = true;
  // wait for timeout
}

