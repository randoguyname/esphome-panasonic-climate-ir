import esphome.codegen as cg
from esphome.components import button, climate_ir, select, time
import esphome.config_validation as cv
from esphome.types import ConfigType

AUTO_LOAD = ["climate_ir", "button", "select"]

climate_ir_panasonic_ns = cg.esphome_ns.namespace("climate_ir_panasonic")
PanasonicIrClimate = climate_ir_panasonic_ns.class_(
    "PanasonicIrClimate", climate_ir.ClimateIR
)
PanasonicACFanSwingSelect = climate_ir_panasonic_ns.class_(
    "PanasonicACFanSwingSelect", select.Select
)
PanasonicACButton = climate_ir_panasonic_ns.class_("PanasonicAcButton", button.Button)

CodeTypeEnum = (
    climate_ir_panasonic_ns.namespace("codes").namespace("CodeType").enum("CodeType")
)

CONF_TIME_ENTITY_ID = "time_entity_id"
CONF_BUTTONS = "buttons"
CONF_CANCEL_TIMER_BUTTON = "cancel_timer"
CONF_POWERFUL_BUTTON = "powerful"
CONF_ECO_BUTTON = "eco"
CONF_QUIET_BUTTON = "quiet"

CONF_FAN_SWING = "fan_swing_select"

BUTTON_SCHEMA = button.button_schema(
    PanasonicACButton, entity_category=cv.ENTITY_CATEGORY_CONFIG
)

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(PanasonicIrClimate).extend(
    {
        cv.Required(CONF_TIME_ENTITY_ID): cv.use_id(time.RealTimeClock),
        cv.Optional(
            CONF_BUTTONS,
            default={
                CONF_CANCEL_TIMER_BUTTON: {"name": "Cancel Timer"},
                CONF_POWERFUL_BUTTON: {"name": "Powerful"},
                CONF_ECO_BUTTON: {"name": "Eco"},
                CONF_QUIET_BUTTON: {"name": "Quiet"},
            },
        ): {
            cv.Optional(
                CONF_CANCEL_TIMER_BUTTON, default={"name": "Cancel Timer"}
            ): BUTTON_SCHEMA,
            cv.Optional(
                CONF_POWERFUL_BUTTON, default={"name": "Powerful"}
            ): BUTTON_SCHEMA,
            cv.Optional(CONF_ECO_BUTTON, default={"name": "Eco"}): BUTTON_SCHEMA,
            cv.Optional(CONF_QUIET_BUTTON, default={"name": "Quiet"}): BUTTON_SCHEMA,
        },
        cv.Optional(
            CONF_FAN_SWING, default={"name": "Fan Swing"}
        ): select.select_schema(
            PanasonicACFanSwingSelect, entity_category=cv.ENTITY_CATEGORY_CONFIG
        ),
    }
)

FAN_SWING_OPTIONS = [
    "AUTO",
    "TOP",
    "MTOP",
    "MIDDLE",
    "MBOTTOM",
    "BOTTOM",
]


async def to_code(config: ConfigType) -> None:
    var = await climate_ir.new_climate_ir(config)

    sel = await select.new_select(config[CONF_FAN_SWING], options=FAN_SWING_OPTIONS)

    cancel_timer_button = await button.new_button(
        config[CONF_BUTTONS][CONF_CANCEL_TIMER_BUTTON]
    )

    powerful_button = await button.new_button(
        config[CONF_BUTTONS][CONF_POWERFUL_BUTTON]
    )

    quiet_button = await button.new_button(config[CONF_BUTTONS][CONF_QUIET_BUTTON])

    eco_button = await button.new_button(config[CONF_BUTTONS][CONF_ECO_BUTTON])

    time_sensor = await cg.get_variable(config[CONF_TIME_ENTITY_ID])

    cg.add(var.set_time_entity(time_sensor))

    cg.add(var.set_fan_swing_select(sel))
    cg.add(var.set_button(cancel_timer_button, CodeTypeEnum.CANCEL_TIMER))
    cg.add(var.set_button(powerful_button, CodeTypeEnum.POWERFUL))
    cg.add(var.set_button(eco_button, CodeTypeEnum.ECO))
    cg.add(var.set_button(quiet_button, CodeTypeEnum.QUIET))
