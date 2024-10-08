#pragma once

#include "esphome/components/button/button.h"
#include "../ld2450.h"

namespace esphome {
  namespace ld2450 {

    class QueryButton: public button::Button, public Parented<LD2450Component> {
      public:
        QueryButton() = default;

      protected:
        void press_action() override;
    };

  }  // namespace ld2450
}  // namespace esphome
