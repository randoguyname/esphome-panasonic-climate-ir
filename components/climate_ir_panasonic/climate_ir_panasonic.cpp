#include "climate_ir_panasonic.h"
#include "esphome/core/log.h"

namespace esphome::climate_ir_panasonic {

void PanasonicIrClimate::transmit_state() {
  uint8_t payload[18];
  std::copy(PAYLOAD_TEMPLATE_BIT_REVERSED, PAYLOAD_TEMPLATE_BIT_REVERSED + 18, payload);

  // power bit
  payload[12] |= this->mode == climate::ClimateMode::CLIMATE_MODE_OFF;

  payload[11] |= (uint8_t) this->target_temperature << 1;

  ESPTime time = this->time_sensor_->now();
  uint16_t time_value = time.hour;
  time_value *= 60;
  time_value += time.minute;

  payload[2] |= time_value & 255;
  payload[1] |= time_value >> 8;


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

  payload[12] |= (uint8_t) mode << 4;

  FanSpeed speed;

  if (this->has_custom_fan_mode()) {
    StringRef fan_mode = this->get_custom_fan_mode();

    if (fan_mode.compare("FAN_LOW")) {
      speed = FanSpeed::LEVEL_1;
    } else if (fan_mode.compare("FAN_MLOW")) {
      speed = FanSpeed::LEVEL_2;
    } else if (fan_mode.compare("FAN_MEDIUM")) {
      speed = FanSpeed::LEVEL_3;
    } else if (fan_mode.compare("FAN_MHIGH")) {
      speed = FanSpeed::LEVEL_4;
    } else if (fan_mode.compare("FAN_HIGH")) {
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

  payload[9] |= (uint8_t) speed << 4;

  FanSwing swing;

  switch (this->swing_mode) { // TODO: add all air swing options
    case climate::ClimateSwingMode::CLIMATE_SWING_OFF:
      swing = FanSwing::LEVEL_1;
      break;
    case climate::ClimateSwingMode::CLIMATE_SWING_VERTICAL:
    default:
      swing = FanSwing::AUTO;
      break;
  }

  payload[9] |= (uint8_t) swing;

  // TODO: timer configuration

  // Payload is now finished

  payload[0] = this->calculate_checksum_(payload);

  this->transmit_(payload);
}

uint8_t PanasonicIrClimate::calculate_checksum_(uint8_t payload[18]) {
  uint8_t sum = 0;

  for (int i = 0; i < 18; i ++) {
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

void PanasonicIrClimate::transmit_(uint8_t payload[18]) {
  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();

  // Start with sync

  this->transmit_sync_(data);

  // Transmit generic message header

  for (int i = 0; i < MESSAGE_PREFIX_LEN; i++) {
    for (int j = 0; j < 8; j++) {
      if ((MESSAGE_PREFIX[i] << j) & 0b10000000)
        this->transmit_high_(data);
      else
        this->transmit_low_(data);
    }
  }

  // spacer
  data->item(450, 10000);


  // re-sync

  this->transmit_sync_(data);

  // transmit payload

  for (int i = 17; i >= 0; i--) {
    for (int j = 0; j < 8; j++) {
      if ((payload[i] >> j) & 1)
        this->transmit_high_(data);
      else
        this->transmit_low_(data);
    }
  }

  transmit.perform();
}

}
