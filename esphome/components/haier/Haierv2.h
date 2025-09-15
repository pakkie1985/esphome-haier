#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/api/enums.pb.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace haier {

class Haier : public esphome::climate::Climate, public PollingComponent {
 public:
  Haier() : PollingComponent(2000) {}

  void set_uart(esphome::uart::UARTComponent *uart) { this->uart_ = uart; }

  void setup() override {
    ESP_LOGD("Haier", "Opzetten van Haier climate component...");
    if (!this->uart_) {
      ESP_LOGE("Haier", "UART component niet ingesteld!");
      this->mark_failed();
      return;
    }
  }

  void update() override {
    ESP_LOGD("Haier", "Pollen van Haier AC status...");
    parse_status();
  }

  esphome::climate::ClimateTraits traits() override {
    esphome::climate::ClimateTraits traits;

    traits.set_supported_modes({
        esphome::api::enums::CLIMATE_MODE_OFF,
        esphome::api::enums::CLIMATE_MODE_COOL,
        esphome::api::enums::CLIMATE_MODE_HEAT,
        esphome::api::enums::CLIMATE_MODE_DRY,
        esphome::api::enums::CLIMATE_MODE_FAN_ONLY,
        esphome::api::enums::CLIMATE_MODE_HEAT_COOL,
        esphome::api::enums::CLIMATE_MODE_AUTO
    });

    traits.set_supported_fan_modes({
        esphome::api::enums::CLIMATE_FAN_OFF,
        esphome::api::enums::CLIMATE_FAN_LOW,
        esphome::api::enums::CLIMATE_FAN_MEDIUM,
        esphome::api::enums::CLIMATE_FAN_HIGH,
        esphome::api::enums::CLIMATE_FAN_AUTO,
        esphome::api::enums::CLIMATE_FAN_MIDDLE,
        esphome::api::enums::CLIMATE_FAN_FOCUS,
        esphome::api::enums::CLIMATE_FAN_DIFFUSE
    });

    traits.set_supported_swing_modes({
        esphome::api::enums::CLIMATE_SWING_OFF,
        esphome::api::enums::CLIMATE_SWING_VERTICAL,
        esphome::api::enums::CLIMATE_SWING_HORIZONTAL,
        esphome::api::enums::CLIMATE_SWING_BOTH
    });

    traits.set_supports_current_temperature(true);
    traits.set_visual_min_temperature(16.0f);
    traits.set_visual_max_temperature(30.0f);
    traits.set_visual_temperature_step(1.0f);

    return traits;
  }

  void control(const esphome::climate::ClimateCall &call) override {
    ESP_LOGD("Haier", "Control oproep ontvangen");

    if (call.get_mode().has_value()) {
      auto new_mode = *call.get_mode();
      switch (new_mode) {
        case esphome::api::enums::CLIMATE_MODE_OFF:
        case esphome::api::enums::CLIMATE_MODE_COOL:
        case esphome::api::enums::CLIMATE_MODE_HEAT:
        case esphome::api::enums::CLIMATE_MODE_DRY:
        case esphome::api::enums::CLIMATE_MODE_FAN_ONLY:
        case esphome::api::enums::CLIMATE_MODE_HEAT_COOL:
        case esphome::api::enums::CLIMATE_MODE_AUTO:
          this->mode = new_mode;
          ESP_LOGD("Haier", "Mode instellen op %d", this->mode);
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde mode: %d", new_mode);
          break;
      }
    }

    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      if (temp >= 16.0f && temp <= 30.0f) {
        this->target_temperature = temp;
        ESP_LOGD("Haier", "Target temperatuur instellen op %.1f", this->target_temperature);
      } else {
        ESP_LOGW("Haier", "Target temperatuur %.1f buiten bereik (16-30)", temp);
      }
    }

    if (call.get_fan_mode().has_value()) {
      auto new_fan_mode = *call.get_fan_mode();
      switch (new_fan_mode) {
        case esphome::api::enums::CLIMATE_FAN_OFF:
        case esphome::api::enums::CLIMATE_FAN_LOW:
        case esphome::api::enums::CLIMATE_FAN_MEDIUM:
        case esphome::api::enums::CLIMATE_FAN_MIDDLE:
        case esphome::api::enums::CLIMATE_FAN_HIGH:
        case esphome::api::enums::CLIMATE_FAN_AUTO:
        case esphome::api::enums::CLIMATE_FAN_FOCUS:
        case esphome::api::enums::CLIMATE_FAN_DIFFUSE:
          this->fan_mode = new_fan_mode;
          ESP_LOGD("Haier", "Fan mode instellen op %d", this->fan_mode);
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde fan mode: %d", new_fan_mode);
          break;
      }
    }

    if (call.get_swing_mode().has_value()) {
      auto new_swing_mode = *call.get_swing_mode();
      switch (new_swing_mode) {
        case esphome::api::enums::CLIMATE_SWING_OFF:
        case esphome::api::enums::CLIMATE_SWING_VERTICAL:
        case esphome::api::enums::CLIMATE_SWING_HORIZONTAL:
        case esphome::api::enums::CLIMATE_SWING_BOTH:
          this->swing_mode = new_swing_mode;
          ESP_LOGD("Haier", "Swing mode instellen op %d", this->swing_mode);
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde swing mode: %d", new_swing_mode);
          break;
      }
    }

    send_command();
    this->publish_state();
  }

 protected:
  esphome::uart::UARTComponent *uart_ = nullptr;
  uint8_t status[64];
  static const uint8_t TEMPERATURE_OFFSET = 0;
  static const uint8_t SET_POINT_OFFSET = 1;
  static const uint8_t MODE_OFFSET = 2;
  static const uint8_t FAN_OFFSET = 3;
  static const uint8_t SWING_OFFSET = 4;

  void parse_status() {
    ESP_LOGD("Haier", "Status van AC parsen...");

    // Simuleer of lees status via UART (implementeer dit)
    status[TEMPERATURE_OFFSET] = 45; // Voorbeeld
    status[SET_POINT_OFFSET] = 8; // Voorbeeld
    status[MODE_OFFSET] = 0x01; // Voorbeeld
    status[FAN_OFFSET] = 0x04; // Voorbeeld
    status[SWING_OFFSET] = 0x00; // Voorbeeld

    this->current_temperature = status[TEMPERATURE_OFFSET] / 2.0f;
    this->target_temperature = status[SET_POINT_OFFSET] + 16.0f;

    switch (status[MODE_OFFSET]) {
      case 0x00:
        this->mode = esphome::api::enums::CLIMATE_MODE_OFF;
        break;
      // Voeg andere cases toe zoals in je originele code
      default:
        this->mode = esphome::api::enums::CLIMATE_MODE_OFF;
        break;
    }

    // Voeg vergelijkbare switches toe voor fan_mode en swing_mode zoals in je originele code

    this->publish_state();
  }

  void send_command() {
    // Implementeer UART commando hier
  }
};

}  // namespace haier
}  // namespace esphome
