#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace rowing_monitor {

enum StrokePhase {
  AT_TOP = 0,
  MOVING_DOWN = 1,
  WAITING_FOR_RETURN = 2
};

class RowingMonitorComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_pin_step1(InternalGPIOPin *pin) { pin_step1_ = pin; }
  void set_pin_step2(InternalGPIOPin *pin) { pin_step2_ = pin; }

  void set_top_enter_threshold(int v) { top_enter_threshold_ = v; }
  void set_top_leave_threshold(int v) { top_leave_threshold_ = v; }
  void set_bottom_threshold(int v) { bottom_threshold_ = v; }
  void set_short_threshold(int v) { short_threshold_ = v; }
  void set_micro_threshold(int v) { micro_threshold_ = v; }

  void set_min_stroke_ms(uint32_t v) { min_stroke_ms_ = v; }
  void set_session_timeout_ms(uint32_t v) { session_timeout_ms_ = v; }
  void set_active_idle_ms(uint32_t v) { active_idle_ms_ = v; }
  void set_meters_per_travel(float v) { meters_per_travel_ = v; }

  void set_strokes_sensor(sensor::Sensor *s) { strokes_sensor_ = s; }
  void set_short_strokes_sensor(sensor::Sensor *s) { short_strokes_sensor_ = s; }
  void set_micro_strokes_sensor(sensor::Sensor *s) { micro_strokes_sensor_ = s; }
  void set_spm_sensor(sensor::Sensor *s) { spm_sensor_ = s; }
  void set_distance_sensor(sensor::Sensor *s) { distance_sensor_ = s; }
  void set_short_distance_sensor(sensor::Sensor *s) { short_distance_sensor_ = s; }
  void set_micro_distance_sensor(sensor::Sensor *s) { micro_distance_sensor_ = s; }
  void set_active_time_sensor(sensor::Sensor *s) { active_time_sensor_ = s; }
  void set_avg_valid_travel_sensor(sensor::Sensor *s) { avg_valid_travel_sensor_ = s; }
  void set_avg_short_travel_sensor(sensor::Sensor *s) { avg_short_travel_sensor_ = s; }
  void set_avg_micro_travel_sensor(sensor::Sensor *s) { avg_micro_travel_sensor_ = s; }
  void set_phase_sensor(text_sensor::TextSensor *s) { phase_sensor_ = s; }

 protected:
  static void gpio_intr(RowingMonitorComponent *arg);

  uint8_t read_state_fast_();
  void handle_encoder_isr_();
  float compute_spm_();
  void publish_all_();
  void register_valid_stroke_(uint32_t now_ms, long travel);
  void register_short_stroke_(long travel);
  void register_micro_stroke_(long travel);
  const char *phase_name_(StrokePhase p);

  InternalGPIOPin *pin_step1_{nullptr};
  InternalGPIOPin *pin_step2_{nullptr};

  volatile long encoder_pos_{0};
  volatile uint8_t prev_state_{0};
  volatile bool pos_changed_{false};

  StrokePhase phase_{AT_TOP};
  long cycle_min_pos_{0};
  long cycle_max_pos_{0};

  uint32_t last_movement_ms_{0};
  uint32_t last_stroke_ms_{0};
  uint32_t session_start_ms_{0};
  bool session_active_{false};

  float active_time_s_{0.0f};
  uint32_t last_active_accum_ms_{0};

  uint32_t valid_strokes_{0};
  uint32_t short_strokes_{0};
  uint32_t micro_strokes_{0};

  float distance_m_{0.0f};
  float short_distance_m_{0.0f};
  float micro_distance_m_{0.0f};

  float sum_valid_travel_{0.0f};
  float sum_short_travel_{0.0f};
  float sum_micro_travel_{0.0f};

  uint32_t stroke_timestamps_[8] = {0};
  int stroke_ts_index_{0};

  int top_enter_threshold_{-3};
  int top_leave_threshold_{-5};
  int bottom_threshold_{-10};
  int short_threshold_{-8};
  int micro_threshold_{-4};

  uint32_t min_stroke_ms_{700};
  uint32_t session_timeout_ms_{20000};
  uint32_t active_idle_ms_{1500};

  float meters_per_travel_{0.5f};

  sensor::Sensor *strokes_sensor_{nullptr};
  sensor::Sensor *short_strokes_sensor_{nullptr};
  sensor::Sensor *micro_strokes_sensor_{nullptr};
  sensor::Sensor *spm_sensor_{nullptr};
  sensor::Sensor *distance_sensor_{nullptr};
  sensor::Sensor *short_distance_sensor_{nullptr};
  sensor::Sensor *micro_distance_sensor_{nullptr};
  sensor::Sensor *active_time_sensor_{nullptr};
  sensor::Sensor *avg_valid_travel_sensor_{nullptr};
  sensor::Sensor *avg_short_travel_sensor_{nullptr};
  sensor::Sensor *avg_micro_travel_sensor_{nullptr};
  text_sensor::TextSensor *phase_sensor_{nullptr};
};

}  // namespace rowing_monitor
}  // namespace esphome
