import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_DISTANCE,
    UNIT_CENTIMETER,
    DEVICE_CLASS_ILLUMINANCE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_RULER,
    ICON_FLASH,
    ICON_MOTION_SENSOR,
)
from . import CONF_LD2450_ID, LD2450Component

DEPENDENCIES = ["ld2450"]
UNIT_CENTIMETER_PER_SECOND = "cm|s"
CONF_TARGET_X = "x"
CONF_TARGET_Y = "y"
CONF_TARGET_SPEED = "speed"
CONF_TARGET_RESOLUTION = "resolution"

ICON_SPEEDOMETR = "mdi:speedometer"
ICON_RESOLUTION = "mdi:resize"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LD2450_ID): cv.use_id(LD2450Component),
    }
)

CONFIG_SCHEMA = CONFIG_SCHEMA.extend(
    {
        cv.Optional(f"target{x}"): cv.Schema(
            {
                cv.Optional(CONF_TARGET_X): sensor.sensor_schema(
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                    icon=ICON_RULER,
                ),
                cv.Optional(CONF_TARGET_Y): sensor.sensor_schema(
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                    icon=ICON_RULER,
                ),
                cv.Optional(CONF_TARGET_SPEED): sensor.sensor_schema(
                    unit_of_measurement=UNIT_CENTIMETER_PER_SECOND,
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                    icon=ICON_SPEEDOMETR,
                ),
                cv.Optional(CONF_TARGET_RESOLUTION): sensor.sensor_schema(
                    unit_of_measurement=UNIT_CENTIMETER,
                    entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                    icon=ICON_RESOLUTION,
                ),                
            }
        )
        for x in range(1, 4)
    }
)


async def to_code(config):
    ld2450_component = await cg.get_variable(config[CONF_LD2450_ID])
    for x in range(1,4):
        if target_conf := config.get(f"target{x}"):
            if x_config := target_conf.get(CONF_TARGET_X):
                sens = await sensor.new_sensor(x_config)
                cg.add(ld2450_component.set_target_x_sensor(x, sens))
            if y_config := target_conf.get(CONF_TARGET_Y):
                sens = await sensor.new_sensor(y_config)
                cg.add(ld2450_component.set_target_y_sensor(x, sens))
            if speed_config := target_conf.get(CONF_TARGET_SPEED):
                sens = await sensor.new_sensor(speed_config)
                cg.add(ld2450_component.set_target_speed_sensor(x, sens))
            if resolution_config := target_conf.get(CONF_TARGET_RESOLUTION):
                sens = await sensor.new_sensor(resolution_config)
                cg.add(ld2450_component.set_target_resolution_sensor(x, sens))