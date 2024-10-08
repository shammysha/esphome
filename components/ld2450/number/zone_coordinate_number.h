#pragma once

#include "esphome/components/number/number.h"
#include "../ld2450.h"

namespace esphome {
  namespace ld2450 {

    class ZoneCoordinateNumber: public number::Number, public Parented<LD2450Component> {
      public:
        ZoneCoordinateNumber();

      protected:
        void control(float value) override;
    };

  }  // namespace ld2450
}  // namespace esphome
