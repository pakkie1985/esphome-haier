#include "esphome.h"

class Haier : public esphome::climate::Climate, public PollingComponent {
 public:
  void setup() override {
    // Initialisatie
  }

  void loop() override {
    // Eventuele periodieke code
  }

  void parse_status() {
    // Voorbeeld hoe je leden aanspreekt
    this->current_temperature = status[TEMPERATURE_OFFSET] / 2;
    this->target_temperature = status[SET_POINT_OFFSET] + 16;

    switch(status[MODE_OFFSET]) {
      case 0: this->mode = esphome::climate::CLIMATE_MODE_OFF; break;
      case 1: this->mode = esphome::climate::CLIMATE_MODE_HEAT; break;
      case 2: this->mode = esphome::climate::CLIMATE_MODE_COOL; break;
      case 3: this->mode = esphome::climate::CLIMATE_MODE_AUTO; break;
      case 4: this->mode = esphome::climate::CLIMATE_MODE_FAN_ONLY; break;
      case 5: this->mode = esphome::climate::CLIMATE_MODE_DRY; break;
    }

    // Fan mode voorbeeld
    this->fan_mode = esphome::climate::CLIMATE_FAN_AUTO; // pas aan op basis van status

    // Swing mode voorbeeld
    this->swing_mode = esphome::climate::CLIMATE_SWING_BOTH; // pas aan op basis van status

    // State publiceren
    this->publish_state();
  }

  void control(const esphome::climate::ClimateCall &call) override {
    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      this->set_target_temperature(temp);
    }

    if (call.get_mode().has_value()) {
      this->mode = *call.get_mode();
    }

    if (call.get_fan_mode().has_value()) {
      this->fan_mode = *call.get_fan_mode();
    }

    if (call.get_swing_mode().has_value()) {
      this->swing_mode = *call.get_swing_mode();
    }

    this->publish_state();
  }

  esphome::climate::ClimateTraits traits() override {
    esphome::climate::ClimateTraits traits;
    traits.set_supports_current_temperature(true);
    traits.set_supports_two_point_target_temperature(false);
    traits.set_supported_modes({esphome::climate::CLIMATE_MODE_OFF,
                                esphome::climate::CLIMATE_MODE_COOL,
                                esphome::climate::CLIMATE_MODE_HEAT,
                                esphome::climate::CLIMATE_MODE_AUTO,
                                esphome::climate::CLIMATE_MODE_DRY,
                                esphome::climate::CLIMATE_MODE_FAN_ONLY});
    traits.set_supported_fan_modes({esphome::climate::CLIMATE_FAN_OFF,
                                    esphome::climate::CLIMATE_FAN_LOW,
                                    esphome::climate::CLIMATE_FAN_MEDIUM,
                                    esphome::climate::CLIMATE_FAN_HIGH,
                                    esphome::climate::CLIMATE_FAN_AUTO});
    traits.set_supported_swing_modes({esphome::climate::CLIMATE_SWING_OFF,
                                      esphome::climate::CLIMATE_SWING_HORIZONTAL,
                                      esphome::climate::CLIMATE_SWING_VERTICAL,
                                      esphome::climate::CLIMATE_SWING_BOTH});
    return traits;
  }
};
