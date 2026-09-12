#pragma once

#include <cinttypes>
#include <array>

namespace esphome::climate_ir_panasonic::codes {

const uint8_t CODE_LEN = 8;

const std::array<uint8_t, CODE_LEN> CANCEL_TIMER =  {0x40, 0x04, 0x07, 0x20, 0x01, 0x6D, 0x4C, 0x76};
const std::array<uint8_t, CODE_LEN> ECO =           {0x40, 0x04, 0x07, 0x20, 0x01, 0xA1, 0xAC, 0x02};
const std::array<uint8_t, CODE_LEN> POWERFUL =      {0x40, 0x04, 0x07, 0x20, 0x01, 0x61, 0xAC, 0x82};
const std::array<uint8_t, CODE_LEN> QUIET =         {0x40, 0x04, 0x07, 0x20, 0x01, 0x81, 0xCC, 0x5C};

enum class CodeType {
  CANCEL_TIMER,
  ECO,
  POWERFUL,
  QUIET,
};

std::array<uint8_t, CODE_LEN> code_from_type(CodeType code_type);

}
