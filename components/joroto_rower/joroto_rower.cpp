#include "joroto_rower.h"
#include "esphome/core/log.h"
#include <Arduino.h>

namespace esphome {
namespace joroto_rower {

static const char *const TAG = "joroto_rower";

static const int8_t QUAD_TABLE[16] = {
    0, -1, +1,  0,
   +1,  0,  0, -1,
   -1,  0,  0, +1,
    0, +1, -1,  0
};

void JorotoRower::setup() {
  // INPUT_PULLUP:
  pinMode(this->pin_step1_->get_pin(), INPUT_PULLUP);
  pinMode(this->pin_step2_->get_pin(), INPUT_PULLUP);

  this->prev_state_ = this->read_state_fast_();
  this->reset_cycle_(0);

  attachInterruptArg(digitalPinToInterrupt(this->pin_step1_->get_pin()), gpio_isr_static, this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(this->pin_step2_->get_pin()), gpio_isr_static, this, CHANGE);

  ESP_LOGI(TAG, "JOROTO rower component started");
}

void JorotoRower::update() {
  const uint32_t now = millis();

  int32_t pos_snapshot;
  bool changed_snapshot;

  noInterrupts();
  pos_snapshot = this->encoder_pos_;
  changed_snapshot = this->pos_changed_;
  this->pos_changed_ = false;
  interrupts();

  if (changed_snapshot) {
    this->last_movement_ms_ = now;

    if (!this->session_active_) {
      this->session_active_ = true;
      this->session_start_ms_ = now;
      ESP_LOGI(TAG, "SESSION START");
    }

    if (!this->movement_active_) {
      this->movement_active_ = true;
      this->movement_start_ms_ = now;
    }
  }

  // aktive Zeit nur während echter Bewegung
  if (this->movement_active_ && (now - this->last_movement_ms_ > this->active_idle_ms_)) {
    this->active_time_ms_ += (this->last_movement_ms_ - this->movement_start_ms_);
    this->movement_active_ = false;
  }

  // Session-Ende
  if (this->session_active_ && (now - this->last_movement_ms_ > this->session_timeout_ms_)) {
    if (this->movement_active_) {
      this->active_time_ms_ += (this->last_movement_ms_ - this->movement_start_ms_);
      this->movement_active_ = false;
    }

    float avg_valid = this->valid_strokes_ ? (float) this->valid_travel_sum_ / this->valid_strokes_ : 0.0f;
    float avg_short = this->short_strokes_ ? (float) this->short_travel_sum_ / this->short_strokes_ : 0.0f;
    float avg_micro = this->micro_strokes_ ? (float) this->micro_travel_sum_ / this->micro_strokes_ : 0.0f;

    ESP_LOGI(TAG,
             "SESSION END  duration_s=%u  active_s=%.1f  valid=%u  short=%u  micro=%u  "
             "avgValidTravel=%.1f  avgShortTravel=%.1f  avgMicroTravel=%.1f  "
             "dist_m=%.2f  short_dist_m=%.2f  micro_dist_m=%.2f",
             (now - this->session_start_ms_) / 1000,
             this->active_time_ms_ / 1000.0f,
             this->valid_strokes_,
             this->short_strokes_,
             this->micro_strokes_,
             avg_valid,
             avg_short,
             avg_micro,
             this->distance_m_,
             this->short_distance_m_,
             this->micro_distance_m_);

    this->session_active_ = false;
  }

  // Min/Max laufender Zyklus
  if (pos_snapshot < this->cycle_min_pos_) this->cycle_min_pos_ = pos_snapshot;
  if (pos_snapshot > this->cycle_max_pos_) this->cycle_max_pos_ = pos_snapshot;

  switch (this->phase_) {
    case AT_TOP:
      this->cycle_min_pos_ = pos_snapshot;
      this->cycle_max_pos_ = pos_snapshot;

      if (pos_snapshot <= this->top_leave_threshold_) {
        this->phase_ = MOVING_DOWN;
        this->cycle_min_pos_ = pos_snapshot;
        this->cycle_max_pos_ = pos_snapshot;
      }
      break;

    case MOVING_DOWN:
      if (this->cycle_min_pos_ <= this->bottom_threshold_) {
        this->phase_ = WAITING_FOR_RETURN;
      }

      // Rückkehr nach oben ohne tief genug unten zu sein => kurzer/mikro Zug
      if (pos_snapshot >= this->top_enter_threshold_ && this->cycle_min_pos_ > this->bottom_threshold_) {
        int32_t travel = this->cycle_max_pos_ - this->cycle_min_pos_;
        if (travel <= this->micro_stroke_threshold_) {
          this->register_micro_stroke_(travel);
        } else if (travel <= this->short_stroke_threshold_) {
          this->register_short_stroke_(travel);
        }
        this->phase_ = AT_TOP;
      }
      break;

    case WAITING_FOR_RETURN:
      if (pos_snapshot >= this->top_enter_threshold_) {
        int32_t travel = this->cycle_max_pos_ - this->cycle_min_pos_;

        if ((now - this->last_stroke_ms_) >= this->min_stroke_ms_) {
          this->register_valid_stroke_(now, travel);
        } else {
          // zeitlich zu kurz => als short/micro erfassen
          if (travel <= this->micro_stroke_threshold_) {
            this->register_micro_stroke_(travel);
          } else {
            this->register_short_stroke_(travel);
          }
        }

        this->phase_ = AT_TOP;
      }
      break;
  }

  if (this->valid_strokes_sensor_ != nullptr)
    this->valid_strokes_sensor_->publish_state(this->valid_strokes_);
  if (this->short_strokes_sensor_ != nullptr)
    this->short_strokes_sensor_->publish_state(this->short_strokes_);
  if (this->micro_strokes_sensor_ != nullptr)
    this->micro_strokes_sensor_->publish_state(this->micro_strokes_);
  if (this->spm_sensor_ != nullptr)
    this->spm_sensor_->publish_state(this->compute_spm_());

  float active_s = this->active_time_ms_ / 1000.0f;
  if (this->movement_active_) {
    active_s += (now - this->movement_start_ms_) / 1000.0f;
  }

  if (this->active_time_sensor_ != nullptr)
    this->active_time_sensor_->publish_state(active_s);
  if (this->distance_sensor_ != nullptr)
    this->distance_sensor_->publish_state(this->distance_m_);
  if (this->short_distance_sensor_ != nullptr)
    this->short_distance_sensor_->publish_state(this->short_distance_m_);
  if (this->micro_distance_sensor_ != nullptr)
    this->micro_distance_sensor_->publish_state(this->micro_distance_m_);
  if (this->position_sensor_ != nullptr)
    this->position_sensor_->publish_state(pos_snapshot);

  ESP_LOGD(TAG,
           "pos=%ld phase=%d min=%ld max=%ld valid=%u short=%u micro=%u spm=%.1f active_s=%.1f dist_m=%.2f",
           (long) pos_snapshot,
           (int) this->phase_,
           (long) this->cycle_min_pos_,
           (long) this->cycle_max_pos_,
           this->valid_strokes_,
           this->short_strokes_,
           this->micro_strokes_,
           this->compute_spm_(),
           active_s,
           this->distance_m_);
}

void IRAM_ATTR JorotoRower::gpio_isr_static(void *arg) {
  auto *self = reinterpret_cast<JorotoRower *>(arg);
  self->gpio_isr();
}

void IRAM_ATTR JorotoRower::gpio_isr() {
  uint8_t new_state = this->read_state_fast_();
  uint8_t index = (this->prev_state_ << 2) | new_state;
  int8_t delta = QUAD_TABLE[index];

  if (delta != 0) {
    // Falls Richtung invertiert ist: hier einfach -delta statt +delta
    this->encoder_pos_ += delta;
    this->pos_changed_ = true;
  }

  this->prev_state_ = new_state;
}

uint8_t JorotoRower::read_state_fast_() {
  uint8_t a = digitalRead(this->pin_step1_->get_pin());
  uint8_t b = digitalRead(this->pin_step2_->get_pin());
  return (a << 1) | b;
}

float JorotoRower::compute_spm_() {
  int valid_count = 0;
  uint32_t intervals[5];

  for (int i = 0; i < 5; i++) {
    int idx1 = (this->stroke_ts_index_ - 1 - i + 8) % 8;
    int idx2 = (this->stroke_ts_index_ - 2 - i + 8) % 8;

    uint32_t t1 = this->stroke_timestamps_[idx1];
    uint32_t t2 = this->stroke_timestamps_[idx2];

    if (t1 > 0 && t2 > 0 && t1 > t2) {
      intervals[valid_count++] = t1 - t2;
    }
  }

  if (valid_count == 0) return 0.0f;

  uint32_t sum = 0;
  for (int i = 0; i < valid_count; i++) sum += intervals[i];

  return 60000.0f / ((float) sum / valid_count);
}

void JorotoRower::register_valid_stroke_(uint32_t now_ms, int32_t travel) {
  this->valid_strokes_++;
  this->valid_travel_sum_ += travel;
  this->distance_m_ += travel * this->meters_per_valid_travel_;
  this->last_stroke_ms_ = now_ms;

  this->stroke_timestamps_[this->stroke_ts_index_] = now_ms;
  this->stroke_ts_index_ = (this->stroke_ts_index_ + 1) % 8;

  ESP_LOGI(TAG, "STROKE #%u  travel=%ld  spm=%.1f  dist_m=%.2f",
           this->valid_strokes_, (long) travel, this->compute_spm_(), this->distance_m_);
}

void JorotoRower::register_short_stroke_(int32_t travel) {
  this->short_strokes_++;
  this->short_travel_sum_ += travel;
  this->short_distance_m_ += travel * this->meters_per_short_travel_;
}

void JorotoRower::register_micro_stroke_(int32_t travel) {
  this->micro_strokes_++;
  this->micro_travel_sum_ += travel;
  this->micro_distance_m_ += travel * this->meters_per_micro_travel_;
}

void JorotoRower::reset_cycle_(int32_t pos) {
  this->cycle_min_pos_ = pos;
  this->cycle_max_pos_ = pos;
}

}  // namespace joroto_rower
}  // namespace esphome
