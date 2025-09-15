#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/api/enums.pb.h"

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
        esphome::api::enums::CLIMATE_MODE_OFF,
        esphome::api::enums::CLIMATE_MODE_COOL,
        esphome::api::enums::CLIMATE_MODE_HEAT,
        esphome::api::enums::CLIMATE_MODE_DRY,
        esphome::api::enums::CLIMATE_MODE_FAN_ONLY,
        esphome::api::enums::CLIMATE_MODE_HEAT_COOL,
        esphome::api::enums::CLIMATE_MODE_AUTO
    });

    // Ondersteunde fan modes
    traits.set_supported_fan_modes({
        esphome::api::enums::CLIMATE_FAN_OFF,
        esphome::api::enums::CLIMATE_FAN_LOW,
        esphome::api::enums::CLIMATE_FAN_MEDIUM,
        esphome::api::enums::CLIMATE_FAN_HIGH,
        esphome::api::enums::CLIMATE_FAN_AUTO
    });

    // Swing modes
    traits.set_supports_swing_mode(true);

    // Ondersteunt huidige temperatuur
    traits.set_supports_current_temperature(true);

    return traits;
  }

  void control(const esphome::climate::ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      this->mode_ = *call.get_mode();
    }
    if (call.get_target_temperature().has_value()) {
      this->target_temperature_ = *call.get_target_temperature();
    }
    if (call.get_fan_mode().has_value()) {
      this->fan_mode_ = *call.get_fan_mode();
    }
    if (call.get_swing_mode().has_value()) {
      this->swing_mode_ = *call.get_swing_mode();
    }
    this->publish_state();
  }

 private:
  void parse_status() {
    // Hier je AC-status uitlezen en ESPHome variabelen bijwerken
    // Bijvoorbeeld:
    // this->current_temperature_ = sensor_value;
    // this->target_temperature_ = ac_setpoint;
    this->publish_state();
  }
};
