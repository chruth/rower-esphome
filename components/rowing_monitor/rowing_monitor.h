#pragma once

#include <cstdint>

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace rowing_monitor {

class RowingMonitor : public Component {
 public:
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_pin_step1(GPIOPin *pin) { pin_step1_ = pin; }
  void set_pin_step2(GPIOPin *pin) { pin_step2_ = pin; }

  void set_top_enter_threshold(int32_t v) { top_enter_threshold_ = v; }
  void set_top_leave_threshold(int32_t v) { top_leave_threshold_ = v; }
  void set_bottom_threshold(int32_t v) { bottom_threshold_ = v; }
  void set_short_threshold(int32_t v) { short_threshold_ = v; }
  void set_micro_threshold(int32_t v) { micro_threshold_ = v; }

  void set_min_stroke_ms(uint32_t v) { min_stroke_ms_ = v; }
  void set_session_timeout_ms(uint32_t v) { session_timeout_ms_ = v; }
  void set_active_idle_ms(uint32_t v) { active_idle_ms_ = v; }
  void set_meters_per_travel(float v) { meters_per_travel_ = v; }

  void set_valid_strokes_sensor(sensor::Sensor *s) { valid_strokes_sensor_ = s; }
  void set_short_strokes_sensor(sensor::Sensor *s) { short_strokes_sensor_ = s; }
  void set_micro_strokes_sensor(sensor::Sensor *s) { micro_strokes_sensor_ = s; }
  void set_spm_sensor(sensor::Sensor *s) { spm_sensor_ = s; }
  void set_active_time_sensor(sensor::Sensor *s) { active_time_sensor_ = s; }
  void set_distance_sensor(sensor::Sensor *s) { distance_sensor_ = s; }
  void set_short_distance_sensor(sensor::Sensor *s) { short_distance_sensor_ = s; }
  void set_micro_distance_sensor(sensor::Sensor *s) { micro_distance_sensor_ = s; }
  void set_avg_valid_travel_sensor(sensor::Sensor *s) { avg_valid_travel_sensor_ = s; }
  void set_avg_short_travel_sensor(sensor::Sensor *s) { avg_short_travel_sensor_ = s; }
  void set_avg_micro_travel_sensor(sensor::Sensor *s) { avg_micro_travel_sensor_ = s; }

  void set_phase_text_sensor(text_sensor::TextSensor *s) { phase_text_sensor_ = s; }

 protected:
  enum class Phase : uint8_t {
    AT_TOP = 0,
    MOVING_DOWN = 1,
    WAITING_FOR_RETURN = 2,
  };

  enum class StrokeKind : uint8_t {
    VALID = 0,
    SHORT = 1,
    MICRO = 2,
  };

  static void IRAM_ATTR gpio_isr(void *arg);
  void IRAM_ATTR handle_isr_();

  uint8_t read_state_fast_() const;
  void update_phase_(Phase phase);
  const char *phase_name_(Phase phase) const;

  void on_position_update_(uint32_t now_ms, int32_t pos);
  void on_motion_(uint32_t now_ms);

  void maybe_end_session_(uint32_t now_ms);
  void sample_active_time_(uint32_t now_ms);

  void reset_cycle_tracking_(int32_t pos);
  void update_cycle_tracking_(int32_t pos);
  void complete_cycle_(uint32_t now_ms, int32_t pos);

  void register_stroke_(StrokeKind kind, uint32_t now_ms, int32_t travel, int32_t depth);
  void add_valid_stroke_timestamp_(uint32_t now_ms);
  float compute_spm_() const;

  void publish_phase_();
  void publish_state_();
  void log_periodic_debug_(uint32_t now_ms) const;

  GPIOPin *pin_step1_{nullptr};
  GPIOPin *pin_step2_{nullptr};

  volatile int32_t encoder_pos_{0};
  volatile uint8_t prev_state_{0};
  volatile bool pos_changed_{false};

  static constexpr int8_t QUAD_TABLE_[16] = {
      0, -1, +1, 0,  //
      +1, 0, 0, -1,  //
      -1, 0, 0, +1,  //
      0, +1, -1, 0,
  };

  int32_t top_enter_threshold_{-3};
  int32_t top_leave_threshold_{-5};
  int32_t bottom_threshold_{-10};
  int32_t short_threshold_{-8};
  int32_t micro_threshold_{-4};

  uint32_t min_stroke_ms_{700};
  uint32_t session_timeout_ms_{20000};
  uint32_t active_idle_ms_{1500};
  float meters_per_travel_{0.6667f};

  Phase phase_{Phase::AT_TOP};
  Phase published_phase_{Phase::AT_TOP};
  int32_t cycle_min_pos_{0};
  int32_t cycle_max_pos_{0};
  bool cycle_reached_bottom_{false};

  bool session_active_{false};
  uint32_t session_start_ms_{0};
  uint32_t last_motion_ms_{0};
  uint32_t last_active_sample_ms_{0};
  uint32_t last_valid_stroke_ms_{0};

  uint64_t active_time_ms_{0};

  uint32_t last_publish_ms_{0};
  uint32_t last_debug_ms_{0};

  uint32_t valid_strokes_{0};
  uint32_t short_strokes_{0};
  uint32_t micro_strokes_{0};

  float valid_distance_m_{0.0f};
  float short_distance_m_{0.0f};
  float micro_distance_m_{0.0f};

  int64_t valid_travel_sum_{0};
  int64_t short_travel_sum_{0};
  int64_t micro_travel_sum_{0};

  static constexpr uint8_t STROKE_TS_CAPACITY_ = 8;
  uint32_t valid_stroke_ts_[STROKE_TS_CAPACITY_] = {0};
  uint8_t valid_stroke_ts_head_{0};
  uint8_t valid_stroke_ts_count_{0};

  sensor::Sensor *valid_strokes_sensor_{nullptr};
  sensor::Sensor *short_strokes_sensor_{nullptr};
  sensor::Sensor *micro_strokes_sensor_{nullptr};
  sensor::Sensor *spm_sensor_{nullptr};
  sensor::Sensor *active_time_sensor_{nullptr};
  sensor::Sensor *distance_sensor_{nullptr};
  sensor::Sensor *short_distance_sensor_{nullptr};
  sensor::Sensor *micro_distance_sensor_{nullptr};
  sensor::Sensor *avg_valid_travel_sensor_{nullptr};
  sensor::Sensor *avg_short_travel_sensor_{nullptr};
  sensor::Sensor *avg_micro_travel_sensor_{nullptr};
  text_sensor::TextSensor *phase_text_sensor_{nullptr};
};

}  // namespace rowing_monitor
}  // namespace esphome

