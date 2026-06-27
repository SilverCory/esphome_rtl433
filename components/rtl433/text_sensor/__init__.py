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
CONF_DEVICE_MODEL = "device_model"
CONF_DEVICE_ID = "device_id"
CONF_FIELD = "field"

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(Rtl433TextSensor)
    .extend(
        {
            cv.GenerateID(CONF_RTL433_ID): cv.use_id(Rtl433Component),
            cv.Required(CONF_DEVICE_MODEL): cv.string,
            cv.Optional(CONF_DEVICE_ID): cv.string,
            cv.Required(CONF_FIELD): cv.string,
        }
    )
)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    parent = await cg.get_variable(config[CONF_RTL433_ID])
    cg.add(var.set_device_model(config[CONF_DEVICE_MODEL]))
    if CONF_DEVICE_ID in config:
        cg.add(var.set_device_id(config[CONF_DEVICE_ID]))
    cg.add(var.set_field(config[CONF_FIELD]))
    cg.add(parent.register_listener(var))
