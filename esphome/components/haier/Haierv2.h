#pragma once

#include "esphome.h"

class Haier : public esphome::climate::Climate, public PollingComponent {
 public:
  Haier() : PollingComponent(2000) {}

  void setup() override {}

  void update() override {
    parse_status();
  }

  esphome::climate::ClimateTraits traits() override {
    auto traits = esphome::climate::ClimateTraits();

    // Ondersteunde modes
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
        esphome::climate::CLIMATE_FAN_AUTO
    });

    // Swing modes
    traits.set_supports_swing_mode(true);

    // Ondersteunt huidige temperatuur
    traits.set_supports_current_temperature(true);

    return traits;
  }

  void control(const esphome::climate::ClimateCall &call) override {
    if (call.get_mode().has_value()) mode = *call.get_mode();
    if (call.get_target_temperature().has_value()) target_temperature = *call.get_target_temperature();
    if (call.get_fan_mode().has_value()) fan_mode = *call.get_fan_mode();
    if (call.get_swing_mode().has_value()) swing_mode = *call.get_swing_mode();
    publish_state();
  }

 private:
  float current_temperature{0};
  float target_temperature{24};
  esphome::climate::ClimateMode mode{esphome::climate::CLIMATE_MODE_OFF};
  esphome::climate::ClimateFanMode fan_mode{esphome::climate::CLIMATE_FAN_AUTO};
  esphome::climate::ClimateSwingMode swing_mode{esphome::climate::CLIMATE_SWING_OFF};

  void parse_status() {
    // Hier je AC-status uitlezen en bovenstaande variabelen bijwerken
    // Voorbeeld:
    // current_temperature = sensor_value;
    // target_temperature = ac_setpoint;
    publish_state();
  }
};
