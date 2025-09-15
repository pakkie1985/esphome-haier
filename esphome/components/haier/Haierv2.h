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

    esphome::api::enums::ClimateMode new_mode = esphome::api::enums::CLIMATE_MODE_OFF; // Default

    if (call.get_mode().has_value()) {
      new_mode = *call.get_mode();
    }

    // Logic uit je error-log: Switch voor mode, set command bytes (placeholder)
    uint8_t command[20] = {0}; // Pas grootte aan aan protocol

    switch (new_mode) {
      case esphome::api::enums::CLIMATE_MODE_OFF:
        command[POWER_OFFSET] = 0;
        break;
      case esphome::api::enums::CLIMATE_MODE_HEAT_COOL:
        command[MODE_OFFSET] = 5; // Voorbeeld, pas aan
        command[POWER_OFFSET] = 1;
        break;
      case esphome::api::enums::CLIMATE_MODE_HEAT:
        command[MODE_OFFSET] = 1;
        command[POWER_OFFSET] = 1;
        break;
      case esphome::api::enums::CLIMATE_MODE_DRY:
        command[MODE_OFFSET] = 2;
        command[POWER_OFFSET] = 1;
        break;
      case esphome::api::enums::CLIMATE_MODE_FAN_ONLY:
        command[MODE_OFFSET] = 3;
        command[POWER_OFFSET] = 1;
        break;
      case esphome::api::enums::CLIMATE_MODE_COOL:
        command[MODE_OFFSET] = 0;
        command[POWER_OFFSET] = 1;
        break;
      case esphome::api::enums::CLIMATE_MODE_AUTO:
        command[MODE_OFFSET] = 4;
        command[POWER_OFFSET] = 1;
        break;
      default:
        ESP_LOGW("Haier", "Niet ondersteunde mode: %d", new_mode);
        return;
    }

    this->mode = new_mode;

    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      if (temp >= 16.0f && temp <= 30.0f) {
        command[SET_POINT_OFFSET] = static_cast<uint8_t>(temp - 16);
        this->target_temperature = temp;
      } else {
        ESP_LOGW("Haier", "Target temperatuur %.1f buiten bereik", temp);
      }
    }

    if (call.get_fan_mode().has_value()) {
      auto new_fan_mode = *call.get_fan_mode();
      switch (new_fan_mode) {
        case esphome::api::enums::CLIMATE_FAN_LOW:
          command[FAN_OFFSET] = 1;
          break;
        case esphome::api::enums::CLIMATE_FAN_MIDDLE:
          command[FAN_OFFSET] = 3;
          break;
        case esphome::api::enums::CLIMATE_FAN_MEDIUM:
          command[FAN_OFFSET] = 2;
          break;
        case esphome::api::enums::CLIMATE_FAN_HIGH:
          command[FAN_OFFSET] = 4;
          break;
        case esphome::api::enums::CLIMATE_FAN_AUTO:
          command[FAN_OFFSET] = 5;
          break;
        case esphome::api::enums::CLIMATE_FAN_ON:
          command[FAN_OFFSET] = 6;
          break;
        case esphome::api::enums::CLIMATE_FAN_OFF:
          command[FAN_OFFSET] = 0;
          break;
        case esphome::api::enums::CLIMATE_FAN_FOCUS:
          command[FAN_OFFSET] = 7;
          break;
        case esphome::api::enums::CLIMATE_FAN_DIFFUSE:
          command[FAN_OFFSET] = 8;
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde fan mode: %d", new_fan_mode);
          break;
      }
      this->fan_mode = new_fan_mode;
    }

    if (call.get_swing_mode().has_value()) {
      auto new_swing_mode = *call.get_swing_mode();
      switch (new_swing_mode) {
        case esphome::api::enums::CLIMATE_SWING_OFF:
          command[SWING_OFFSET] = 0;
          break;
        case esphome::api::enums::CLIMATE_SWING_VERTICAL:
          command[SWING_OFFSET] = 2;
          break;
        case esphome::api::enums::CLIMATE_SWING_HORIZONTAL:
          command[SWING_OFFSET] = 3;
          break;
        case esphome::api::enums::CLIMATE_SWING_BOTH:
          command[SWING_OFFSET] = 1;
          break;
        default:
          ESP_LOGW("Haier", "Niet ondersteunde swing mode: %d", new_swing_mode);
          break;
      }
      this->swing_mode = new_swing_mode;
    }

    send_command(command);
    this->publish_state();
  }

 protected:
  esphome::uart::UARTComponent *uart_ = nullptr;
  uint8_t status[20]; // Pas grootte aan aan protocol

  static const uint8_t POWER_OFFSET = 0; // Voorbeeld offsets, pas aan
  static const uint8_t MODE_OFFSET = 1;
  static const uint8_t FAN_OFFSET = 2;
  static const uint8_t SWING_OFFSET = 3;
  static const uint8_t TEMPERATURE_OFFSET = 4;
  static const uint8_t SET_POINT_OFFSET = 5;

  void parse_status() {
    ESP_LOGD("Haier", "Status van AC parsen...");

    // Lees status via UART (implementeer dit, bijv. met checksum)
    if (this->uart_ && this->uart_->available()) {
      this->uart_->read_array(status, sizeof(status));
    } else {
      ESP_LOGW("Haier", "Geen status beschikbaar");
      return;
    }

    this->current_temperature = status[TEMPERATURE_OFFSET] / 2.0f;
    this->target_temperature = status[SET_POINT_OFFSET] + 16.0f;

    // Mode logic uit je error-log (aangepast aan switch)
    if (status[POWER_OFFSET] == 0) {
      this->mode = esphome::api::enums::CLIMATE_MODE_OFF;
    } else {
      uint8_t mode_byte = status[MODE_OFFSET];
      if (mode_byte == 0) {
        this->mode = esphome::api::enums::CLIMATE_MODE_AUTO; // Voorbeeld
      } else if (mode_byte == 1) {
        this->mode = esphome::api::enums::CLIMATE_MODE_COOL;
      } else if (mode_byte == 2) {
        this->mode = esphome::api::enums::CLIMATE_MODE_HEAT;
      } else if (mode_byte == 3) {
        this->mode = esphome::api::enums::CLIMATE_MODE_DRY;
      } else if (mode_byte == 4) {
        this->mode = esphome::api::enums::CLIMATE_MODE_FAN_ONLY;
      } else if (mode_byte == 5) {
        this->mode = esphome::api::enums::CLIMATE_MODE_HEAT_COOL;
      } else {
        this->mode = esphome::api::enums::CLIMATE_MODE_AUTO;
      }
    }

    // Fan mode logic (aangepast aan switch)
    uint8_t fan_byte = status[FAN_OFFSET];
    if (fan_byte == 1) {
      this->fan_mode = esphome::api::enums::CLIMATE_FAN_LOW;
    } else if (fan_byte == 2) {
      this->fan_mode = esphome::api::enums::CLIMATE_FAN_MEDIUM;
    } else if (fan_byte == 3) {
      this->fan_mode = esphome::api::enums::CLIMATE_FAN_HIGH;
    } else if (fan_byte == 4) {
      this->fan_mode = esphome::api::enums::CLIMATE_FAN_AUTO;
    } else if (fan_byte == 5) {
      this->fan_mode = esphome::api::enums::CLIMATE_FAN_MIDDLE;
    } else {
      this->fan_mode = esphome::api::enums::CLIMATE_FAN_AUTO;
    }

    // Swing mode logic
    uint8_t swing_byte = status[SWING_OFFSET];
    if (swing_byte == 0) {
      this->swing_mode = esphome::api::enums::CLIMATE_SWING_OFF;
    } else if (swing_byte == 1) {
      this->swing_mode = esphome::api::enums::CLIMATE_SWING_BOTH;
    } else if (swing_byte == 2) {
      this->swing_mode = esphome::api::enums::CLIMATE_SWING_HORIZONTAL;
    } else if (swing_byte == 3) {
      this->swing_mode = esphome::api::enums::CLIMATE_SWING_VERTICAL;
    } else {
      this->swing_mode = esphome::api::enums::CLIMATE_SWING_OFF;
    }

    this->publish_state();
  }

  void send_command(uint8_t command[]) {
    // Voeg checksum toe als nodig, dan verstuur via UART
    ESP_LOGD("Haier", "Commando sturen...");
    if (this->uart_) {
      this->uart_->write_array(command, sizeof(command));
    }
  }
};

}  // namespace haier
}  // namespace esphome
