#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace haier {

class Haier : public esphome::climate::Climate, public PollingComponent {
 public:
  Haier() : PollingComponent(2000) {}  // Poll elke 2 seconden

  // Stel UART component in (moet in YAML geconfigureerd worden)
  void set_uart(UARTComponent *uart) { this->uart_ = uart; }

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

    // Ondersteunde climate modes
    traits.set_supported_modes({
        esphome::climate::CLIMATE_MODE_OFF,
        esphome::climate::CLIMATE_MODE_COOL,
        esphome::climate::CLIMATE_MODE_HEAT,
        esphome::climate::CLIMATE_MODE_DRY,
        esphome::climate::CLIMATE_MODE_FAN_ONLY,
        esphome::climate::CLIMATE_MODE_HEAT_COOL,
        esphome::climate::CLIMATE_MODE_AUTO
    });

    // Ondersteunde fan modes
    traits.set_supported_fan_modes({
        esphome::climate::CLIMATE_FAN_OFF,
        esphome::climate::CLIMATE_FAN_LOW,
        esphome::climate::CLIMATE_FAN_MEDIUM,
        esphome::climate::CLIMATE_FAN_HIGH,
        esphome::climate::CLIMATE_FAN_AUTO,
        esphome::climate::CLIMATE_FAN_MIDDLE,
        esphome::climate::CLIMATE_FAN_FOCUS,
        esphome::climate::CLIMATE_FAN_DIFFUSE
    });

    // Ondersteunde swing modes
    traits.set_supported_swing_modes({
        esphome::climate::CLIMATE_SWING_OFF,
        esphome::climate::CLIMATE_SWING_VERTICAL,
        esphome::climate::CLIMATE_SWING_HORIZONTAL,
        esphome::climate::CLIMATE_SWING_BOTH
    });

    // Temperatuur instellingen
    traits.set_supports_current_temperature(true);
    traits.set_visual_min_temperature(16.0f);
    traits.set_visual_max_temperature(30.0f);
    traits.set_visual_temperature_step(1.0f);

