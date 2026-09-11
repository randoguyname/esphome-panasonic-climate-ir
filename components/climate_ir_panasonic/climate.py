import esphome.codegen as cg
from esphome.components import climate_ir, time
import esphome.config_validation as cv
from esphome.types import ConfigType

AUTO_LOAD = ["climate_ir"]

climate_ir_panasonic_ns = cg.esphome_ns.namespace("climate_ir_panasonic")
PanasonicIrClimate = climate_ir_panasonic_ns.class_(
    "PanasonicIrClimate", climate_ir.ClimateIR
)

CONF_TIME_ENTITY_ID = "time_entity_id"

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(PanasonicIrClimate).extend(
    {
        cv.Required(CONF_TIME_ENTITY_ID): cv.use_id(time.RealTimeClock),
    }
)


async def to_code(config: ConfigType) -> None:
    var = await climate_ir.new_climate_ir(config)
    sens = await cg.get_variable(config[CONF_TIME_ENTITY_ID])

    cg.add(var.set_time_entity(sens))
