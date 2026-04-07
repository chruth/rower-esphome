#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace joroto_rower {

class JorotoRower : public PollingComponent {
 public:
  void set_pin_step1(InternalGPIOPin *pin) { this->pin_step1_ = pin; }
  void set_pin_step2(InternalGPIOPin *pin) { this->pin_step2_ = pin; }

  void set_top_enter_threshold(int v) { this->top_enter_threshold_ = v; }
  void set_top_leave_threshold(int v) { this->top_leave_threshold_ = v; }
  void set_bottom_threshold(int v) { this->bottom_threshold_ = v; }

  void set_short_stroke_threshold(int v) { this->short_stroke_threshold_ = v; }
  void set_micro_stroke_threshold(int v) { this->micro_stroke_threshold_ = v; }

  void set_min_stroke_ms(uint32_t v) { this->min_stroke_ms_ = v; }
  void set_session_timeout_ms(uint32_t v) { this->session_timeout_ms_ = v; }
  void set_active_idle_ms(uint32_t v) { this->active_idle_ms_ = v; }

  void set_meters_per_valid_travel(float v) { this->meters_per_valid_travel_ = v; }
  void set_meters_per_short_travel(float v) { this->meters_per_short_travel_ = v; }
  void set_meters_per_micro_travel(float v) { this->meters_per_micro_travel_ = v; }

  void set_valid_strokes_sensor(sensor::Sensor *s) { this->valid_strokes_sensor_ = s; }
  void set_short_strokes_sensor(sensor::Sensor *s) { this->short_strokes_sensor_ = s; }
  void set_micro_strokes_sensor(sensor::Sensor *s) { this->micro_strokes_sensor_ = s; }
  void set_spm_sensor(sensor::Sensor *s) { this->spm_sensor_ = s; }
  void set_active_time_sensor(sensor::Sensor *s) { this->active_time_sensor_ = s; }
  void set_distance_sensor(sensor::Sensor *s) { this->distance_sensor_ = s; }
  void set_short_distance_sensor(sensor::Sensor *s) { this->short_distance_sensor_ = s; }
  void set_micro_distance_sensor(sensor::Sensor *s) { this->micro_distance_sensor_ = s; }
  void set_position_sensor(sensor::Sensor *s) { this->position_sensor_ = s; }

  void setup() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  static void IRAM_ATTR gpio_isr_static(void *arg);
  void IRAM_ATTR gpio_isr();

  uint8_t read_state_fast_();
  float compute_spm_();
  void register_valid_stroke_(uint32_t now_ms, int32_t travel);
  void register_short_stroke_(int32_t travel);
  void register_micro_stroke_(int32_t travel);
  void reset_cycle_(int32_t pos);

  enum StrokePhase {
    AT_TOP,
    MOVING_DOWN,
    WAITING_FOR_RETURN
  };

  // Pins
  InternalGPIOPin *pin_step1_{nullptr};
  InternalGPIOPin *pin_step2_{nullptr};

  // Quadrature
  volatile int32_t encoder_pos_{0};
  volatile uint8_t prev_state_{0};
  volatile bool pos_changed_{false};

  // Stroke FSM
  StrokePhase phase_{AT_TOP};
  int32_t cycle_min_pos_{0};
  int32_t cycle_max_pos_{0};

  // Config
  int top_enter_threshold_{-3};
  int top_leave_threshold_{-5};
  int bottom_threshold_{-10};

  int short_stroke_threshold_{8};
  int micro_stroke_threshold_{3};

  uint32_t min_stroke_ms_{700};
  uint32_t session_timeout_ms_{20000};
  uint32_t active_idle_ms_{600};

  float meters_per_valid_travel_{1.0f};
  float meters_per_short_travel_{1.0f};
  float meters_per_micro_travel_{1.0f};

  // Session / timing
  bool session_active_{false};
  bool movement_active_{false};
  uint32_t session_start_ms_{0};
  uint32_t last_movement_ms_{0};
  uint32_t movement_start_ms_{0};
  uint32_t active_time_ms_{0};
  uint32_t last_stroke_ms_{0};

  // Counters
  uint32_t valid_strokes_{0};
  uint32_t short_strokes_{0};
  uint32_t micro_strokes_{0};

  uint64_t valid_travel_sum_{0};
  uint64_t short_travel_sum_{0};
  uint64_t micro_travel_sum_{0};

  float distance_m_{0.0f};
  float short_distance_m_{0.0f};
  float micro_distance_m_{0.0f};

  // SPM
  uint32_t stroke_timestamps_[8] = {0};
  uint8_t stroke_ts_index_{0};

  // Sensors
  sensor::Sensor *valid_strokes_sensor_{nullptr};
  sensor::Sensor *short_strokes_sensor_{nullptr};
  sensor::Sensor *micro_strokes_sensor_{nullptr};
  sensor::Sensor *spm_sensor_{nullptr};
  sensor::Sensor *active_time_sensor_{nullptr};
  sensor::Sensor *distance_sensor_{nullptr};
  sensor::Sensor *short_distance_sensor_{nullptr};
  sensor::Sensor *micro_distance_sensor_{nullptr};
  sensor::Sensor *position_sensor_{nullptr};
};

}  // namespace joroto_rower
}  // namespace esphome
