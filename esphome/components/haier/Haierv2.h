#pragma once

#include "esphome.h"

using namespace esphome;

class Haier : public climate::Climate, public PollingComponent {
 public:
  // Constructor
  Haier() : PollingComponent(15000) {}  // Polling elke 15 seconden

  // ESPHome Climate traits
  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supports_action(true);
    traits.set_supports_modes({climate::MODE_OFF, climate::MODE_COOL, climate::MODE_HEAT, climate::MODE_DRY, climate::MODE_FAN_ONLY, climate::MODE_AUTO});
    traits.set_supports_fan_modes({climate::FAN_OFF, climate::FAN_LOW, climate::FAN_MEDIUM, climate::FAN_HIGH, climate::FAN_AUTO});
    traits.set_supports_swing_modes({climate::SWING_OFF, climate::SWING_VERTICAL, climate::SWING_HORIZONTAL, climate::SWING_BOTH});
    traits.set_supports_target_temperature(true);
    traits.set_visual_min_temperature(16.0f);
    traits.set_visual_max_temperature(30.0f);
    return traits;
  }

  void setup() override {
    // Setup code hier
  }

  void update() override {
    // Poll status van de airco
    parse_status();
  }

  void control(const climate::ClimateCall &call) override {
    // Mode
    if (call.get_mode().has_value()) {
      auto new_mode = *call.get_mode();
      mode_ = new_mode;
    }

    // Fan mode
    if (call.get_fan_mode().has_value()) {
      auto new_fan = *call.get_fan_mode();
      fan_mode_ = new_fan;
    }

    // Swing mode
    if (call.get_swing_mode().has_value()) {
      auto new_swing = *call.get_swing_mode();
      swing_mode_ = new_swing;
    }

    // Target temperature
    if (call.get_target_temperature().has_value()) {
      target_temperature_ = *call.get_target_temperature();
    }

    publish_state();  // Laat ESPHome weten dat de status is veranderd
  }

  void parse_status() {
    // Hier moet je de echte data van de airco uitlezen en omzetten
    // Bijvoorbeeld:
    // current_temperature_ = lees_sensor_temp();
    // target_temperature_ = lees_airco_setpoint();
    // mode_, fan_mode_, swing_mode_ = actuele waarden

    // Voor demo:
    current_temperature_ = 24.0;
    target_temperature_ = 22.0;
    mode_ = climate::MODE_COOL;
    fan_mode_ = climate::FAN_AUTO;
    swing_mode_ = climate::SWING_BOTH;

    publish_state();
  }

  // Optioneel sensoren
  sensor::Sensor *temperature_sensor{nullptr};
  sensor::Sensor *humidity_sensor{nullptr};
};
