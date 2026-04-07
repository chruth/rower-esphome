import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

CONF_TEXT_SENSOR = "text_sensor"
CONF_PHASE = "phase"

TEXT_SENSOR_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_PHASE): text_sensor.text_sensor_schema(),
    }
)


async def register_text_sensors(var, config):
    nested = config.get(CONF_TEXT_SENSOR, {})
    if CONF_PHASE in nested:
        sens = await text_sensor.new_text_sensor(nested[CONF_PHASE])
        cg.add(var.set_phase_text_sensor(sens))
    elif CONF_PHASE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_PHASE])
        cg.add(var.set_phase_text_sensor(sens))
