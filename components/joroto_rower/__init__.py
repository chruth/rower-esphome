import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    UNIT_MINUTE,
    UNIT_SECOND,
    UNIT_METER,
    ICON_COUNTER,
    ICON_TIMER,
    ICON_RULER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    DEVICE_CLASS_DISTANCE,
)

joroto_rower_ns = cg.esphome_ns.namespace("joroto_rower")
JorotoRower = joroto_rower_ns.class_("JorotoRower", cg.PollingComponent)

CONF_PIN_STEP1 = "pin_step1"
CONF_PIN_STEP2 = "pin_step2"

CONF_TOP_ENTER_THRESHOLD = "top_enter_threshold"
CONF_TOP_LEAVE_THRESHOLD = "top_leave_threshold"
CONF_BOTTOM_THRESHOLD = "bottom_threshold"

CONF_SHORT_STROKE_THRESHOLD = "short_stroke_threshold"
CONF_MICRO_STROKE_THRESHOLD = "micro_stroke_threshold"

CONF_MIN_STROKE_MS = "min_stroke_ms"
CONF_SESSION_TIMEOUT_MS = "session_timeout_ms"
CONF_ACTIVE_IDLE_MS = "active_idle_ms"

CONF_METERS_PER_VALID_TRAVEL = "meters_per_valid_travel"
CONF_METERS_PER_SHORT_TRAVEL = "meters_per_short_travel"
CONF_METERS_PER_MICRO_TRAVEL = "meters_per_micro_travel"

CONF_VALID_STROKES = "valid_strokes"
CONF_SHORT_STROKES = "short_strokes"
CONF_MICRO_STROKES = "micro_strokes"
CONF_SPM = "spm"
CONF_ACTIVE_TIME = "active_time"
CONF_DISTANCE = "distance"
CONF_SHORT_DISTANCE = "short_distance"
CONF_MICRO_DISTANCE = "micro_distance"
CONF_POSITION = "position"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(JorotoRower),

        cv.Required(CONF_PIN_STEP1): pins.internal_gpio_input_pin_schema,
        cv.Required(CONF_PIN_STEP2): pins.internal_gpio_input_pin_schema,

        cv.Optional(CONF_TOP_ENTER_THRESHOLD, default=-3): cv.int_,
        cv.Optional(CONF_TOP_LEAVE_THRESHOLD, default=-5): cv.int_,
        cv.Optional(CONF_BOTTOM_THRESHOLD, default=-10): cv.int_,

        cv.Optional(CONF_SHORT_STROKE_THRESHOLD, default=8): cv.int_range(min=0),
        cv.Optional(CONF_MICRO_STROKE_THRESHOLD, default=3): cv.int_range(min=0),

        cv.Optional(CONF_MIN_STROKE_MS, default=700): cv.positive_int,
        cv.Optional(CONF_SESSION_TIMEOUT_MS, default=20000): cv.positive_int,
        cv.Optional(CONF_ACTIVE_IDLE_MS, default=600): cv.positive_int,

        cv.Optional(CONF_METERS_PER_VALID_TRAVEL, default=1.0): cv.float_,
        cv.Optional(CONF_METERS_PER_SHORT_TRAVEL, default=1.0): cv.float_,
        cv.Optional(CONF_METERS_PER_MICRO_TRAVEL, default=1.0): cv.float_,

        cv.Optional(CONF_VALID_STROKES): sensor.sensor_schema(
            accuracy_decimals=0,
            icon=ICON_COUNTER,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_STROKES): sensor.sensor_schema(
            accuracy_decimals=0,
            icon=ICON_COUNTER,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_STROKES): sensor.sensor_schema(
            accuracy_decimals=0,
            icon=ICON_COUNTER,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SPM): sensor.sensor_schema(
            unit_of_measurement=UNIT_MINUTE,
            accuracy_decimals=1,
            icon=ICON_TIMER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_ACTIVE_TIME): sensor.sensor_schema(
            unit_of_measurement=UNIT_SECOND,
            accuracy_decimals=1,
            icon=ICON_TIMER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_DISTANCE,
            icon=ICON_RULER,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_SHORT_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_DISTANCE,
            icon=ICON_RULER,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_MICRO_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_METER,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_DISTANCE,
            icon=ICON_RULER,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_POSITION): sensor.sensor_schema(
            accuracy_decimals=0,
            icon=ICON_RULER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(cv.polling_component_schema("200ms"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    pin1 = await cg.gpio_pin_expression(config[CONF_PIN_STEP1])
    pin2 = await cg.gpio_pin_expression(config[CONF_PIN_STEP2])
    cg.add(var.set_pin_step1(pin1))
    cg.add(var.set_pin_step2(pin2))

    cg.add(var.set_top_enter_threshold(config[CONF_TOP_ENTER_THRESHOLD]))
    cg.add(var.set_top_leave_threshold(config[CONF_TOP_LEAVE_THRESHOLD]))
    cg.add(var.set_bottom_threshold(config[CONF_BOTTOM_THRESHOLD]))

    cg.add(var.set_short_stroke_threshold(config[CONF_SHORT_STROKE_THRESHOLD]))
    cg.add(var.set_micro_stroke_threshold(config[CONF_MICRO_STROKE_THRESHOLD]))

    cg.add(var.set_min_stroke_ms(config[CONF_MIN_STROKE_MS]))
    cg.add(var.set_session_timeout_ms(config[CONF_SESSION_TIMEOUT_MS]))
    cg.add(var.set_active_idle_ms(config[CONF_ACTIVE_IDLE_MS]))

    cg.add(var.set_meters_per_valid_travel(config[CONF_METERS_PER_VALID_TRAVEL]))
    cg.add(var.set_meters_per_short_travel(config[CONF_METERS_PER_SHORT_TRAVEL]))
    cg.add(var.set_meters_per_micro_travel(config[CONF_METERS_PER_MICRO_TRAVEL]))

    if CONF_VALID_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_VALID_STROKES])
        cg.add(var.set_valid_strokes_sensor(sens))
    if CONF_SHORT_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_STROKES])
        cg.add(var.set_short_strokes_sensor(sens))
    if CONF_MICRO_STROKES in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_STROKES])
        cg.add(var.set_micro_strokes_sensor(sens))
    if CONF_SPM in config:
        sens = await sensor.new_sensor(config[CONF_SPM])
        cg.add(var.set_spm_sensor(sens))
    if CONF_ACTIVE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_ACTIVE_TIME])
        cg.add(var.set_active_time_sensor(sens))
    if CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))
    if CONF_SHORT_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_SHORT_DISTANCE])
        cg.add(var.set_short_distance_sensor(sens))
    if CONF_MICRO_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_MICRO_DISTANCE])
        cg.add(var.set_micro_distance_sensor(sens))
    if CONF_POSITION in config:
        sens = await sensor.new_sensor(config[CONF_POSITION])
        cg.add(var.set_position_sensor(sens))
