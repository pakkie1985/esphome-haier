#pragma once

#include "esphome.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/climate/ climate_traits.h"
#include "esphome/components/climate/ climate_call.h"

using namespace esphome;

class Haier : public climate::Climate, public PollingComponent {
public:
    // Temperatuur, mode, fan en swing variabelen zelf declareren
    float current_temperature;
    float target_temperature;
    climate::ClimateMode mode;
    climate::ClimateFanMode fan_mode;
    climate::ClimateSwingMode swing_mode;

    Haier() : PollingComponent(10000) {} // polling interval 10s

    // Climate eigenschappen voor Home Assistant
    climate::ClimateTraits traits() override {
        auto t = climate::ClimateTraits();
        t.set_supports_current_temperature(true);
        t.set_supports_two_point_target_temperature(false);
        t.set_supports_target_temperature(true);

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
            climate::CLIMATE_FAN_LOW,
            climate::CLIMATE_FAN_MEDIUM,
            climate::CLIMATE_FAN_HIGH,
            climate::CLIMATE_FAN_AUTO
        });

        t.set_supported_swing_modes({
            climate::CLIMATE_SWING_OFF,
            climate::CLIMATE_SWING_VERTICAL,
            climate::CLIMATE_SWING_HORIZONTAL,
            climate::CLIMATE_SWING_BOTH
        });
        return t;
    }

    void setup() override {
        // Initial values
        current_temperature = 20.0;
        target_temperature = 22.0;
        mode = climate::CLIMATE_MODE_OFF;
        fan_mode = climate::CLIMATE_FAN_AUTO;
        swing_mode = climate::CLIMATE_SWING_OFF;
    }

    void update() override {
        // Hier kun je status van je AC ophalen en parsen
        // bv: parseStatus();
    }

    void control(const climate::ClimateCall &call) override {
        if (call.get_mode().has_value()) {
            mode = *call.get_mode();
        }

        if (call.get_fan_mode().has_value()) {
            fan_mode = *call.get_fan_mode();
        }

        if (call.get_swing_mode().has_value()) {
            swing_mode = *call.get_swing_mode();
        }

        if (call.get_target_temperature().has_value()) {
            target_temperature = *call.get_target_temperature();
        }

        // Hier stuur je de waarden naar je AC hardware
        // bv: sendCommand(mode, fan_mode, swing_mode, target_temperature);

        // Publiceer nieuwe status naar Home Assistant
        publish_state();
    }
};
