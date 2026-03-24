import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, text_sensor
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ICON,
    CONF_ACCURACY_DECIMALS,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_DURATION,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_METER,
    UNIT_SECOND,
)

from esphome import pins
from . import rowing_monitor_ns, RowingMonitorComponent

CONF_PIN_STEP1 = "pin_step1"
CONF_PIN_STEP2 = "pin_step2"

CONF_STROKES = "strokes"
CONF_SHORT_STROKES = "short_strokes"
CONF_MICRO_STROKES = "micro_strokes"
CONF_SPM = "spm"
CONF_DISTANCE = "distance"
CONF_SHORT_DISTANCE = "short_distance"
CONF_MICRO_DISTANCE = "micro_distance"
CONF_ACTIVE_TIME = "active_time"
CONF_AVG_VALID_TRAVEL = "avg_valid_travel"
CONF_AVG_SHORT_TRAVEL = "avg_short_travel"
CONF_AVG_MICRO_TRAVEL = "avg_micro_travel"
CONF_PHASE = "phase"

CONF_TOP_ENTER_THRESHOLD = "top_enter_threshold"
CONF_TOP_LEAVE_THRESHOLD = "top_leave_threshold"
CONF_BOTTOM_THRESHOLD = "bottom_threshold"
CONF_SHORT_THRESHOLD = "short_threshold"
CONF_MICRO_THRESHOLD = "micro_threshold"
CONF_MIN_STROKE_MS = "min_stroke_ms"
CONF_SESSION_TIMEOUT_MS = "session_timeout_ms"
CONF_ACTIVE_IDLE_MS = "active_idle_ms"
CONF_METERS_PER_TRAVEL = "meters_per_travel"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RowingMonitorComponent),
        cv.Required(CONF_PIN_STEP1): pins.internal_gpio_input_pin_schema,
        cv.Required(CONF_PIN_STEP2): pins.internal_gpio_input_pin_schema,

        cv.Optional(CONF_TOP_ENTER_THRESHOLD, default=-3): cv.int_,
        cv.Optional(CONF_TOP_LEAVE_THRESHOLD, default=-5): cv.int_,
        cv.Optional(CONF_BOTTOM_THRESHOLD, default=-10): cv.int_,
        cv.Optional(CONF_SHORT_THRESHOLD, default=-8): cv.int_,
        cv.Optional(CONF_MICRO_THRESHOLD, default=-4): cv.int_,
        cv.Optional(CONF_MIN_STROKE_MS, default=700): cv.positive_int,
        cv.Optional(CONF_SESSION_TIMEOUT_MS, default=20000): cv.positive_int,
        cv.Optional(CONF_ACTIVE_IDLE_MS, default=1500): cv.positive_int,
        cv.Optional(CONF_METERS_PER_TRAVEL, default=0.5): cv.float_,

        cv.Optional(CONF_STROKES): sensor.sensor_schema(
            unit_of_measurement="strokes",
            icon="mdi:counter",
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_STROKES): sensor.sensor_schema(
            unit_of_measurement="strokes",
            icon="mdi:counter",
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_STROKES): sensor.sensor_schema(
            unit_of_measurement="strokes",
            icon="mdi:counter",
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SPM): sensor.sensor_schema(
            unit_of_measurement="spm",
            icon="mdi:rowing",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            icon="mdi:map-marker-distance",
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            icon="mdi:map-marker-distance",
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            icon="mdi:map-marker-distance",
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_ACTIVE_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_SECOND,
            icon="mdi:timer-play-outline",
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_DURATION,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AVG_VALID_TRAVEL): sensor.sensor_schema(
            unit_of_measurement="counts",
            icon="mdi:arrow-expand-vertical",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AVG_SHORT_TRAVEL): sensor.sensor_schema(
            unit_of_measurement="counts",
            icon="mdi:arrow-expand-vertical",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AVG_MICRO_TRAVEL): sensor.sensor_schema(
            unit_of_measurement="counts",
            icon="mdi:arrow-expand-vertical",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PHASE): text_sensor.text_sensor_schema(
            icon="mdi:state-machine"
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    pin1 = await cg.gpio_pin_expression(config[CONF_PIN_STEP1])
    cg.add(var.set_pin_step1(pin1))

    pin2 = await cg.gpio_pin_expression(config[CONF_PIN_STEP2])
    cg.add(var.set_pin_step2(pin2))

    cg.add(var.set_top_enter_threshold(config[CONF_TOP_ENTER_THRESHOLD]))
    cg.add(var.set_top_leave_threshold(config[CONF_TOP_LEAVE_THRESHOLD]))
    cg.add(var.set_bottom_threshold(config[CONF_BOTTOM_THRESHOLD]))
    cg.add(var.set_short_threshold(config[CONF_SHORT_THRESHOLD]))
    cg.add(var.set_micro_threshold(config[CONF_MICRO_THRESHOLD]))
    cg.add(var.set_min_stroke_ms(config[CONF_MIN_STROKE_MS]))
    cg.add(var.set_session_timeout_ms(config[CONF_SESSION_TIMEOUT_MS]))
    cg.add(var.set_active_idle_ms(config[CONF_ACTIVE_IDLE_MS]))
    cg.add(var.set_meters_per_travel(config[CONF_METERS_PER_TRAVEL]))

    if CONF_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_STROKES])
        cg.add(var.set_strokes_sensor(sens))

    if CONF_SHORT_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_STROKES])
        cg.add(var.set_short_strokes_sensor(sens))

    if CONF_MICRO_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_STROKES])
        cg.add(var.set_micro_strokes_sensor(sens))

    if CONF_SPM in config:
        sens = await sensor.new_sensor(config[CONF_SPM])
        cg.add(var.set_spm_sensor(sens))

    if CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))

    if CONF_SHORT_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_DISTANCE])
        cg.add(var.set_short_distance_sensor(sens))

    if CONF_MICRO_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_DISTANCE])
        cg.add(var.set_micro_distance_sensor(sens))

    if CONF_ACTIVE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_ACTIVE_TIME])
        cg.add(var.set_active_time_sensor(sens))

    if CONF_AVG_VALID_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_VALID_TRAVEL])
        cg.add(var.set_avg_valid_travel_sensor(sens))

    if CONF_AVG_SHORT_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_SHORT_TRAVEL])
        cg.add(var.set_avg_short_travel_sensor(sens))

    if CONF_AVG_MICRO_TRAVEL in config:
        sens = await sensor.new_sensor(config[CONF_AVG_MICRO_TRAVEL])
        cg.add(var.set_avg_micro_travel_sensor(sens))

    if CONF_PHASE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_PHASE])
        cg.add(var.set_phase_sensor(sens))
