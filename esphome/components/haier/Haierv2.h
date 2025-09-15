#pragma once

#include "esphome.h"

using namespace esphome;

class Haier : public climate::Climate, public PollingComponent {
 public:
  // Stel hier je update interval in
  Haier() : PollingComponent(2000) {}

  void setup() override {
    // Setup code hier
  }

  void update() override {
    parse_status();
  }

  // Traits van de unit
  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supported_modes({
        climate::CLIMATE_MODE_OFF,
        climate::CLIMATE_MODE_COOL,
        climate::CLIMATE_MODE_HEAT,
        climate::CLIMATE_MODE_DRY,
        climate::CLIMATE_MODE_FAN_ONLY,
        climate::CLIMATE_MODE_HEAT_COOL,
        climate::CLIMATE_MODE_AUTO
    });
    traits.set_supported_fan_modes({
        climate::CLIMATE_FAN_OFF,
        climate::CLIMATE_FAN_LOW,
        climate::CLIMATE_FAN_MEDIUM,
        climate::CLIMATE_FAN_HIGH,
        climate::CLIMATE_FAN_AUTO
    });
    traits.set_supports_current_temperature(true);
    traits.set_supports_swing_mode(true);
    return traits;
  }

  void control(const climate::ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      auto new_mode = *call.get_mode();
      mode = new_mode;
    }

    if (call.get_target_temperature().has_value()) {
      target_temperature = *call.get_target_temperature();
    }

    if (call.get_fan_mode().has_value()) {
      auto new_fan = *call.get_fan_mode();
      fan_mode = new_fan;
    }

    if (call.get_swing_mode().has_value()) {
      auto new_swing = *call.get_swing_mode();
      swing_mode = new_swing;
    }

    publish_state();
  }

 private:
  // Hier de interne status van je AC
  float current_temperature{0};
  float target_temperature{24};
  climate::ClimateMode mode{climate::CLIMATE_MODE_OFF};
  climate::ClimateFanMode fan_mode{climate::CLIMATE_FAN_AUTO};
  climate::ClimateSwingMode swing_mode{climate::CLIMATE_SWING_OFF};

  void parse_status() {
    // Hier moet je de status van je AC uitlezen en instellen
    // Bijvoorbeeld:
    // current_temperature = sensor_read_temp();
    // mode = ...; fan_mode = ...; swing_mode = ...;

    publish_state();
  }
};
