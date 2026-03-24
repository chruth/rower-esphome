#include "rowing_monitor.h"
#include "esphome/core/log.h"
#include "driver/gpio.h"

namespace esphome {
namespace rowing_monitor {

static const char *const TAG = "rowing_monitor";

void IRAM_ATTR RowingMonitor::gpio_isr(void *arg) {
  auto *self = static_cast<RowingMonitor *>(arg);
  self->handle_isr_();
}

uint8_t IRAM_ATTR RowingMonitor::read_state_fast_() {
  uint8_t a = gpio_get_level((gpio_num_t) pin_step1_->get_pin());
  uint8_t b = gpio_get_level((gpio_num_t) pin_step2_->get_pin());
  return (a << 1) | b;
}

void IRAM_ATTR RowingMonitor::handle_isr_() {
  uint8_t new_state = ((uint8_t) gpio_get_level((gpio_num_t) pin_step1_->get_pin()) << 1) |
                      (uint8_t) gpio_get_level((gpio_num_t) pin_step2_->get_pin());
  uint8_t index = (prev_state_ << 2) | new_state;
  int8_t delta = quad_table_[index];

  if (delta != 0) {
    encoder_pos_ += delta;
    pos_changed_ = true;
  }

  prev_state_ = new_state;
}

const char *RowingMonitor::phase_name_(StrokePhase p) {
  switch (p) {
    case AT_TOP:
      return "AT_TOP";
    case MOVING_DOWN:
      return "MOVING_DOWN";
    case WAITING_FOR_RETURN:
      return "WAITING_RETURN";
    default:
      return "?";
  }
}

void RowingMonitor::setup() {
  pin_step1_->setup();
  pin_step2_->setup();

  prev_state_ = this->read_state_fast_();

  gpio_set_intr_type((gpio_num_t) pin_step1_->get_pin(), GPIO_INTR_ANYEDGE);
  gpio_set_intr_type((gpio_num_t) pin_step2_->get_pin(), GPIO_INTR_ANYEDGE);

  gpio_isr_handler_add((gpio_num_t) pin_step1_->get_pin(), gpio_isr, this);
  gpio_isr_handler_add((gpio_num_t) pin_step2_->get_pin(), gpio_isr, this);

  ESP_LOGI(TAG, "ROW FSM START");
  publish_phase_();
  publish_state_();
}

float RowingMonitor::compute_spm_() {
  int valid_count = 0;
  uint32_t intervals[5];

  for (int i = 0; i < 5; i++) {
    int idx1 = (stroke_ts_index_ - 1 - i + 8) % 8;
    int idx2 = (stroke_ts_index_ - 2 - i + 8) % 8;

    uint32_t t1 = stroke_timestamps_[idx1];
    uint32_t t2 = stroke_timestamps_[idx2];

    if (t1 > 0 && t2 > 0 && t1 > t2) {
      intervals[valid_count++] = t1 - t2;
    }
  }

  if (valid_count == 0)
    return 0.0f;

  uint32_t sum = 0;
  for (int i = 0; i < valid_count; i++)
    sum += intervals[i];

  return 60000.0f / ((float) sum / (float) valid_count);
}

void RowingMonitor::register_valid_stroke_(uint32_t now_ms, long travel) {
  valid_strokes_++;
  valid_travel_sum_ += travel;
  distance_m_ += travel * meters_per_travel_;
  last_stroke_ms_ = now_ms;

  stroke_timestamps_[stroke_ts_index_] = now_ms;
  stroke_ts_index_ = (stroke_ts_index_ + 1) % 8;

  ESP_LOGI(TAG, "STROKE #%u  travel=%ld  spm=%.1f", valid_strokes_, travel, compute_spm_());
}

void RowingMonitor::register_short_stroke_(long travel) {
  short_strokes_++;
  short_travel_sum_ += travel;
  short_distance_m_ += travel * meters_per_travel_;

  ESP_LOGI(TAG, "SHORT #%u  travel=%ld", short_strokes_, travel);
}

void RowingMonitor::register_micro_stroke_(long travel) {
  micro_strokes_++;
  micro_travel_sum_ += travel;
  micro_distance_m_ += travel * meters_per_travel_;

  ESP_LOGI(TAG, "MICRO #%u  travel=%ld", micro_strokes_, travel);
}

void RowingMonitor::publish_phase_() {
  if (phase_text_sensor_ != nullptr) {
    phase_text_sensor_->publish_state(phase_name_(phase_));
  }
}

void RowingMonitor::publish_state_() {
  if (valid_strokes_sensor_ != nullptr)
    valid_strokes_sensor_->publish_state(valid_strokes_);
  if (short_strokes_sensor_ != nullptr)
    short_strokes_sensor_->publish_state(short_strokes_);
  if (micro_strokes_sensor_ != nullptr)
    micro_strokes_sensor_->publish_state(micro_strokes_);
  if (spm_sensor_ != nullptr)
    spm_sensor_->publish_state(compute_spm_());
  if (active_time_sensor_ != nullptr)
    active_time_sensor_->publish_state(active_time_s_);
  if (distance_sensor_ != nullptr)
    distance_sensor_->publish_state(distance_m_);
  if (short_distance_sensor_ != nullptr)
    short_distance_sensor_->publish_state(short_distance_m_);
  if (micro_distance_sensor_ != nullptr)
    micro_distance_sensor_->publish_state(micro_distance_m_);

  if (avg_valid_travel_sensor_ != nullptr) {
    float v = valid_strokes_ > 0 ? (float) valid_travel_sum_ / (float) valid_strokes_ : 0.0f;
    avg_valid_travel_sensor_->publish_state(v);
  }
  if (avg_short_travel_sensor_ != nullptr) {
    float v = short_strokes_ > 0 ? (float) short_travel_sum_ / (float) short_strokes_ : 0.0f;
    avg_short_travel_sensor_->publish_state(v);
  }
  if (avg_micro_travel_sensor_ != nullptr) {
    float v = micro_strokes_ > 0 ? (float) micro_travel_sum_ / (float) micro_strokes_ : 0.0f;
    avg_micro_travel_sensor_->publish_state(v);
  }
}

void RowingMonitor::loop() {
  uint32_t now = millis();

  long pos_snapshot;
  bool changed_snapshot;

  noInterrupts();
  pos_snapshot = encoder_pos_;
  changed_snapshot = pos_changed_;
  pos_changed_ = false;
  interrupts();

  if (changed_snapshot) {
    last_movement_ms_ = now;

    if (!session_active_) {
      session_active_ = true;
      session_start_ms_ = now;
      last_active_sample_ms_ = now;
      ESP_LOGI(TAG, "SESSION START");
    }
  }

  if (session_active_) {
    uint32_t dt = now - last_active_sample_ms_;
    last_active_sample_ms_ = now;

    if ((now - last_movement_ms_) <= active_idle_ms_) {
      active_time_s_ += dt / 1000.0f;
    }
  }

  if (session_active_ && (now - last_movement_ms_ > session_timeout_ms_)) {
    session_active_ = false;

    float avg_valid = valid_strokes_ > 0 ? (float) valid_travel_sum_ / (float) valid_strokes_ : 0.0f;
    float avg_short = short_strokes_ > 0 ? (float) short_travel_sum_ / (float) short_strokes_ : 0.0f;
    float avg_micro = micro_strokes_ > 0 ? (float) micro_travel_sum_ / (float) micro_strokes_ : 0.0f;

    ESP_LOGI(TAG,
             "SESSION END  duration_s=%u  active_s=%.1f  valid=%u  short=%u  micro=%u  avgValidTravel=%.1f  "
             "avgShortTravel=%.1f  avgMicroTravel=%.1f  dist_m=%.2f  short_dist_m=%.2f  micro_dist_m=%.2f",
             (now - session_start_ms_) / 1000, active_time_s_, valid_strokes_, short_strokes_, micro_strokes_,
             avg_valid, avg_short, avg_micro, distance_m_, short_distance_m_, micro_distance_m_);
  }

  if (pos_snapshot < cycle_min_pos_)
    cycle_min_pos_ = pos_snapshot;
  if (pos_snapshot > cycle_max_pos_)
    cycle_max_pos_ = pos_snapshot;

  StrokePhase old_phase = phase_;

  switch (phase_) {
    case AT_TOP:
      cycle_min_pos_ = pos_snapshot;
      cycle_max_pos_ = pos_snapshot;

      if (pos_snapshot <= top_leave_threshold_) {
        phase_ = MOVING_DOWN;
        cycle_min_pos_ = pos_snapshot;
        cycle_max_pos_ = pos_snapshot;
      }
      break;

    case MOVING_DOWN:
      if (cycle_min_pos_ <= bottom_threshold_) {
        phase_ = WAITING_FOR_RETURN;
      }

      if (pos_snapshot >= top_enter_threshold_ && cycle_min_pos_ > micro_threshold_) {
        phase_ = AT_TOP;
      } else if (pos_snapshot >= top_enter_threshold_ && cycle_min_pos_ <= micro_threshold_ &&
                 cycle_min_pos_ > short_threshold_) {
        long travel = cycle_max_pos_ - cycle_min_pos_;
        register_micro_stroke_(travel);
        phase_ = AT_TOP;
      } else if (pos_snapshot >= top_enter_threshold_ && cycle_min_pos_ <= short_threshold_ &&
                 cycle_min_pos_ > bottom_threshold_) {
        long travel = cycle_max_pos_ - cycle_min_pos_;
        register_short_stroke_(travel);
        phase_ = AT_TOP;
      }
      break;

    case WAITING_FOR_RETURN:
      if (pos_snapshot >= top_enter_threshold_) {
        long travel = cycle_max_pos_ - cycle_min_pos_;

        if ((now - last_stroke_ms_) >= min_stroke_ms_) {
          register_valid_stroke_(now, travel);
        }

        phase_ = AT_TOP;
      }
      break;
  }

  if (phase_ != old_phase) {
    publish_phase_();
  }

  if (now - last_publish_ms_ >= 500) {
    last_publish_ms_ = now;

    ESP_LOGD(TAG, "pos=%ld  phase=%s  min=%ld  max=%ld  valid=%u  short=%u  micro=%u  spm=%.1f  active_s=%.1f  "
                   "dist_m=%.2f",
             pos_snapshot, phase_name_(phase_), cycle_min_pos_, cycle_max_pos_, valid_strokes_, short_strokes_,
             micro_strokes_, compute_spm_(), active_time_s_, distance_m_);

    publish_state_();
  }
}

}  // namespace rowing_monitor
}  // namespace esphome
