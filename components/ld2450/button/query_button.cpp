#include "query_button.h"

namespace esphome {
  namespace ld2450 {

    void QueryButton::press_action() {
      this->parent_->read_all_info();
    }

  }  // namespace ld2450
}  // namespace esphome
