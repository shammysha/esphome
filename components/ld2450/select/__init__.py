import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
    CONF_BAUD_RATE,
    ICON_THERMOMETER,
    ICON_RULER,
)
from .. import CONF_LD2450_ID, LD2450Component, ld2450_ns

CONF_ZONE_FILTER = "zone_filter"
ICON_LEAK = "mdi:liak"

BaudRateSelect = ld2450_ns.class_("BaudRateSelect", select.Select)
ZoneFilterSelect = ld2450_ns.class_("ZoneFilterSelect", select.Select)

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD2450_ID): cv.use_id(LD2450Component),
    cv.Optional(CONF_BAUD_RATE): select.select_schema(
        BaudRateSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_THERMOMETER,
    ),
    cv.Optional(CONF_ZONE_FILTER): select.select_schema(
        ZoneFilterSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_LEAK,
    ),    
}


async def to_code(config):
    ld2450_component = await cg.get_variable(config[CONF_LD2450_ID])
    if baud_rate_config := config.get(CONF_BAUD_RATE):
        s = await select.new_select(
            baud_rate_config,
            options=[
                "9600",
                "19200",
                "38400",
                "57600",
                "115200",
                "230400",
                "256000",
                "460800",
            ],
        )
        await cg.register_parented(s, config[CONF_LD2450_ID])
        cg.add(ld2450_component.set_baud_rate_select(s))
    if zone_filter_config := config.get(CONF_ZONE_FILTER):
        s = await select.new_select(
            zone_filter_config,
            options=[
                "disabled",
                "detect",
                "ignore",
            ],
        )
        await cg.register_parented(s, config[CONF_LD2450_ID])
        cg.add(ld2450_component.set_zone_filter_select(s))        
        