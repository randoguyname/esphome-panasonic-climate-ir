#pragma once

#include "esphome/components/select/select.h"
#include "esphome/core/component.h"

namespace esphome::climate_ir_panasonic {

class PanasonicACFanSwingSelect final : public esphome::select::Select, public esphome::Component {
 protected:
  void control(const std::string &value) override {
    this->publish_state(value);
  }
};

}  // namespace esphome
