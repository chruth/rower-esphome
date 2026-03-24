import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_DISTANCE,
    CONF_NAME,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_EMPTY,
    UNIT_METER,
    UNIT_SECOND,
)

from . import RowingMonitor

CONF_ROWING_MONITOR_ID = "rowing_monitor_id"
CONF_VALID_STROKES = "valid_strokes"
CONF_SHORT_STROKES = "short_strokes"
CONF_MICRO_STROKES = "micro_strokes"
CONF_SPM = "spm"
CONF_ACTIVE_TIME = "active_time"
CONF_SHORT_DISTANCE = "short_distance"
CONF_MICRO_DISTANCE = "micro_distance"
CONF_AVG_VALID_TRAVEL = "avg_valid_travel"
CONF_AVG_SHORT_TRAVEL = "avg_short_travel"
CONF_AVG_MICRO_TRAVEL = "avg_micro_travel"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ROWING_MONITOR_ID): cv.use_id(RowingMonitor),
        cv.Optional(CONF_VALID_STROKES): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_STROKES): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_STROKES): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SPM): sensor.sensor_schema(
            unit_of_measurement="spm",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_ACTIVE_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_SECOND,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=2,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=2,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=2,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_AVG_VALID_TRAVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AVG_SHORT_TRAVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AVG_MICRO_TRAVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ROWING_MONITOR_ID])

    if CONF_VALID_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_VALID_STROKES])
        cg.add(parent.set_valid_strokes_sensor(sens))

    if CONF_SHORT_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_STROKES])
        cg.add(parent.set_short_strokes_sensor(sens))

    if CONF_MICRO_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_STROKES])
        cg.add(parent.set_micro_strokes_sensor(sens))

    if CONF_SPM in config:
        sens = await sensor.new_sensor(config[CONF_SPM])
        cg.add(parent.set_spm_sensor(sens))

    if CONF_ACTIVE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_ACTIVE_TIME])
        cg.add(parent.set_active_time_sensor(sens))

    if CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(parent.set_distance_sensor(sens))

    if CONF_SHORT_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_DISTANCE])
        cg.add(parent.set_short_distance_sensor(sens))

    if CONF_MICRO_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_DISTANCE])
        cg.add(parent.set_micro_distance_sensor(sens))

    if CONF_AVG_VALID_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_VALID_TRAVEL])
        cg.add(parent.set_avg_valid_travel_sensor(sens))

    if CONF_AVG_SHORT_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_SHORT_TRAVEL])
        cg.add(parent.set_avg_short_travel_sensor(sens))

    if CONF_AVG_MICRO_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_MICRO_TRAVEL])
        cg.add(parent.set_avg_micro_travel_sensor(sens))
