#include "esphome/components/climate_ir/climate_ir.h"
#include "esphome/components/time/real_time_clock.h"

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

const uint8_t PAYLOAD_TEMPLATE_BIT_REVERSED[18] = {0, 0, 0x81, 0, 0, 0x80, 0x08, 0, '\r', 0, 0x80, 0, 0x08, 0, 0x04, 0xe0, ' ', 0x02};
const uint8_t MESSAGE_PREFIX[] = {0x40, 0x04, 0x07, 0x20, 0x00, 0x00, 0x00, 0x60};
const uint8_t PAYLOAD_PREFIX[] = {0x40, 0x04, 0x07, 0x20, 0x00};

class PanasonicIrClimate final : public climate_ir::ClimateIR {
  public:
    PanasonicIrClimate() : climate_ir::ClimateIR(TEMP_MIN, TEMP_MAX, 1.0f, true, true,
        {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
        climate::CLIMATE_FAN_HIGH},
        {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL}) {
            this->set_supported_custom_fan_modes(PANASONIC_FAN_MODES);
        };

    void set_time_entity(time::RealTimeClock* time_sensor) {
      this->time_sensor_ = time_sensor;
    };
  protected:
    time::RealTimeClock *time_sensor_{nullptr};
    void transmit_state() override;
    // bool on_receive(remote_base::RemoteReceiveData data) override;

    uint8_t calculate_checksum_(uint8_t payload[18]);
    void transmit_(uint8_t payload[18]);
    void transmit_sync_(remote_base::RemoteTransmitData *data);
    void transmit_high_(remote_base::RemoteTransmitData *data) {data->item(450, 1200); };
    void transmit_low_(remote_base::RemoteTransmitData *data) {data->item(450, 450); };
};

}
