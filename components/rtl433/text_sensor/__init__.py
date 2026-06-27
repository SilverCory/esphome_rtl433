import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

from .. import rtl433_ns, Rtl433Component

DEPENDENCIES = ["rtl433"]

Rtl433TextSensor = rtl433_ns.class_(
    "Rtl433TextSensor", text_sensor.TextSensor, rtl433_ns.class_("Rtl433Listener")
)

CONF_RTL433_ID = "rtl433_id"
CONF_SENSOR_DEVICE_MODEL = "sensor_device_model"
CONF_SENSOR_DEVICE_ID = "sensor_device_id"
CONF_SENSOR_FIELD = "sensor_field"

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(Rtl433TextSensor)
    .extend(
        {
            cv.GenerateID(CONF_RTL433_ID): cv.use_id(Rtl433Component),
            cv.Required(CONF_SENSOR_DEVICE_MODEL): cv.string,
            cv.Optional(CONF_SENSOR_DEVICE_ID): cv.string,
            cv.Required(CONF_SENSOR_FIELD): cv.string,
        }
    )
)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    parent = await cg.get_variable(config[CONF_RTL433_ID])
    cg.add(var.set_sensor_device_model(config[CONF_SENSOR_DEVICE_MODEL]))
    if CONF_SENSOR_DEVICE_ID in config:
        cg.add(var.set_sensor_device_id(config[CONF_SENSOR_DEVICE_ID]))
    cg.add(var.set_sensor_field(config[CONF_SENSOR_FIELD]))
    cg.add(parent.register_listener(var))
