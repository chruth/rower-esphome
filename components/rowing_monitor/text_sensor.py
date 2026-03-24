import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import RowingMonitor

CONF_ROWING_MONITOR_ID = "rowing_monitor_id"
CONF_PHASE = "phase"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ROWING_MONITOR_ID): cv.use_id(RowingMonitor),
        cv.Optional(CONF_PHASE): text_sensor.text_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ROWING_MONITOR_ID])

    if CONF_PHASE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_PHASE])
        cg.add(parent.set_phase_text_sensor(sens))
