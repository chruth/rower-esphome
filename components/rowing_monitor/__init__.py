import esphome.codegen as cg
import esphome.config_validation as cv

AUTO_LOAD = ["sensor"]
MULTI_CONF = False

rowing_monitor_ns = cg.esphome_ns.namespace("rowing_monitor")
RowingMonitor = rowing_monitor_ns.class_("RowingMonitor", cg.Component)

CONF_PIN_STEP1 = "pin_step1"
CONF_PIN_STEP2 = "pin_step2"
CONF_TOP_ENTER_THRESHOLD = "top_enter_threshold"
CONF_TOP_LEAVE_THRESHOLD = "top_leave_threshold"
CONF_BOTTOM_THRESHOLD = "bottom_threshold"
CONF_SHORT_THRESHOLD = "short_threshold"
CONF_MICRO_THRESHOLD = "micro_threshold"
CONF_MIN_STROKE_MS = "min_stroke_ms"
CONF_SESSION_TIMEOUT_MS = "session_timeout_ms"
CONF_ACTIVE_IDLE_MS = "active_idle_ms"
CONF_METERS_PER_TRAVEL = "meters_per_travel"
CONF_ROWING_MONITOR_ID = "rowing_monitor_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RowingMonitor),
        cv.Required(CONF_PIN_STEP1): cv.int_,
        cv.Required(CONF_PIN_STEP2): cv.int_,
        cv.Optional(CONF_TOP_ENTER_THRESHOLD, default=-3): cv.int_,
        cv.Optional(CONF_TOP_LEAVE_THRESHOLD, default=-5): cv.int_,
        cv.Optional(CONF_BOTTOM_THRESHOLD, default=-10): cv.int_,
        cv.Optional(CONF_SHORT_THRESHOLD, default=-8): cv.int_,
        cv.Optional(CONF_MICRO_THRESHOLD, default=-4): cv.int_,
        cv.Optional(CONF_MIN_STROKE_MS, default=700): cv.positive_int,
        cv.Optional(CONF_SESSION_TIMEOUT_MS, default=20000): cv.positive_int,
        cv.Optional(CONF_ACTIVE_IDLE_MS, default=1500): cv.positive_int,
        cv.Optional(CONF_METERS_PER_TRAVEL, default=0.4202): cv.float_,
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config["id"])
    await cg.register_component(var, config)

    cg.add(var.set_pin_step1(config[CONF_PIN_STEP1]))
    cg.add(var.set_pin_step2(config[CONF_PIN_STEP2]))
    cg.add(var.set_top_enter_threshold(config[CONF_TOP_ENTER_THRESHOLD]))
    cg.add(var.set_top_leave_threshold(config[CONF_TOP_LEAVE_THRESHOLD]))
    cg.add(var.set_bottom_threshold(config[CONF_BOTTOM_THRESHOLD]))
    cg.add(var.set_short_threshold(config[CONF_SHORT_THRESHOLD]))
    cg.add(var.set_micro_threshold(config[CONF_MICRO_THRESHOLD]))
    cg.add(var.set_min_stroke_ms(config[CONF_MIN_STROKE_MS]))
    cg.add(var.set_session_timeout_ms(config[CONF_SESSION_TIMEOUT_MS]))
    cg.add(var.set_active_idle_ms(config[CONF_ACTIVE_IDLE_MS]))
    cg.add(var.set_meters_per_travel(config[CONF_METERS_PER_TRAVEL]))
