#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/climate/climate_traits.h"

using namespace esphome;
using namespace esphome::climate;

class Haier : public PollingComponent, public climate::Climate {
public:
    // Constructor
    Haier() {}

    // Polling interval
    void setup() override {
        // Initialisatie bij opstarten
        ESP_LOGD("Haier", "Setup complete");
    }

    void loop() override {}

    // Traits (Wat deze climate component ondersteunt)
    climate::ClimateTraits traits() override {
        climate::ClimateTraits t;
        t.set_supports_current_temperature(true);
        t.set_supported_modes({
            climate::CLIMATE_MODE_OFF,
            climate::CLIMATE_MODE_COOL,
            climate::CLIMATE_MODE_HEAT,
            climate::CLIMATE_MODE_DRY,
            climate::CLIMATE_MODE_FAN_ONLY,
            climate::CLIMATE_MODE_AUTO
        });
        t.set_supported_fan_modes({
            climate::CLIMATE_FAN_OFF,
            climate::CLIMATE_FAN_ON,
            climate::CLIMATE_FAN_LOW,
            climate::CLIMATE_FAN_MEDIUM,
            climate::CLIMATE_FAN_HIGH,
            climate::CLIMATE_FAN_AUTO,
            climate::CLIMATE_FAN_FOCUS,
            climate::CLIMATE_FAN_DIFFUSE
        });
        t.set_supported_swing_modes({
            climate::CLIMATE_SWING_OFF,
            climate::CLIMATE_SWING_VERTICAL,
            climate::CLIMATE_SWING_HORIZONTAL,
            climate::CLIMATE_SWING_BOTH
        });
        return t;
    }

    // Control functie: ontvangt commando's van Home Assistant
    void control(const climate::ClimateCall &call) override {
        if (call.get_mode().has_value()) {
            mode = *call.get_mode();
        }

        if (call.get_target_temperature().has_value()) {
            target_temperature = *call.get_target_temperature();
        }

        if (call.get_fan_mode().has_value()) {
            fan_mode = *call.get_fan_mode();
        }

        if (call.get_swing_mode().has_value()) {
            swing_mode = *call.get_swing_mode();
        }

        // Update state naar Home Assistant
        this->publish_state();
    }

    // Simuleer het uitlezen van status van AC (hier kan je eigen logica toevoegen)
    void update() override {
        // Stel hier current_temperature in, bijvoorbeeld van een sensor
        current_temperature = 22.0; // voorbeeldwaarde
        publish_state();
    }

private:
    float target_temperature = 24.0;
    climate::ClimateMode mode = climate::CLIMATE_MODE_OFF;
    climate::ClimateFanMode fan_mode = climate::CLIMATE_FAN_AUTO;
    climate::ClimateSwingMode swing_mode = climate::CLIMATE_SWING_OFF;
};
