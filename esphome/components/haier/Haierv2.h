#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"

using namespace esphome;

class Haier : public climate::Climate, public PollingComponent {
 public:
  Haier() : PollingComponent(15000) {}  // Poll elke 15 seconden

  void setup() override {
    // Hier init code van je airco
  }

  void update() override {
    // Hier status uitlezen van je airco
    parse_status();
  }

  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supports_action(true);
    traits.set_supports_modes({
        climate::CLIMATE_MODE_OFF,
        climate::CLIMATE_MODE_COOL,
        climate::CLIMATE_MODE_HEAT,
        climate::CLIMATE_MODE_DRY,
        climate::CLIMATE_MODE_FAN_ONLY,
        climate::CLIMATE_MODE_AUTO
    });
    traits.set_supports_fan_modes({
        climate::CLIMATE_FAN_OFF,
        climate::CLIMATE_FAN_LOW,
        climate::CLIMATE_FAN_MEDIUM,
        climate::CLIMATE_FAN_HIGH,
        climate::CLIMATE_FAN_AUTO
    });
    traits.set_supports_swing_modes({
        climate::CLIMATE_SWING_OFF,
        climate::CLIMATE_SWING_VERTICAL,
        climate::CLIMATE_SWING_HORIZONTAL,
        climate::CLIMATE_SWING_BOTH
    });
    traits.set_supports_target_temperature(true);
    traits.set_visual_min_temperature(16.0f);
    traits.set_visual_max_temperature(30.0f);
    return traits;
  }

  void control(const climate::ClimateCall &call) override {
    // Mode
    if (call.get_mode().has_value()) {
      mode_ = *call.get_mode();
    }

    // Fan mode
    if (call.get_fan_mode().has_value()) {
      fan_mode_ = *call.get_fan_mode();
    }

    // Swing mode
    if (call.get_swing_mode().has_value()) {
      swing_mode_ = *call.get_swing_mode();
    }

    // Target temperature
    if (call.get_target_temperature().has_value()) {
      target_temperature_ = *call.get_target_temperature();
    }

    publish_state();  // update ESPHome
  }

  void parse_status() {
    // Dummy voorbeeld, lees hier je airco status in
    current_temperature_ = 24.0;
    target_temperature_ = 22.0;
    mode_ = climate::CLIMATE_MODE_COOL;
    fan_mode_ = climate::CLIMATE_FAN_AUTO;
    swing_mode_ = climate::CLIMATE_SWING_BOTH;

    publish_state();
  }

  // Sensoren (optioneel)
  sensor::Sensor *temperature_sensor{nullptr};
  sensor::Sensor *humidity_sensor{nullptr};
};
