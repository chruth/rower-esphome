#include "rowing_monitor.h"

#include <algorithm>
#include <cmath>

#include "driver/gpio.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace rowing_monitor {

static const char *const TAG = "rowing_monitor";

void IRAM_ATTR RowingMonitor::gpio_isr(void *arg) {
  auto *self = static_cast<RowingMonitor *>(arg);
  self->handle_isr_();
}

uint8_t RowingMonitor::read_state_fast_() const {
  // Use direct GPIO reads so ISR path stays lightweight and deterministic.
  const uint8_t a = gpio_get_level((gpio_num_t) this->pin_step1_->get_pin()) ? 1 : 0;
  const uint8_t b = gpio_get_level((gpio_num_t) this->pin_step2_->get_pin()) ? 1 : 0;
  return (a << 1) | b;
}

void RowingMonitor::handle_isr_() {
  const uint8_t state = this->read_state_fast_() & 0x3;
  const uint8_t idx = (this->prev_state_ << 2) | state;
  const int8_t delta = QUAD_TABLE_[idx];
  if (delta != 0) {
    this->encoder_pos_ += delta;
    this->pos_changed_ = true;
  }
  this->prev_state_ = state;
}

void RowingMonitor::setup() {
  ESP_LOGI(TAG, "Setting up rowing_monitor");

  if (this->pin_step1_ == nullptr || this->pin_step2_ == nullptr) {
    ESP_LOGE(TAG, "Pins not configured (pin_step1/pin_step2)");
    this->mark_failed();
    return;
  }

  this->pin_step1_->setup();
  this->pin_step2_->setup();

  this->prev_state_ = this->read_state_fast_() & 0x3;

  // Ensure the global ISR service is available before handler registration.
  const esp_err_t isr_service = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if (isr_service != ESP_OK && isr_service != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Failed to install GPIO ISR service: %d", (int) isr_service);
    this->mark_failed();
    return;
  }

  gpio_set_intr_type((gpio_num_t) this->pin_step1_->get_pin(), GPIO_INTR_ANYEDGE);
  gpio_set_intr_type((gpio_num_t) this->pin_step2_->get_pin(), GPIO_INTR_ANYEDGE);
  const esp_err_t isr_step1 =
      gpio_isr_handler_add((gpio_num_t) this->pin_step1_->get_pin(), &RowingMonitor::gpio_isr, this);
  const esp_err_t isr_step2 =
      gpio_isr_handler_add((gpio_num_t) this->pin_step2_->get_pin(), &RowingMonitor::gpio_isr, this);
  if (isr_step1 != ESP_OK || isr_step2 != ESP_OK) {
    ESP_LOGE(TAG, "Failed to attach GPIO ISRs: step1=%d step2=%d", (int) isr_step1, (int) isr_step2);
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG,
           "Configured thresholds: top_enter=%ld top_leave=%ld micro=%ld short=%ld bottom=%ld",
           (long) this->top_enter_threshold_, (long) this->top_leave_threshold_, (long) this->micro_threshold_,
           (long) this->short_threshold_, (long) this->bottom_threshold_);
  ESP_LOGI(TAG,
           "Timing: min_stroke=%ums active_idle=%ums session_timeout=%ums meters_per_travel=%.4f",
           (unsigned) this->min_stroke_ms_, (unsigned) this->active_idle_ms_, (unsigned) this->session_timeout_ms_,
           (double) this->meters_per_travel_);

  this->published_phase_ = Phase::MOVING_DOWN;  // force initial publish even if AT_TOP
  this->phase_ = Phase::AT_TOP;
  this->publish_phase_();  // publish initial phase immediately
  this->reset_cycle_tracking_(0);
}

void RowingMonitor::loop() {
  const uint32_t now = millis();

  bool changed = false;
  int32_t pos = 0;
  {
    InterruptLock lock;
    changed = this->pos_changed_;
    pos = this->encoder_pos_;
    this->pos_changed_ = false;
  }

  if (changed) {
    this->on_motion_(now);
    this->on_position_update_(now, pos);
  }

  if (this->session_active_) {
    this->sample_active_time_(now);
    this->maybe_end_session_(now);
  }

  if (now - this->last_publish_ms_ >= 500U) {
    this->last_publish_ms_ = now;
    this->publish_state_();
  }

  if (now - this->last_debug_ms_ >= 5000U && (this->session_active_ || changed)) {
    this->last_debug_ms_ = now;
    this->log_periodic_debug_(now);
  }
}

