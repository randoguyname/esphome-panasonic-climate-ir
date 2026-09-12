#include "codes.h"

namespace esphome::climate_ir_panasonic::codes {

std::array<uint8_t, CODE_LEN> code_from_type(CodeType code_type) {
  switch (code_type) {
    case CodeType::CANCEL_TIMER:
      return CANCEL_TIMER;
    case CodeType::ECO:
      return ECO;
    case CodeType::POWERFUL:
      return POWERFUL;
    case CodeType::QUIET:
      return QUIET;
  }
}

}  // namespace esphome::climate_ir_panasonic::codes
