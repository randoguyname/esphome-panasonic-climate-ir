#pragma once

#include "esphome/components/climate_ir/climate_ir.h"
#include "esphome/components/time/real_time_clock.h"
#include "panasonic_ac_fan_swing_select.h"
#include "codes.h"
#include "panasonic_ac_button.h"
#include <cinttypes>


namespace esphome::climate_ir_panasonic {

#define PANASONIC_FAN_MODES { \
  "FAN_LOW", \
  "FAN_MLOW", \
  "FAN_MEDIUM", \
  "FAN_MHIGH", \
  "FAN_HIGH", \
  "FAN_AUTO" \
}

#define PANASONIC_SWING_MODES { \
  "SWING_LOW", \
  "SWING_MLOW", \
  "SWING_MIDDLE", \
  "SWING_MHIGH", \
  "SWING_HIGH", \
  "SWING_AUTO", \
}

enum class Mode : uint8_t {
  AUTO = 0b000,
  HEAT = 0b100,
  COOL = 0b011,
  DRY  = 0b010,
  FAN  = 0b110,
};

enum class FanSpeed : uint8_t {
  AUTO    = 0b1010,
  LEVEL_1 = 0b0011,
  LEVEL_2 = 0b0100,
  LEVEL_3 = 0b0101,
  LEVEL_4 = 0b0110,
  LEVEL_5 = 0b0111,
};

enum class FanSwing : uint8_t {
  AUTO    = 0b0011,
  LEVEL_1 = 0b0100,
  LEVEL_2 = 0b0101,
  LEVEL_3 = 0b1111,
  LEVEL_4 = 0b0001,
  LEVEL_5 = 0b0010,
};

const uint8_t TEMP_MIN = 16;  // TODO: update these
const uint8_t TEMP_MAX = 30;  // Celsius

const uint8_t MESSAGE_PREFIX_LEN = 8;
const uint8_t PAYLOAD_PREFIX_LEN = 5;
const uint8_t PAYLOAD_LEN = 18;

const std::array<uint8_t, PAYLOAD_LEN> PAYLOAD_TEMPLATE_BIT_REVERSED = {0, 0, 0x81, 0, 0, 0x80, 0x08, 0, '\r', 0, 0x80, 0, 0x08, 0, 0x04, 0xe0, ' ', 0x02};
const std::array<uint8_t, MESSAGE_PREFIX_LEN> MESSAGE_PREFIX = {0x40, 0x04, 0x07, 0x20, 0x00, 0x00, 0x00, 0x60};
const std::array<uint8_t, PAYLOAD_PREFIX_LEN> PAYLOAD_PREFIX = {0x40, 0x04, 0x07, 0x20, 0x00};

class PanasonicIrClimate final : public climate_ir::ClimateIR {
  public:
    PanasonicIrClimate() : climate_ir::ClimateIR(TEMP_MIN, TEMP_MAX, 1.0f, true, true,
        {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
        climate::CLIMATE_FAN_HIGH}) {
            this->set_supported_custom_fan_modes(PANASONIC_FAN_MODES);
        };

    void set_time_entity(time::RealTimeClock* time_sensor) {
      this->time_sensor_ = time_sensor;
    };

    void set_fan_swing_select(select::Select* fan_swing_select);

    void set_button(PanasonicAcButton* button, codes::CodeType button_type);

  protected:
    time::RealTimeClock *time_sensor_{nullptr};
    select::Select *fan_swing_select_ {nullptr};
    FanSwing fan_swing_state_;

    PanasonicAcButton *cancel_timer_button_ {nullptr};
    PanasonicAcButton *powerful_button_ {nullptr};
    PanasonicAcButton *eco_button_ {nullptr};
    PanasonicAcButton *quiet_button_ {nullptr};

    FanSwing get_fan_swing_from_string_(StringRef value);

    void transmit_state() override;
    // bool on_receive(remote_base::RemoteReceiveData data) override;

    uint8_t calculate_checksum_(std::array<uint8_t, PAYLOAD_LEN> payload);
    void transmit_payload_(std::array<uint8_t, PAYLOAD_LEN> payload);
    void transmit_sync_(remote_base::RemoteTransmitData *data);
    void transmit_header_(remote_base::RemoteTransmitData *data);
    void transmit_code_(std::array<uint8_t, codes::CODE_LEN>);
    void transmit_high_(remote_base::RemoteTransmitData *data) {data->item(450, 1200); };
    void transmit_low_(remote_base::RemoteTransmitData *data) {data->item(450, 450); };
};

} // namespace esphome::climate-ir-panasonic