void RowingMonitor::update_phase_(Phase phase) {
  if (this->phase_ == phase)
    return;
  this->phase_ = phase;
  this->publish_phase_();
}

const char *RowingMonitor::phase_name_(Phase phase) const {
  switch (phase) {
    case Phase::AT_TOP:
      return "AT_TOP";
    case Phase::MOVING_DOWN:
      return "MOVING_DOWN";
    case Phase::WAITING_FOR_RETURN:
      return "WAITING_RETURN";
    default:
      return "UNKNOWN";
  }
}

void RowingMonitor::publish_phase_() {
  if (this->published_phase_ == this->phase_)
    return;
  this->published_phase_ = this->phase_;

  if (this->phase_text_sensor_ != nullptr) {
    this->phase_text_sensor_->publish_state(this->phase_name_(this->phase_));
  }
}

void RowingMonitor::on_motion_(uint32_t now_ms) {
  this->last_motion_ms_ = now_ms;

  if (!this->session_active_) {
    this->session_active_ = true;
    this->session_start_ms_ = now_ms;
    this->last_active_sample_ms_ = now_ms;
    ESP_LOGI(TAG, "Session started");
  }
}

void RowingMonitor::maybe_end_session_(uint32_t now_ms) {
  if (now_ms - this->last_motion_ms_ <= this->session_timeout_ms_)
    return;

  const uint32_t duration_ms = now_ms - this->session_start_ms_;
  const float duration_s = duration_ms / 1000.0f;
  const float active_s = this->active_time_ms_ / 1000.0f;

  const float avg_valid = this->valid_strokes_ > 0 ? (float) this->valid_travel_sum_ / (float) this->valid_strokes_ : 0.0f;
  const float avg_short = this->short_strokes_ > 0 ? (float) this->short_travel_sum_ / (float) this->short_strokes_ : 0.0f;
  const float avg_micro = this->micro_strokes_ > 0 ? (float) this->micro_travel_sum_ / (float) this->micro_strokes_ : 0.0f;

  ESP_LOGI(TAG,
           "Session ended: duration=%.1fs active=%.1fs valid=%u short=%u micro=%u dist=%.2fm short_dist=%.2fm micro_dist=%.2fm avg_travel(valid/short/micro)=%.1f/%.1f/%.1f",
           (double) duration_s, (double) active_s, (unsigned) this->valid_strokes_, (unsigned) this->short_strokes_,
           (unsigned) this->micro_strokes_, (double) (this->valid_distance_m_ + this->short_distance_m_),
           (double) this->short_distance_m_, (double) this->micro_distance_m_, (double) avg_valid, (double) avg_short,
           (double) avg_micro);

  this->session_active_ = false;
  this->session_start_ms_ = 0;
  this->last_motion_ms_ = 0;
  this->last_active_sample_ms_ = 0;
}

void RowingMonitor::sample_active_time_(uint32_t now_ms) {
  if (this->last_active_sample_ms_ == 0) {
    this->last_active_sample_ms_ = now_ms;
    return;
  }

  const uint32_t dt = now_ms - this->last_active_sample_ms_;
  this->last_active_sample_ms_ = now_ms;

  if (now_ms - this->last_motion_ms_ <= this->active_idle_ms_) {
    this->active_time_ms_ += dt;
  }
}

void RowingMonitor::reset_cycle_tracking_(int32_t pos) {
  this->cycle_min_pos_ = pos;
  this->cycle_max_pos_ = pos;
  this->cycle_reached_bottom_ = false;
}

void RowingMonitor::update_cycle_tracking_(int32_t pos) {
  this->cycle_min_pos_ = std::min(this->cycle_min_pos_, pos);
  this->cycle_max_pos_ = std::max(this->cycle_max_pos_, pos);
  if (this->cycle_min_pos_ <= this->bottom_threshold_) {
    this->cycle_reached_bottom_ = true;
  }
}

