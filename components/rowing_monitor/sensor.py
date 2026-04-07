import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_SENSOR,
    DEVICE_CLASS_DISTANCE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
)

CONF_VALID_STROKES = "valid_strokes"
CONF_SHORT_STROKES = "short_strokes"
CONF_MICRO_STROKES = "micro_strokes"
CONF_SPM = "spm"
CONF_ACTIVE_TIME = "active_time"
CONF_DISTANCE = "distance"
CONF_SHORT_DISTANCE = "short_distance"
CONF_MICRO_DISTANCE = "micro_distance"
CONF_AVG_VALID_TRAVEL = "avg_valid_travel"
CONF_AVG_SHORT_TRAVEL = "avg_short_travel"
CONF_AVG_MICRO_TRAVEL = "avg_micro_travel"

SENSOR_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_VALID_STROKES): sensor.sensor_schema(
            unit_of_measurement="",
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_STROKES): sensor.sensor_schema(
            unit_of_measurement="",
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_STROKES): sensor.sensor_schema(
            unit_of_measurement="",
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SPM): sensor.sensor_schema(
            unit_of_measurement="spm",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_ACTIVE_TIME): sensor.sensor_schema(
            unit_of_measurement="s",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
            unit_of_measurement="m",
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_DISTANCE): sensor.sensor_schema(
            unit_of_measurement="m",
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_DISTANCE): sensor.sensor_schema(
            unit_of_measurement="m",
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_AVG_VALID_TRAVEL): sensor.sensor_schema(
            unit_of_measurement="",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AVG_SHORT_TRAVEL): sensor.sensor_schema(
            unit_of_measurement="",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AVG_MICRO_TRAVEL): sensor.sensor_schema(
            unit_of_measurement="",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


async def register_sensors(var, config):
    nested = config.get(CONF_SENSOR, {})
    if CONF_VALID_STROKES in nested:
        sens = await sensor.new_sensor(nested[CONF_VALID_STROKES])
        cg.add(var.set_valid_strokes_sensor(sens))
    elif CONF_VALID_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_VALID_STROKES])
        cg.add(var.set_valid_strokes_sensor(sens))
    if CONF_SHORT_STROKES in nested:
        sens = await sensor.new_sensor(nested[CONF_SHORT_STROKES])
        cg.add(var.set_short_strokes_sensor(sens))
    elif CONF_SHORT_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_STROKES])
        cg.add(var.set_short_strokes_sensor(sens))
    if CONF_MICRO_STROKES in nested:
        sens = await sensor.new_sensor(nested[CONF_MICRO_STROKES])
        cg.add(var.set_micro_strokes_sensor(sens))
    elif CONF_MICRO_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_STROKES])
        cg.add(var.set_micro_strokes_sensor(sens))
    if CONF_SPM in nested:
        sens = await sensor.new_sensor(nested[CONF_SPM])
        cg.add(var.set_spm_sensor(sens))
    elif CONF_SPM in config:
        sens = await sensor.new_sensor(config[CONF_SPM])
        cg.add(var.set_spm_sensor(sens))
    if CONF_ACTIVE_TIME in nested:
        sens = await sensor.new_sensor(nested[CONF_ACTIVE_TIME])
        cg.add(var.set_active_time_sensor(sens))
    elif CONF_ACTIVE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_ACTIVE_TIME])
        cg.add(var.set_active_time_sensor(sens))
    if CONF_DISTANCE in nested:
        sens = await sensor.new_sensor(nested[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))
    elif CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))
    if CONF_SHORT_DISTANCE in nested:
        sens = await sensor.new_sensor(nested[CONF_SHORT_DISTANCE])
        cg.add(var.set_short_distance_sensor(sens))
    elif CONF_SHORT_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_DISTANCE])
        cg.add(var.set_short_distance_sensor(sens))
    if CONF_MICRO_DISTANCE in nested:
        sens = await sensor.new_sensor(nested[CONF_MICRO_DISTANCE])
        cg.add(var.set_micro_distance_sensor(sens))
    elif CONF_MICRO_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_DISTANCE])
        cg.add(var.set_micro_distance_sensor(sens))
    if CONF_AVG_VALID_TRAVEL in nested:
        sens = await sensor.new_sensor(nested[CONF_AVG_VALID_TRAVEL])
        cg.add(var.set_avg_valid_travel_sensor(sens))
    elif CONF_AVG_VALID_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_VALID_TRAVEL])
        cg.add(var.set_avg_valid_travel_sensor(sens))
    if CONF_AVG_SHORT_TRAVEL in nested:
        sens = await sensor.new_sensor(nested[CONF_AVG_SHORT_TRAVEL])
        cg.add(var.set_avg_short_travel_sensor(sens))
    elif CONF_AVG_SHORT_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_SHORT_TRAVEL])
        cg.add(var.set_avg_short_travel_sensor(sens))
    if CONF_AVG_MICRO_TRAVEL in nested:
        sens = await sensor.new_sensor(nested[CONF_AVG_MICRO_TRAVEL])
        cg.add(var.set_avg_micro_travel_sensor(sens))
    elif CONF_AVG_MICRO_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_MICRO_TRAVEL])
        cg.add(var.set_avg_micro_travel_sensor(sens))
