#pragma once

#include "esphome.h"

// Alias voor de climate namespace
namespace cl = esphome::climate;

class Haier : public cl::Climate, public PollingComponent {
 public:
  Haier() : PollingComponent(2000) {}

  void setup() override {}
  
  void update() override {
    parse_status();
  }

  cl::ClimateTraits traits() override {
    auto traits = cl::ClimateTraits();
    
    // Ondersteunde modi
    traits.set_supported_modes({
        cl::CLIMATE_MODE_OFF,
        cl::CLIMATE_MODE_COOL,
        cl::CLIMATE_MODE_HEAT,
        cl::CLIMATE_MODE_DRY,
        cl::CLIMATE_MODE_FAN_ONLY,
        cl::CLIMATE_MODE_HEAT_COOL,
        cl::CLIMATE_MODE_AUTO
    });

    // Ondersteunde fan modes
    traits.set_supported_fan_modes({
        cl::CLIMATE_FAN_OFF,
        cl::CLIMATE_FAN_LOW,
        cl::CLIMATE_FAN_MEDIUM,
        cl::CLIMATE_FAN_HIGH,
        cl::CLIMATE_FAN_AUTO
    });

    // Swing modes
    traits.set_supports_swing_mode(true);

    // Huidige temperatuur
    traits.set_supports_current_temperature(true);

    return traits;
  }

  void control(const cl::ClimateCall &call) override {
    if (call.get_mode().has_value()) mode = *call.get_mode();
    if (call.get_target_temperature().has_value()) target_temperature = *call.get_target_temperature();
    if (call.get_fan_mode().has_value()) fan_mode = *call.get_fan_mode();
    if (call.get_swing_mode().has_value()) swing_mode = *call.get_swing_mode();
    publish_state();
  }

 private:
  float current_temperature{0};
  float target_temperature{24};
  cl::ClimateMode mode{cl::CLIMATE_MODE_OFF};
  cl::ClimateFanMode fan_mode{cl::CLIMATE_FAN_AUTO};
  cl::ClimateSwingMode swing_mode{cl::CLIMATE_SWING_OFF};

  void parse_status() {
    // Hier je AC status uitlezen en bovenstaande variabelen bijwerken
    // voorbeeld:
    // current_temperature = sensor_value;
    // target_temperature = ac_setpoint;
    publish_state();
  }
};
