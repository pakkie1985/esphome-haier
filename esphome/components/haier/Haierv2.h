#pragma once

#include "esphome.h"

namespace esphome {
namespace haier_v2 {

class Haier : public PollingComponent, public climate::Climate {
 public:
  Haier(UARTComponent *parent) : parent_(parent) {}

  // Polling interval
  void setup() override {
    // Initialisatie indien nodig
  }

  void update() override {
    parse_status();
  }

  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supports_target_temperature(true);
    traits.set_supports_fan_mode(true);
    traits.set_supports_swing_mode(true);
    traits.set_supports_modes({climate::CLIMATE_MODE_OFF,
                               climate::CLIMATE_MODE_COOL,
                               climate::CLIMATE_MODE_HEAT,
                               climate::CLIMATE_MODE_DRY,
                               climate::CLIMATE_MODE_FAN_ONLY,
                               climate::CLIMATE_MODE_HEAT_COOL,
                               climate::CLIMATE_MODE_AUTO});
    traits.set_supports_fan_modes({climate::CLIMATE_FAN_OFF,
                                   climate::CLIMATE_FAN_LOW,
                                   climate::CLIMATE_FAN_MEDIUM,
                                   climate::CLIMATE_FAN_HIGH,
                                   climate::CLIMATE_FAN_AUTO});
    traits.set_supports_swing_modes({climate::CLIMATE_SWING_OFF,
                                     climate::CLIMATE_SWING_VERTICAL,
                                     climate::CLIMATE_SWING_HORIZONTAL,
                                     climate::CLIMATE_SWING_BOTH});
    traits.set_min_temperature(16.0);
    traits.set_max_temperature(30.0);
    return traits;
  }

  void control(const climate::ClimateCall &call) override {
    // Mode
    if (call.get_mode().has_value()) {
      mode_ = *call.get_mode();
      publish_state();
    }

    // Target temperature
    if (call.get_target_temperature().has_value()) {
      target_temperature_ = *call.get_target_temperature();
      publish_state();
    }

    // Fan mode
    if (call.get_fan_mode().has_value()) {
      fan_mode_ = *call.get_fan_mode();
      publish_state();
    }

    // Swing mode
    if (call.get_swing_mode().has_value()) {
      swing_mode_ = *call.get_swing_mode();
      publish_state();
    }

    // TODO: hier stuur je data naar je Haier via UART
    // bv: parent_->write(...);
  }

  void parse_status() {
    // TODO: hier je UART data lezen en current temperatuur / mode / fan / swing bijwerken
    // voorbeeld:
    // current_temperature_ = uart_data.temperature;
    // mode_ = uart_data.mode;
    // fan_mode_ = uart_data.fan_mode;
    // swing_mode_ = uart_data.swing_mode;
    // publish_state();
  }

 protected:
  UARTComponent *parent_;

  // interne variabelen
  float target_temperature_{24.0};
  float current_temperature_{24.0};
  climate::ClimateMode mode_{climate::CLIMATE_MODE_OFF};
  climate::ClimateFanMode fan_mode_{climate::CLIMATE_FAN_AUTO};
  climate::ClimateSwingMode swing_mode_{climate::CLIMATE_SWING_OFF};
};

}  // namespace haier_v2
}  // namespace esphome
