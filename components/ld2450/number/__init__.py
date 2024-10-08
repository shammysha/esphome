import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_TIMEOUT,
    DEVICE_CLASS_DISTANCE,
    UNIT_CENTIMETER,
    ENTITY_CATEGORY_CONFIG,
    ICON_RULER,
)
from .. import CONF_LD2450_ID, LD2450Component, ld2450_ns

ZoneCoordinateNumber = ld2450_ns.class_("ZoneCoordinateNumber", number.Number)

CONF_LEFT = "left"
CONF_TOP = "top"
CONF_RIGHT = "right"
CONF_BOTTOM = "bottom"

MAP_COORDINATES = {
    CONF_LEFT: 0,
    CONF_TOP: 1,
    CONF_RIGHT: 2,
    CONF_BOTTOM: 3,
}

CONFIG_SCHEMA = cv.Schema({
        cv.GenerateID(CONF_LD2450_ID): cv.use_id(LD2450Component),  
})

CONFIG_SCHEMA = CONFIG_SCHEMA.extend({
    cv.Optional(f"zone{x}"): cv.Schema(
        {
            cv.Required(CONF_LEFT): number.number_schema(
                ZoneCoordinateNumber,
                device_class=DEVICE_CLASS_DISTANCE,
                unit_of_measurement=UNIT_CENTIMETER,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon=ICON_RULER,
            ),
            cv.Required(CONF_TOP): number.number_schema(
                ZoneCoordinateNumber,
                device_class=DEVICE_CLASS_DISTANCE,
                unit_of_measurement=UNIT_CENTIMETER,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon=ICON_RULER,
            ),
            cv.Required(CONF_RIGHT): number.number_schema(
                ZoneCoordinateNumber,
                device_class=DEVICE_CLASS_DISTANCE,
                unit_of_measurement=UNIT_CENTIMETER,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon=ICON_RULER,
            ),
            cv.Required(CONF_BOTTOM): number.number_schema(
                ZoneCoordinateNumber,
                device_class=DEVICE_CLASS_DISTANCE,
                unit_of_measurement=UNIT_CENTIMETER,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon=ICON_RULER,
            ),
        }
    )
    for x in range(1, 4)
})


async def to_code(config):
    ld2450_component = await cg.get_variable(config[CONF_LD2450_ID])
    for x in range(1,4):
        if zone_conf := config.get(f"zone{x}"):
            for p in MAP_COORDINATES:
                coordinate_config =  zone_conf.get(p)
                if coordinate_config:
                    n = cg.new_Pvariable(coordinate_config[CONF_ID])
                    await number.register_number(n, coordinate_config, min_value=-1000, max_value=1000, step=1)
                    
                    await cg.register_parented(n, config[CONF_LD2450_ID])
                    cg.add(ld2450_component.set_zone_coordinate_number(n))
