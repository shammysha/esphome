import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, vl5310x
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_METER,
    ICON_ARROW_EXPAND_VERTICAL,
    CONF_ADDRESS,
    CONF_TIMEOUT,
    CONF_ENABLE_PIN,
)
from esphome import pins

DEPENDENCIES = ["i2c"]

vl53l0x_m_ns = cg.esphome_ns.namespace("vl53l0x_m")
VL53L0XMSensor = vl53l0x_m_ns.class_(
    "VL53L0XMSensor", vl5310x, cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        VL53L0XMSensor
    )
    .extend(sensor.VL53L0XSensor)
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    cg.add(var.set_signal_rate_limit(config[CONF_SIGNAL_RATE_LIMIT]))
    cg.add(var.set_long_range(config[CONF_LONG_RANGE]))
    cg.add(var.set_timeout_us(config[CONF_TIMEOUT]))

    if CONF_ENABLE_PIN in config:
        enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable))

    await i2c.register_i2c_device(var, config)
