#pragma once

#include "esphome.h"

using namespace esphome;
using namespace esphome::climate;

class Haier : public Climate, public PollingComponent {
 public:
  // Constructor
  Haier() : PollingComponent(2000) {}  // poll elke 2 seconden

  // Traits van je klimaat apparaat
  ClimateTraits traits() override {
    ClimateTraits t;
    t.set_supports_current_temperature(true);
    t.set_supports_target_temperature(true);
    t.modes = {CLIMATE_MODE_OFF, CLIMATE_MODE_COOL, CLIMATE_MODE_HEAT,
               CLIMATE_MODE_DRY, CLIMATE_MODE_FAN_ONLY, CLIMATE_MODE_HEAT_COOL, CLIMATE_MODE_AUTO};
    t.fan_modes = {CLIMATE_FAN_OFF, CLIMATE_FAN_LOW, CLIMATE_FAN_MEDIUM,
                   CLIMATE_FAN_HIGH, CLIMATE_FAN_AUTO, CLIMATE_FAN_FOCUS, CLIMATE_FAN_DIFFUSE};
    t.swing_modes = {CLIMATE_SWING_OFF, CLIMATE_SWING_VERTICAL, CLIMATE_SWING_HORIZONTAL, CLIMATE_SWING_BOTH};
    t.min_temperature = 16.0f;
    t.max_temperature = 30.0f;
    t.supports_two_point_target_temperature = false;
    return t;
  }

  // Polling functie
  void update() override {
    // Hier zou je status van de airco uitlezen via je protocol
    // voorbeeld:
    float current_temp = 22.5;  // dummywaarde
    this->publish_state(current_temp);
  }

  // Controle functie
  void control(const ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      this->mode = *call.get_mode();
    }

    if (call.get_target_temperature().has_value()) {
      this->set_target_temperature(*call.get_target_temperature());
    }

    if (call.get_fan_mode().has_value()) {
      this->fan_mode = *call.get_fan_mode();
    }

    if (call.get_swing_mode().has_value()) {
      this->swing_mode = *call.get_swing_mode();
    }

    this->publish_state();  // belangrijk: update Home Assistant
  }
};
