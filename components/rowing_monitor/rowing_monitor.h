#pragma once

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

  void set_top_enter_threshold(long v) { top_enter_threshold_ = v; }
  void set_top_leave_threshold(long v) { top_leave_threshold_ = v; }
  void set_bottom_threshold(long v) { bottom_threshold_ = v; }
  void set_short_threshold(long v) { short_threshold_ = v; }
  void set_micro_threshold(long v) { micro_threshold_ = v; }

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
  enum StrokePhase {
    AT_TOP,
    MOVING_DOWN,
    WAITING_FOR_RETURN
  };

  static void IRAM_ATTR gpio_isr(void *arg);
  void IRAM_ATTR handle_isr_();

  uint8_t read_state_fast_();
  float compute_spm_();
  void register_valid_stroke_(uint32_t now_ms, long travel);
  void register_short_stroke_(long travel);
  void register_micro_stroke_(long travel);
  void publish_state_();
  void publish_phase_();
  const char *phase_name_(StrokePhase p);

  GPIOPin *pin_step1_{nullptr};
  GPIOPin *pin_step2_{nullptr};

  volatile long encoder_pos_{0};
  volatile uint8_t prev_state_{0};
  volatile bool pos_changed_{false};

  const int8_t quad_table_[16] = {
      0, -1, +1,  0,
      +1, 0,  0, -1,
      -1, 0,  0, +1,
      0, +1, -1,  0};

  long top_enter_threshold_{-3};
  long top_leave_threshold_{-5};
  long bottom_threshold_{-10};
  long short_threshold_{-8};
  long micro_threshold_{-4};

  uint32_t min_stroke_ms_{700};
  uint32_t session_timeout_ms_{20000};
  uint32_t active_idle_ms_{1500};
  float meters_per_travel_{0.6667f};

  StrokePhase phase_{AT_TOP};
  long cycle_min_pos_{0};
  long cycle_max_pos_{0};

  bool session_active_{false};
  uint32_t session_start_ms_{0};
  uint32_t last_movement_ms_{0};
  uint32_t last_active_sample_ms_{0};
  uint32_t last_stroke_ms_{0};
  uint32_t last_publish_ms_{0};

  float active_time_s_{0.0f};

  uint32_t valid_strokes_{0};
  uint32_t short_strokes_{0};
  uint32_t micro_strokes_{0};

  float distance_m_{0.0f};
  float short_distance_m_{0.0f};
  float micro_distance_m_{0.0f};

  long valid_travel_sum_{0};
  long short_travel_sum_{0};
  long micro_travel_sum_{0};

  uint32_t stroke_timestamps_[8] = {0};
  int stroke_ts_index_{0};

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
