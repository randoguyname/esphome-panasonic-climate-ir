#include "climate_ir_panasonic.h"
#include "esphome/core/log.h"

namespace esphome::climate_ir_panasonic {

void PanasonicIrClimate::transmit_state() {
  std::array<uint8_t, PAYLOAD_LEN> payload = PAYLOAD_TEMPLATE_BIT_REVERSED;

  // power bit
  payload[12] |= this->mode == climate::ClimateMode::CLIMATE_MODE_OFF;

  payload[11] |= (uint8_t) this->target_temperature << 1u;

  ESPTime time = this->time_sensor_->now();
  uint16_t time_value = time.hour;
  time_value *= 60;
  time_value += time.minute;

  payload[2] |= time_value & 255u;
  payload[1] |= time_value >> 8u;


  Mode mode;
  switch (this->mode) {
    case climate::ClimateMode::CLIMATE_MODE_COOL:
      mode = Mode::COOL;
      break;
    case climate::ClimateMode::CLIMATE_MODE_DRY:
      mode = Mode::DRY;
      break;
    case climate::ClimateMode::CLIMATE_MODE_FAN_ONLY:
      mode = Mode::FAN;
      break;
    case climate::ClimateMode::CLIMATE_MODE_HEAT:
      mode = Mode::HEAT;
      break;
    default:
      mode = Mode::AUTO;
      break;
  }

  payload[12] |= (uint8_t) mode << 4u;

  FanSpeed speed;

  if (this->has_custom_fan_mode()) {
    StringRef fan_mode = this->get_custom_fan_mode();

    if (fan_mode.compare("FAN_LOW") == 0) {
      speed = FanSpeed::LEVEL_1;
    } else if (fan_mode.compare("FAN_MLOW") == 0) {
      speed = FanSpeed::LEVEL_2;
    } else if (fan_mode.compare("FAN_MEDIUM") == 0) {
      speed = FanSpeed::LEVEL_3;
    } else if (fan_mode.compare("FAN_MHIGH") == 0) {
      speed = FanSpeed::LEVEL_4;
    } else if (fan_mode.compare("FAN_HIGH") == 0) {
      speed = FanSpeed::LEVEL_5;
    } else {
      speed = FanSpeed::AUTO;
    }
  } else { // builtin fan modes
    switch (this->fan_mode.value()) {
      case climate::ClimateFanMode::CLIMATE_FAN_LOW:
        speed = FanSpeed::LEVEL_1;
        break;
      case climate::ClimateFanMode::CLIMATE_FAN_MEDIUM:
        speed = FanSpeed::LEVEL_3;
        break;
      case climate::ClimateFanMode::CLIMATE_FAN_HIGH:
        speed = FanSpeed::LEVEL_5;
        break;
      default:
        speed = FanSpeed::AUTO;
        break;
    }
  }

  payload[9] |= (uint8_t) speed << 4u;

  FanSwing swing = FanSwing::AUTO; // default to auto
  if (this->fan_swing_select_ != nullptr) {
    auto current_swing_option = this->fan_swing_select_->current_option();

    if (current_swing_option.compare("TOP") == 0) {
      swing = FanSwing::LEVEL_1;
    } else if (current_swing_option.compare("MTOP") == 0) {
      swing = FanSwing::LEVEL_2;
    } else if (current_swing_option.compare("MIDDLE") == 0) {
      swing = FanSwing::LEVEL_3;
    } else if (current_swing_option.compare("MBOTTOM") == 0) {
      swing = FanSwing::LEVEL_4;
    } else if (current_swing_option.compare("BOTTOM") == 0) {
      swing = FanSwing::LEVEL_5;
    }
  }

  payload[9] |= (uint8_t) swing;

  // TODO: timer configuration

  // Payload is now finished

  payload[0] = this->calculate_checksum_(payload);

  this->transmit_payload_(payload);
}

uint8_t PanasonicIrClimate::calculate_checksum_(std::array<uint8_t, PAYLOAD_LEN> payload) {
  uint8_t sum = 0;

  for (int i = 0; i < PAYLOAD_LEN; i ++) {
    sum += payload[i];
  }

  return sum;
}

void PanasonicIrClimate::transmit_sync_(remote_base::RemoteTransmitData *data) {
  data->item(250, 250);
  data->item(250, 250);
  data->item(250, 250);
  data->item(250, 250);
  data->item(250, 250);
  data->item(250, 2000);
}

void PanasonicIrClimate::transmit_header_(remote_base::RemoteTransmitData *data) {
  // Start with sync

  this->transmit_sync_(data);

  // Transmit generic message header

  for (auto byte : MESSAGE_PREFIX) {
    for (int j = 0; j < 8; j++) {
      if ((byte << j) & 0b10000000) {
        this->transmit_high_(data);
      } else {
        this->transmit_low_(data);
      }
    }
  }

  // spacer
  data->item(450, 10000);
}

void PanasonicIrClimate::transmit_payload_(std::array<uint8_t, PAYLOAD_LEN> payload) {
  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();

  this->transmit_header_(data);

  // re-sync

  this->transmit_sync_(data);

  // transmit payload prefix

  for (auto byte : PAYLOAD_PREFIX) {
    for (int j = 0; j < 8; j++) {
      if ((byte << j) & 0b10000000)
        this->transmit_high_(data);
      else
        this->transmit_low_(data);
    }
  }

  // transmit payload

  for (int i = PAYLOAD_LEN - 1; i >= 0; i--) {
    for (int j = 0; j < 8; j++) {
      if ((payload[i] >> j) & 1) {
        this->transmit_high_(data);
      } else {
        this->transmit_low_(data);
      }
    }
  }

  transmit.perform();
}

void PanasonicIrClimate::transmit_code_(std::array<uint8_t, codes::CODE_LEN> code) {
  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();

  this->transmit_header_(data);

  for (auto byte : code) {
    for (int j = 0; j < 8; j++) {
      if ((byte << j) & 0b10000000) {
        this->transmit_high_(data);
      } else {
        this->transmit_low_(data);
      }
    }
  }

  transmit.perform();
}

void PanasonicIrClimate::set_fan_swing_select(select::Select* fan_swing_select) {
  this->fan_swing_select_ = fan_swing_select;

  this->fan_swing_select_->add_on_state_callback([this] (size_t index) {
    FanSwing swing = this->get_fan_swing_from_string_(this->fan_swing_select_->current_option());

    if (swing == this->fan_swing_state_) return;

    this->fan_swing_state_ = swing;

    this->transmit_state();
  });
}

FanSwing PanasonicIrClimate::get_fan_swing_from_string_(StringRef value) {
  if (value.compare("TOP") == 0) {
    return FanSwing::LEVEL_1;
  } else if (value.compare("MTOP") == 0) {
    return FanSwing::LEVEL_2;
  } else if (value.compare("MIDDLE") == 0) {
    return FanSwing::LEVEL_3;
  } else if (value.compare("MBOTTOM") == 0) {
    return FanSwing::LEVEL_4;
  } else if (value.compare("BOTTOM") == 0) {
    return FanSwing::LEVEL_5;
  }

  return FanSwing::AUTO;
}

void PanasonicIrClimate::set_button(PanasonicAcButton* button, codes::CodeType button_type) {
  switch (button_type) {
    case codes::CodeType::CANCEL_TIMER:
      this->cancel_timer_button_ = button;
      break;
    case codes::CodeType::ECO:
      this->eco_button_ = button;
      break;
    case codes::CodeType::POWERFUL:
      this->powerful_button_ = button;
      break;
    case codes::CodeType::QUIET:
      this->quiet_button_ = button;
      break;
  }

  button->add_on_press_callback([this, button_type] {
    this->transmit_code_(codes::code_from_type(button_type));
  });

}

}  // namespace esphome::climate_ir_panasonic