void RowingMonitor::on_position_update_(uint32_t now_ms, int32_t pos) {
  switch (this->phase_) {
    case Phase::AT_TOP: {
      if (pos < this->top_leave_threshold_) {
        this->reset_cycle_tracking_(pos);
        this->update_phase_(Phase::MOVING_DOWN);
      }
      break;
    }
    case Phase::MOVING_DOWN: {
      this->update_cycle_tracking_(pos);
      if (this->cycle_reached_bottom_) {
        this->update_phase_(Phase::WAITING_FOR_RETURN);
      } else if (pos >= this->top_enter_threshold_) {
        this->complete_cycle_(now_ms, pos);
        this->update_phase_(Phase::AT_TOP);
      }
      break;
    }
    case Phase::WAITING_FOR_RETURN: {
      this->update_cycle_tracking_(pos);
      if (pos >= this->top_enter_threshold_) {
        this->complete_cycle_(now_ms, pos);
        this->update_phase_(Phase::AT_TOP);
      }
      break;
    }
    default:
      this->update_phase_(Phase::AT_TOP);
      break;
  }
}

void RowingMonitor::complete_cycle_(uint32_t now_ms, int32_t pos) {
  this->update_cycle_tracking_(pos);

  const int32_t depth = this->cycle_min_pos_;
  const int32_t travel = this->cycle_max_pos_ - this->cycle_min_pos_;
  if (travel <= 0) {
    this->reset_cycle_tracking_(pos);
    return;
  }

  if (this->cycle_reached_bottom_) {
    if (now_ms - this->last_valid_stroke_ms_ >= this->min_stroke_ms_) {
      this->register_stroke_(StrokeKind::VALID, now_ms, travel, depth);
      this->last_valid_stroke_ms_ = now_ms;
    } else {
      ESP_LOGD(TAG, "Valid stroke suppressed by min_stroke_ms (depth=%ld travel=%ld dt=%ums)",
               (long) depth, (long) travel, (unsigned) (now_ms - this->last_valid_stroke_ms_));
    }
  } else if (depth <= this->short_threshold_) {
    this->register_stroke_(StrokeKind::SHORT, now_ms, travel, depth);
  } else if (depth <= this->micro_threshold_) {
    this->register_stroke_(StrokeKind::MICRO, now_ms, travel, depth);
  }

  this->reset_cycle_tracking_(pos);
}

void RowingMonitor::register_stroke_(StrokeKind kind, uint32_t now_ms, int32_t travel, int32_t depth) {
  const float meters = ((float) travel) * this->meters_per_travel_;

  switch (kind) {
    case StrokeKind::VALID: {
      this->valid_strokes_++;
      this->valid_travel_sum_ += travel;
      this->valid_distance_m_ += meters;
      this->add_valid_stroke_timestamp_(now_ms);
      ESP_LOGI(TAG, "Valid stroke: depth=%ld travel=%ld meters=%.3f spm=%.1f",
               (long) depth, (long) travel, (double) meters, (double) this->compute_spm_());
      break;
    }
    case StrokeKind::SHORT: {
      this->short_strokes_++;
      this->short_travel_sum_ += travel;
      this->short_distance_m_ += meters;
      ESP_LOGI(TAG, "Short stroke: depth=%ld travel=%ld meters=%.3f",
               (long) depth, (long) travel, (double) meters);
      break;
    }
    case StrokeKind::MICRO: {
      this->micro_strokes_++;
      this->micro_travel_sum_ += travel;
      this->micro_distance_m_ += meters;
      ESP_LOGI(TAG, "Micro stroke: depth=%ld travel=%ld meters=%.3f",
               (long) depth, (long) travel, (double) meters);
      break;
    }
  }
}

void RowingMonitor::add_valid_stroke_timestamp_(uint32_t now_ms) {
  this->valid_stroke_ts_[this->valid_stroke_ts_head_] = now_ms;
  this->valid_stroke_ts_head_ = (this->valid_stroke_ts_head_ + 1) % STROKE_TS_CAPACITY_;
  if (this->valid_stroke_ts_count_ < STROKE_TS_CAPACITY_)
    this->valid_stroke_ts_count_++;
}

