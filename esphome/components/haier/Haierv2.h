#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace haier {

class Haier : public esphome::climate::Climate, public PollingComponent {
 public:
  Haier() : PollingComponent(2000) {}  // Poll every 2 seconds

  // Declare UART component (must be configured in YAML)
  void set_uart(UARTComponent *uart) { this->uart_ = uart; }

  void setup() override {
    ESP_LOGD("Haier", "Setting up Haier climate component...");
    if (!this->uart_) {
      ESP_LOGE("Haier", "UART component not set!");
      this->mark_failed();
      return;
    }
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
        case climate::CLIMATE_MODE_COOL:
        case climate::CLIMATE_MODE_HEAT:
        case climate::CLIMATE_MODE_DRY:
        case climate::CLIMATE_MODE_FAN_ONLY:
        case climate::CLIMATE_MODE_HEAT_COOL:
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

 protected:
  UARTComponent *uart_ = nullptr;  // UART component for communication
  uint8_t status[64];  // Placeholder for status array (adjust size as needed)
  static const uint8_t TEMPERATURE_OFFSET = 0;  // Example offset
  static const uint8_t SET_POINT_OFFSET = 1;   // Example offset
  static const uint8_t MODE_OFFSET = 2;        // Example offset
  static const uint8_t FAN_OFFSET = 3;         // Example offset
  static const uint8_t SWING_OFFSET = 4;       // Example offset

  void parse_status() {
    ESP_LOGD("Haier", "Parsing AC status...");

    // Placeholder: Read status from UART
    // Example: if (!read_ac_status()) {
    //   ESP_LOGW("Haier", "Failed to read AC status");
    //   return;
    // }
    // For now, simulate status data
    status[TEMPERATURE_OFFSET] = 45;  // Example: 22.5°C (45/2)
    status[SET_POINT_OFFSET] = 8;    // Example: 24°C (8+16)
    status[MODE_OFFSET] = 0x01;      // Example: Cool mode
    status[FAN_OFFSET] = 0x04;       // Example: High fan
    status[SWING_OFFSET] = 0x00;     // Example: Swing off

    // Update temperature readings
    this->current_temperature = status[TEMPERATURE_OFFSET] / 2.0f;
    this->target_temperature = status[SET_POINT_OFFSET] + 16.0f;

    // Update mode
    switch (status[MODE_OFFSET]) {
      case 0x00:
        this->mode = climate::CLIMATE_MODE_OFF;
        break;
      case 0x01:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case 0x02:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case 0x03:
        this->mode = climate::CLIMATE_MODE_DRY;
        break;
      case 0x04:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        break;
      case 0x05:
        this->mode = climate::CLIMATE_MODE_HEAT_COOL;
        break;
      case 0x06:
        this->mode = climate::CLIMATE_MODE_AUTO;
        break;
      default:
        ESP_LOGW("Haier", "Unknown mode: %d", status[MODE_OFFSET]);
        this->mode = climate::CLIMATE_MODE_OFF;
        break;
    }

    // Update fan mode
    switch (status[FAN_OFFSET]) {
      case 0x01:
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
      case 0x02:
        this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
        break;
      case 0x03:
        this->fan_mode = climate::CLIMATE_FAN_MIDDLE;
        break;
      case 0x04:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
      case 0x05:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
      case 0x06:
        this->fan_mode = climate::CLIMATE_FAN_FOCUS;
        break;
      case 0x07:
        this->fan_mode = climate::CLIMATE_FAN_DIFFUSE;
        break;
      default:
        ESP_LOGW("Haier", "Unknown fan mode: %d", status[FAN_OFFSET]);
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
    }

    // Update swing mode
    switch (status[SWING_OFFSET]) {
      case 0x00:
        this->swing_mode = climate::CLIMATE_SWING_OFF;
        break;
      case 0x01:
        this->swing_mode = climate::CLIMATE_SWING_BOTH;
        break;
      case 0x02:
        this->swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
        break;
      case 0x03:
        this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
        break;
      default:
        ESP_LOGW("Haier", "Unknown swing mode: %d", status[SWING_OFFSET]);
        this->swing_mode = climate::CLIMATE_SWING_OFF;
        break;
    }

    this->publish_state();
  }

  void send_command() {
    ESP_LOGD("Haier", "Sending command to AC: mode=%d, temp=%.1f, fan=%d, swing=%d",
             this->mode, this->target_temperature, this->fan_mode, this->swing_mode);
    // Placeholder: Implement UART command sending
    // Example: if (this->uart_) {
    //   uint8_t command[64] = {0};
    //   command[0] = this->mode;
    //   command[1] = (uint8_t)(this->target_temperature - 16);
    //   this->uart_->write_array(command, sizeof(command));
    // }
  }
};

}  // namespace haier
}  // namespace esphome
