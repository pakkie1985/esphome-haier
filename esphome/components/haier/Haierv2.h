#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace haier {

class Haier : public esphome::climate::Climate, public PollingComponent {
 public:
  Haier() : PollingComponent(2000) {}  // Poll every 2 seconds

  void setup() override {
    ESP_LOGD("Haier", "Setting up Haier climate component...");
    // Initialize UART or other communication here if needed
  }

  void update() override {
    ESP_LOGD("Haier", "Polling Haier AC status...");
    parse_status();
  }

  climate::ClimateTraits traits() override {
    climate::ClimateTraits traits;

    // Supported climate modes
    traits.set_supported_modes({
        climate::CLIMATE_MODE_OFF,
        climate::CLIMATE_MODE_COOL,
        climate::CLIMATE_MODE_HEAT,
        climate::CLIMATE_MODE_DRY,
        climate::CLIMATE_MODE_FAN_ONLY,
        climate::CLIMATE_MODE_HEAT_COOL,
        climate::CLIMATE_MODE_AUTO
    });

    // Supported fan modes
    traits.set_supported_fan_modes({
        climate::CLIMATE_FAN_OFF,
        climate::CLIMATE_FAN_LOW,
        climate::CLIMATE_FAN_MEDIUM,
        climate::CLIMATE_FAN_HIGH,
        climate::CLIMATE_FAN_AUTO,
        climate::CLIMATE_FAN_MIDDLE,
        climate::CLIMATE_FAN_FOCUS,
        climate::CLIMATE_FAN_DIFFUSE
    });

    // Supported swing modes
    traits.set_supported_swing_modes({
        climate::CLIMATE_SWING_OFF,
        climate::CLIMATE_SWING_VERTICAL,
        climate::CLIMATE_SWING_HORIZONTAL,
        climate::CLIMATE_SWING_BOTH
    });

    // Temperature settings
    traits.set_supports_current_temperature(true);
    traits.set_visual_min_temperature(16.0f);
    traits.set_visual_max_temperature(30.0f);
    traits.set_visual_temperature_step(1.0f);

    return traits;
  }

  void control(const climate::ClimateCall &call) override {
    ESP_LOGD("Haier", "Received control call");

    // Handle mode change
    if (call.get_mode().has_value()) {
      auto new_mode = *call.get_mode();
      switch (new_mode) {
        case climate::CLIMATE_MODE_OFF:
        case climate::CLIMATE_MODE_HEAT_COOL:
        case climate::CLIMATE_MODE_COOL:
        case climate::CLIMATE_MODE_HEAT:
        case climate::CLIMATE_MODE_DRY:
        case climate::CLIMATE_MODE_FAN_ONLY:
        case climate::CLIMATE_MODE_AUTO:
          this->mode = new_mode;
          ESP_LOGD("Haier", "Setting mode to %d", this->mode);
          break;
        default:
          ESP_LOGW("Haier", "Unsupported mode: %d", new_mode);
          break;
      }
    }

    // Handle target temperature
    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      if (temp >= 16.0f && temp <= 30.0f) {
        this->target_temperature = temp;
        ESP_LOGD("Haier", "Setting target temperature to %.1f", this->target_temperature);
      } else {
        ESP_LOGW("Haier", "Target temperature %.1f out of range (16-30)", temp);
      }
    }

    // Handle fan mode
    if (call.get_fan_mode().has_value()) {
      auto new_fan_mode = *call.get_fan_mode();
      switch (new_fan_mode) {
        case climate::CLIMATE_FAN_OFF:
        case climate::CLIMATE_FAN_ON:
        case climate::CLIMATE_FAN_LOW:
        case climate::CLIMATE_FAN_MEDIUM:
        case climate::CLIMATE_FAN_MIDDLE:
        case climate::CLIMATE_FAN_HIGH:
        case climate::CLIMATE_FAN_AUTO:
        case climate::CLIMATE_FAN_FOCUS:
        case climate::CLIMATE_FAN_DIFFUSE:
          this->fan_mode = new_fan_mode;
          ESP_LOGD("Haier", "Setting fan mode to %d", this->fan_mode);
          break;
        default:
          ESP_LOGW("Haier", "Unsupported fan mode: %d", new_fan_mode);
          break;
      }
    }

    // Handle swing mode
    if (call.get_swing_mode().has_value()) {
      auto new_swing_mode = *call.get_swing_mode();
      switch (new_swing_mode) {
        case climate::CLIMATE_SWING_OFF:
        case climate::CLIMATE_SWING_VERTICAL:
        case climate::CLIMATE_SWING_HORIZONTAL:
        case climate::CLIMATE_SWING_BOTH:
          this->swing_mode = new_swing_mode;
          ESP_LOGD("Haier", "Setting swing mode to %d", this->swing_mode);
          break;
        default:
          ESP_LOGW("Haier", "Unsupported swing mode: %d", new_swing_mode);
          break;
      }
    }

    // Send updated settings to the AC
    send_command();
    this->publish_state();
  }