    return traits;
  }

  void control(const esphome::climate::ClimateCall &call) override {
    ESP_LOGD("Haier", "Control oproep ontvangen");

    // Verwerk mode verandering
    if (call.get_mode().has_value()) {
      auto new_mode = *call.get_mode();
      switch (new_mode) {
        case esphome::climate::CLIMATE_MODE_OFF:
        case esphome::climate::CLIMATE_MODE_COOL:
        case esphome::climate::CLIMATE_MODE_HEAT:
        case esphome::climate::CLIMATE_MODE_DRY:
        case esphome::climate::CLIMATE_MODE_FAN_ONLY:
        case esphome::climate::CLIMATE_MODE_HEAT_COOL:
        case esphome::climate::CLIMATE_MODE_AUTO:
          this->mode = new_mode;
          ESP_LOGD("Haier", "Mode instellen op %d", this->mode);
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde mode: %d", new_mode);
          break;
      }
    }

    // Verwerk target temperatuur
    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      if (temp >= 16.0f && temp <= 30.0f) {
        this->target_temperature = temp;
        ESP_LOGD("Haier", "Target temperatuur instellen op %.1f", this->target_temperature);
      } else {
        ESP_LOGW("Haier", "Target temperatuur %.1f buiten bereik (16-30)", temp);
      }
    }

    // Verwerk fan mode
    if (call.get_fan_mode().has_value()) {
      auto new_fan_mode = *call.get_fan_mode();
      switch (new_fan_mode) {
        case esphome::climate::CLIMATE_FAN_OFF:
        case esphome::climate::CLIMATE_FAN_LOW:
        case esphome::climate::CLIMATE_FAN_MEDIUM:
        case esphome::climate::CLIMATE_FAN_MIDDLE:
        case esphome::climate::CLIMATE_FAN_HIGH:
        case esphome::climate::CLIMATE_FAN_AUTO:
        case esphome::climate::CLIMATE_FAN_FOCUS:
        case esphome::climate::CLIMATE_FAN_DIFFUSE:
          this->fan_mode = new_fan_mode;
          ESP_LOGD("Haier", "Fan mode instellen op %d", this->fan_mode);
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde fan mode: %d", new_fan_mode);
          break;
      }
    }

    // Verwerk swing mode
    if (call.get_swing_mode().has_value()) {
      auto new_swing_mode = *call.get_swing_mode();
      switch (new_swing_mode) {
        case esphome::climate::CLIMATE_SWING_OFF:
        case esphome::climate::CLIMATE_SWING_VERTICAL:
        case esphome::climate::CLIMATE_SWING_HORIZONTAL:
        case esphome::climate::CLIMATE_SWING_BOTH:
          this->swing_mode = new_swing_mode;
          ESP_LOGD("Haier", "Swing mode instellen op %d", this->swing_mode);
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde swing mode: %d", new_swing_mode);
          break;
      }
    }

    // Verstuur bijgewerkte instellingen naar de AC
    send_command();
    this->publish_state();
  }

 protected:
  UARTComponent *uart_ = nullptr;  // UART component voor communicatie
  uint8_t status[64];  // Placeholder voor status array (pas grootte aan indien nodig)
  static const uint8_t TEMPERATURE_OFFSET = 0;  // Voorbeeld offset
  static const uint8_t SET_POINT_OFFSET = 1;   // Voorbeeld offset
  static const uint8_t MODE_OFFSET = 2;        // Voorbeeld offset
  static const uint8_t FAN_OFFSET = 3;         // Voorbeeld offset
  static const uint8_t SWING_OFFSET = 4;       // Voorbeeld offset

  void parse_status() {
    ESP_LOGD("Haier", "Status van AC parsen...");

    // Placeholder: Lees status van UART
    // Voorbeeld: if (!read_ac_status()) {
    //   ESP_LOGW("Haier", "Kon AC status niet lezen");
    //   return;
    // }
    // Simuleer status data voor testen
    status[TEMPERATURE_OFFSET] = 45;  // Voorbeeld: 22.5°C (45/2)
    status[SET_POINT_OFFSET] = 8;     // Voorbeeld: 24°C (8+16)
    status[MODE_OFFSET] = 0x01;       // Voorbeeld: Cool mode
    status[FAN_OFFSET] = 0x04;        // Voorbeeld: High fan
    status[SWING_OFFSET] = 0x00;      // Voorbeeld: Swing off

    // Update temperatuur waarden
    this->current_temperature = status[TEMPERATURE_OFFSET] / 2.0f;
    this->target_temperature = status[SET_POINT_OFFSET] + 16.0f;

    // Update mode
    switch (status[MODE_OFFSET]) {
      case 0x00:
        this->mode = esphome::climate::CLIMATE_MODE_OFF;
        break;
      case 0x01:
        this->mode = esphome::climate::CLIMATE_MODE_COOL;
        break;
      case 0x02:
        this->mode = esphome::climate::CLIMATE_MODE_HEAT;
        break;
      case 0x03:
        this->mode = esphome::climate::CLIMATE_MODE_DRY;
        break;
      case 0x04:
        this->mode = esphome::climate::CLIMATE_MODE_FAN_ONLY;
        break;
      case 0x05:
        this->mode = esphome::climate::CLIMATE_MODE_HEAT_COOL;
        break;
      case 0x06:
        this->mode = esphome::climate::CLIMATE_MODE_AUTO;
        break;
      default:
        ESP_LOGW("Haier", "Onbekende mode: %d", status[MODE_OFFSET]);
        this->mode = esphome::climate::CLIMATE_MODE_OFF;
        break;
    }

    // Update fan mode
    switch (status[FAN_OFFSET]) {
      case 0x01:
        this->fan_mode = esphome::climate::CLIMATE_FAN_LOW;
        break;
      case 0x02:
        this->fan_mode = esphome::climate::CLIMATE_FAN_MEDIUM;
        break;
      case 0x03:
        this->fan_mode = esphome::climate::CLIMATE_FAN_MIDDLE;
        break;
      case 0x04:
        this->fan_mode = esphome::climate::CLIMATE_FAN_HIGH;
        break;
      case 0x05:
        this->fan_mode = esphome::climate::CLIMATE_FAN_AUTO;
        break;
      case 0x06:
        this->fan_mode = esphome::climate::CLIMATE_FAN_FOCUS;
        break;
      case 0x07:
        this->fan_mode = esphome::climate::CLIMATE_FAN_DIFFUSE;
        break;
      default:
        ESP_LOGW("Haier", "Onbekende fan mode: %d", status[FAN_OFFSET]);
        this->fan_mode = esphome::climate::CLIMATE_FAN_LOW;
        break;
    }

    // Update swing mode
    switch (status[SWING_OFFSET]) {
      case 0x00:
        this->swing_mode = esphome::climate::CLIMATE_SWING_OFF;
        break;
      case 0x01:
        this->swing_mode = esphome::climate::CLIMATE_SWING_BOTH;
        break;
      case 0x02:
        this->swing_mode = esphome::climate::CLIMATE_SWING_HORIZONTAL;
        break;
      case 0x03:
        this->swing_mode = esphome::climate::CLIMATE_SWING_VERTICAL;
        break;
      default:
        ESP_LOGW("Haier", "Onbekende swing mode: %d", status[SWING_OFFSET]);
        this->swing_mode = esphome::climate::CLIMATE_SWING_OFF;
        break;
    }

    this->publish_state();
  }

  void send_command() {
    ESP_LOGD("Haier", "Commando sturen naar AC: mode=%d, temp=%.1f, fan=%d, swing=%d",
             this->mode, this->target_temperature, this->fan_mode, this->swing_mode);
    // Placeholder: Implementeer UART commando versturen
    // Voorbeeld: if (this->uart_) {
    //   uint8_t command[64] = {0};
    //   command[0] = this->mode;
    //   command[1] = (uint8_t)(this->target_temperature - 16);
    //   this->uart_->write_array(command, sizeof(command));
    // }
  }
};

}  // namespace haier
}  // namespace esphome
