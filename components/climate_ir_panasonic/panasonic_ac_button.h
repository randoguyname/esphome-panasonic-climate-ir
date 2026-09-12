#pragma once

#include "esphome/components/button/button.h"
#include "esphome/core/component.h"


namespace esphome::climate_ir_panasonic {
class PanasonicAcButton final : public button::Button, public Component {
  void press_action() override {
    // TODO: callback?
  }

};
}