float RowingMonitor::compute_spm_() const {
  if (this->valid_stroke_ts_count_ < 2)
    return 0.0f;

  const uint8_t intervals_to_use = std::min<uint8_t>(5, this->valid_stroke_ts_count_ - 1);

  uint32_t sum = 0;
  uint8_t count = 0;

  for (uint8_t i = 0; i < intervals_to_use; i++) {
    const int16_t newest_index = (int16_t) this->valid_stroke_ts_head_ - 1 - i;
    const int16_t older_index = (int16_t) this->valid_stroke_ts_head_ - 2 - i;

    const uint8_t newest = (newest_index + STROKE_TS_CAPACITY_) % STROKE_TS_CAPACITY_;
    const uint8_t older = (older_index + STROKE_TS_CAPACITY_) % STROKE_TS_CAPACITY_;

    const uint32_t t_new = this->valid_stroke_ts_[newest];
    const uint32_t t_old = this->valid_stroke_ts_[older];
    if (t_new > t_old) {
      sum += (t_new - t_old);
      count++;
    }
  }

  if (count == 0 || sum == 0)
    return 0.0f;

  const float avg_interval_ms = (float) sum / (float) count;
  return 60000.0f / avg_interval_ms;
}

void RowingMonitor::publish_state_() {
  this->publish_if_changed_(this->valid_strokes_sensor_, (float) this->valid_strokes_, this->last_pub_valid_strokes_);
  this->publish_if_changed_(this->short_strokes_sensor_, (float) this->short_strokes_, this->last_pub_short_strokes_);
  this->publish_if_changed_(this->micro_strokes_sensor_, (float) this->micro_strokes_, this->last_pub_micro_strokes_);

  this->publish_if_changed_(this->spm_sensor_, this->compute_spm_(), this->last_pub_spm_);

  this->publish_if_changed_(this->active_time_sensor_, this->active_time_ms_ / 1000.0f, this->last_pub_active_time_);

  const float distance_m = this->valid_distance_m_ + this->short_distance_m_;  // per requested behavior
  this->publish_if_changed_(this->distance_sensor_, distance_m, this->last_pub_distance_);
  this->publish_if_changed_(this->short_distance_sensor_, this->short_distance_m_, this->last_pub_short_distance_);
  this->publish_if_changed_(this->micro_distance_sensor_, this->micro_distance_m_, this->last_pub_micro_distance_);

  const float avg_valid = this->valid_strokes_ > 0 ? (float) this->valid_travel_sum_ / (float) this->valid_strokes_ : 0.0f;
  const float avg_short = this->short_strokes_ > 0 ? (float) this->short_travel_sum_ / (float) this->short_strokes_ : 0.0f;
  const float avg_micro = this->micro_strokes_ > 0 ? (float) this->micro_travel_sum_ / (float) this->micro_strokes_ : 0.0f;
  this->publish_if_changed_(this->avg_valid_travel_sensor_, avg_valid, this->last_pub_avg_valid_travel_);
  this->publish_if_changed_(this->avg_short_travel_sensor_, avg_short, this->last_pub_avg_short_travel_);
  this->publish_if_changed_(this->avg_micro_travel_sensor_, avg_micro, this->last_pub_avg_micro_travel_);
}

bool RowingMonitor::publish_if_changed_(sensor::Sensor *sensor, float value, float &last_value) {
  if (sensor == nullptr)
    return false;
  if (!std::isnan(last_value) && std::fabs(last_value - value) < 0.0001f)
    return false;

  last_value = value;
  sensor->publish_state(value);
  return true;
}

void RowingMonitor::log_periodic_debug_(uint32_t now_ms) const {
  (void) now_ms;
  ESP_LOGD(TAG,
           "pos=%ld phase=%s min=%ld max=%ld valid=%u short=%u micro=%u spm=%.1f active=%.1fs dist=%.2fm short=%.2fm micro=%.2fm",
           (long) this->encoder_pos_, this->phase_name_(this->phase_), (long) this->cycle_min_pos_, (long) this->cycle_max_pos_,
           (unsigned) this->valid_strokes_, (unsigned) this->short_strokes_, (unsigned) this->micro_strokes_,
           (double) this->compute_spm_(), (double) (this->active_time_ms_ / 1000.0f),
           (double) (this->valid_distance_m_ + this->short_distance_m_), (double) this->short_distance_m_,
           (double) this->micro_distance_m_);
}

}  // namespace rowing_monitor
}  // namespace esphome

