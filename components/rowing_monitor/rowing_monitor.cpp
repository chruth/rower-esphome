#include "rowing_monitor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rowing_monitor {

static const char *const TAG = "rowing_monitor";

static const int8_t QUAD_TABLE[16] = {
    0, -1, +1, 0,
    +1, 0, 0, -1,
    -1, 0, 0, +1,
    0, +1, -1, 0};

void RowingMonitorComponent::setup() {
  pin_step1_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  pin_step2_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);

  prev_state_ = read_state_fast_();

  pin_step1_->attach_interrupt(RowingMonitorComponent::gpio_intr, this, gpio::INTERRUPT_ANY_EDGE);
  pin_step2_->attach_interrupt(RowingMonitorComponent::gpio_intr, this, gpio::INTERRUPT_ANY_EDGE);

  ESP_LOGI(TAG, "Rowing monitor started");
}

void RowingMonitorComponent::gpio_intr(RowingMonitorComponent *arg) {
  arg->handle_encoder_isr_();
}

uint8_t RowingMonitorComponent::read_state_fast_() {
  uint8_t a = pin_step1_->digital_read();
  uint8_t b = pin_step2_->digital_read();
  return (a << 1) | b;
}

void RowingMonitorComponent::handle_encoder_isr_() {
  uint8_t new_state = read_state_fast_();
  uint8_t index = (prev_state_ << 2) | new_state;
  int8_t delta = QUAD_TABLE[index];

  if (delta != 0) {
    encoder_pos_ += delta;
    pos_changed_ = true;
  }

  prev_state_ = new_state;
}

float RowingMonitorComponent::compute_spm_() {
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

  return 60000.0f / (static_cast<float>(sum) / valid_count);
}

