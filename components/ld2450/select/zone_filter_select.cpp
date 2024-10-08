#include "zone_filter_select.h"

namespace esphome {
  namespace ld2450 {

    void ZoneFilterSelect::control(const std::string &value) {
      this->publish_state(value);
      this->parent_->set_zone_filter(state);
    }

  }  // namespace ld2450
}  // namespace esphome
