import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_MOTION,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_MOTION_SENSOR,
)
from . import CONF_LD2450_ID, LD2450Component

DEPENDENCIES = ["ld2450"]
CONF_ACTIVITY = "activity"


CONFIG_SCHEMA = {
    cv.GenerateID(CONF_LD2450_ID): cv.use_id(LD2450Component),
    cv.Optional(CONF_ACTIVITY): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_MOTION,
        icon=ICON_MOTION_SENSOR,
    )
}


async def to_code(config):
    ld2450_component = await cg.get_variable(config[CONF_LD2450_ID])
    if activity_config := config.get(CONF_ACTIVITY):
        sens = await binary_sensor.new_binary_sensor(activity_config)
        cg.add(ld2450_component.set_activity_binary_sensor(sens))