const char *RowingMonitorComponent::phase_name_(StrokePhase p) {
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

void RowingMonitorComponent::register_valid_stroke_(uint32_t now_ms, long travel) {
  valid_strokes_++;
  last_stroke_ms_ = now_ms;
  sum_valid_travel_ += static_cast<float>(travel);
  distance_m_ += static_cast<float>(travel) * meters_per_travel_;

  stroke_timestamps_[stroke_ts_index_] = now_ms;
  stroke_ts_index_ = (stroke_ts_index_ + 1) % 8;

  ESP_LOGD(TAG, "VALID STROKE #%u travel=%ld spm=%.1f dist=%.2f",
           valid_strokes_, travel, compute_spm_(), distance_m_);
}

void RowingMonitorComponent::register_short_stroke_(long travel) {
  short_strokes_++;
  sum_short_travel_ += static_cast<float>(travel);
  short_distance_m_ += static_cast<float>(travel) * meters_per_travel_;

  ESP_LOGD(TAG, "SHORT STROKE #%u travel=%ld short_dist=%.2f",
           short_strokes_, travel, short_distance_m_);
}

void RowingMonitorComponent::register_micro_stroke_(long travel) {
  micro_strokes_++;
  sum_micro_travel_ += static_cast<float>(travel);
  micro_distance_m_ += static_cast<float>(travel) * meters_per_travel_;

  ESP_LOGD(TAG, "MICRO STROKE #%u travel=%ld micro_dist=%.2f",
           micro_strokes_, travel, micro_distance_m_);
}

void RowingMonitorComponent::publish_all_() {
  if (strokes_sensor_ != nullptr)
    strokes_sensor_->publish_state(valid_strokes_);
  if (short_strokes_sensor_ != nullptr)
    short_strokes_sensor_->publish_state(short_strokes_);
  if (micro_strokes_sensor_ != nullptr)
    micro_strokes_sensor_->publish_state(micro_strokes_);
  if (spm_sensor_ != nullptr)
    spm_sensor_->publish_state(compute_spm_());
  if (distance_sensor_ != nullptr)
    distance_sensor_->publish_state(distance_m_);
  if (short_distance_sensor_ != nullptr)
    short_distance_sensor_->publish_state(short_distance_m_);
  if (micro_distance_sensor_ != nullptr)
    micro_distance_sensor_->publish_state(micro_distance_m_);
  if (active_time_sensor_ != nullptr)
    active_time_sensor_->publish_state(active_time_s_);

  if (avg_valid_travel_sensor_ != nullptr) {
    float avg = valid_strokes_ > 0 ? (sum_valid_travel_ / valid_strokes_) : 0.0f;
    avg_valid_travel_sensor_->publish_state(avg);
  }

  if (avg_short_travel_sensor_ != nullptr) {
    float avg = short_strokes_ > 0 ? (sum_short_travel_ / short_strokes_) : 0.0f;
    avg_short_travel_sensor_->publish_state(avg);
  }

  if (avg_micro_travel_sensor_ != nullptr) {
    float avg = micro_strokes_ > 0 ? (sum_micro_travel_ / micro_strokes_) : 0.0f;
    avg_micro_travel_sensor_->publish_state(avg);
  }

  if (phase_sensor_ != nullptr)
    phase_sensor_->publish_state(phase_name_(phase_));
}

void RowingMonitorComponent::loop() {
  static uint32_t last_publish_ms = 0;
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
      last_active_accum_ms_ = now;
      ESP_LOGI(TAG, "SESSION START");
    }
  }

  if (session_active_) {
    if ((now - last_movement_ms_) <= active_idle_ms_) {
      if (last_active_accum_ms_ != 0) {
        active_time_s_ += (now - last_active_accum_ms_) / 1000.0f;
      }
    }
    last_active_accum_ms_ = now;
  }

  if (session_active_ && (now - last_movement_ms_ > session_timeout_ms_)) {
    session_active_ = false;

    float avg_valid = valid_strokes_ > 0 ? (sum_valid_travel_ / valid_strokes_) : 0.0f;
    float avg_short = short_strokes_ > 0 ? (sum_short_travel_ / short_strokes_) : 0.0f;
    float avg_micro = micro_strokes_ > 0 ? (sum_micro_travel_ / micro_strokes_) : 0.0f;

    ESP_LOGI(TAG,
             "SESSION END duration_s=%u active_s=%.1f valid=%u short=%u micro=%u "
             "avgValidTravel=%.1f avgShortTravel=%.1f avgMicroTravel=%.1f "
             "dist_m=%.2f short_dist_m=%.2f micro_dist_m=%.2f",
             (now - session_start_ms_) / 1000, active_time_s_,
             valid_strokes_, short_strokes_, micro_strokes_,
             avg_valid, avg_short, avg_micro,
             distance_m_, short_distance_m_, micro_distance_m_);
  }

  if (pos_snapshot < cycle_min_pos_)
    cycle_min_pos_ = pos_snapshot;
  if (pos_snapshot > cycle_max_pos_)
    cycle_max_pos_ = pos_snapshot;

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

      if (pos_snapshot >= top_enter_threshold_ && cycle_min_pos_ > bottom_threshold_) {
        long travel = cycle_max_pos_ - cycle_min_pos_;

        if (travel <= micro_threshold_) {
          register_micro_stroke_(travel);
        } else if (travel <= short_threshold_) {
          register_short_stroke_(travel);
        }

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

  if (now - last_publish_ms >= 500) {
    last_publish_ms = now;
    publish_all_();

    ESP_LOGD(TAG,
             "pos=%ld phase=%s min=%ld max=%ld valid=%u short=%u micro=%u spm=%.1f active_s=%.1f dist_m=%.2f",
             pos_snapshot, phase_name_(phase_), cycle_min_pos_, cycle_max_pos_,
             valid_strokes_, short_strokes_, micro_strokes_, compute_spm_(),
             active_time_s_, distance_m_);
  }
}

}  // namespace rowing_monitor
}  // namespace esphome
