#pragma once

#include "esphome.h"

using namespace esphome;

class Haier : public climate::Climate, public PollingComponent {
 public:
  // Constructor: update interval
  Haier() : PollingComponent(10000) {}

  // Traits van de climate component
  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supports_target_temperature(true);
    traits.set_supports_fan_mode(true);
    traits.set_supports_swing_mode(true);
    traits.set_min_temperature(16);
    traits.set_max_temperature(30);
    traits.set_temperature_step(1);
    traits.add_fan_mode(climate::FAN_OFF);
    traits.add_fan_mode(climate::FAN_LOW);
    traits.add_fan_mode(climate::FAN_MEDIUM);
    traits.add_fan_mode(climate::FAN_HIGH);
    traits.add_fan_mode(climate::FAN_AUTO);
    traits.add_fan_mode(climate::FAN_FOCUS);
    traits.add_fan_mode(climate::FAN_DIFFUSE);
    traits.add_swing_mode(climate::SWING_OFF);
    traits.add_swing_mode(climate::SWING_VERTICAL);
    traits.add_swing_mode(climate::SWING_HORIZONTAL);
    traits.add_swing_mode(climate::SWING_BOTH);
    return traits;
  }

  // Polling loop: update state van AC
  void update() override {
    parse_status();
  }

  // Functie om status van AC uit te lezen (vul hier je protocol in)
  void parse_status() {
    // Voorbeeld: waarden van je AC uitlezen
    // current_temperature_ = ...;
    // target_temperature_ = ...;
    // mode_ = ...;
    // fan_mode_ = ...;
    // swing_mode_ = ...;
    publish_state();
  }

  // Controle van de climate component
  void control(const climate::ClimateCall &call) override {
    if (call.get_target_temperature().has_value()) {
      target_temperature_ = *call.get_target_temperature();
    }

    if (call.get_mode().has_value()) {
      mode_ = *call.get_mode();
    }

    if (call.get_fan_mode().has_value()) {
      fan_mode_ = *call.get_fan_mode();
    }

    if (call.get_swing_mode().has_value()) {
      swing_mode_ = *call.get_swing_mode();
    }

    // Hier kan je je code toevoegen om de AC fysiek aan te sturen
    send_command_to_ac();
    publish_state();
  }

 protected:
  void send_command_to_ac() {
    // Voeg hier je IR / seriële / wifi commando code toe
  }
};
