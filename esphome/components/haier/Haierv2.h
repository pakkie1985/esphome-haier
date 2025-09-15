#pragma once

#include "esphome.h"

using namespace esphome;
using namespace esphome::climate;

#define MIN_SET_TEMPERATURE 16
#define MAX_SET_TEMPERATURE 30
#define MIN_VALID_INTERNAL_TEMP 10
#define MAX_VALID_INTERNAL_TEMP 50

// Mode & fan definitions
#define MODE_OFFSET 14
#define MODE_MSK 0xF0
#define MODE_AUTO 0x00
#define MODE_DRY 0x40
#define MODE_COOL 0x20
#define MODE_HEAT 0x80
#define MODE_FAN 0xC0

#define FAN_MSK 0x0F
#define FAN_LOW 0x03
#define FAN_MID 0x02
#define FAN_HIGH 0x01
#define FAN_AUTO 0x05

// Swing offsets
#define HORIZONTAL_SWING_OFFSET 19
#define VERTICAL_SWING_OFFSET 13
#define HORIZONTAL_SWING_AUTO 0x07
#define VERTICAL_SWING_AUTO 0x0C
#define HORIZONTAL_SWING_CENTER 0x00
#define VERTICAL_SWING_CENTER 0x06

// Temperature & status offsets
#define TEMPERATURE_OFFSET 22
#define SET_POINT_OFFSET 12
#define STATUS_DATA_OFFSET 17
#define POWER_BIT 0
#define QUIET_BIT 3
#define PURIFY_BIT 1
#define AUTO_FAN_MAX_BIT 4

// Commands
#define COMMAND_OFFSET 9
#define RESPONSE_POLL 2
#define POLY 0xa001

class Haier : public Climate, public PollingComponent {
private:
    byte status[47] = {0};
    byte lastCRC = 0;
    bool first_status_received = false;

    // State storage
    byte climate_mode_fan_speed = FAN_AUTO;
    byte climate_mode_setpoint = 0x0A;
    byte fan_mode_fan_speed = FAN_HIGH;
    byte fan_mode_setpoint = 0x08;

    // Initialisation & polling commands
    byte initialization_1[13] = {0xFF,0xFF,0x0A,0x0,0x0,0x0,0x0,0x0,0x00,0x61,0x00,0x07,0x72};
    byte initialization_2[13] = {0xFF,0xFF,0x08,0x40,0x0,0x0,0x0,0x0,0x0,0x70,0xB8,0x86,0x41};
    byte poll[15] = {0xFF,0xFF,0x0A,0x40,0x00,0x00,0x00,0x00,0x00,0x01,0x4D,0x01,0x99,0xB3,0xB4};
    byte control_command[25] = {0xFF,0xFF,0x14,0x40,0x00,0x00,0x00,0x00,0x00,0x01,0x60,0x01,0x09,0x08,0x25,0x00,0x02,0x00,0x00,0x06,0x00,0x00,0x03,0x0B,0x70};

    // Helper functions
    void setHvacMode(byte mode) {
        control_command[MODE_OFFSET] &= ~MODE_MSK;
        control_command[MODE_OFFSET] |= mode;
    }

    byte getHvacMode() { return status[MODE_OFFSET] & MODE_MSK; }

    void setFanSpeed(byte fan) {
        control_command[MODE_OFFSET] &= ~FAN_MSK;
        control_command[MODE_OFFSET] |= fan;
    }

    byte getFanSpeed() { return status[MODE_OFFSET] & FAN_MSK; }

    void setTemperature(byte temp) { control_command[SET_POINT_OFFSET] = temp; }
    byte getTemperature() { return status[SET_POINT_OFFSET]; }

    void setSwing(byte horizontal, byte vertical) {
        control_command[HORIZONTAL_SWING_OFFSET] = horizontal;
        control_command[VERTICAL_SWING_OFFSET] = vertical;
    }

    void setPower(bool on) {
        if (on)
            control_command[STATUS_DATA_OFFSET] |= (1 << POWER_BIT);
        else
            control_command[STATUS_DATA_OFFSET] &= ~(1 << POWER_BIT);
    }

    bool getPower() { return status[STATUS_DATA_OFFSET] & (1 << POWER_BIT); }

    void sendData(byte* message, byte size) {
        byte crc_offset = 2 + message[2];
        message[crc_offset] = getChecksum(message, size);
        word crc16_val = crc16(0, &(message[2]), crc_offset - 2);
        message[crc_offset + 1] = (crc16_val >> 8) & 0xFF;
        message[crc_offset + 2] = crc16_val & 0xFF;
        Serial.write(message, size);
    }

    byte getChecksum(const byte* message, size_t size) {
        byte crc = 0;
        byte pos = 2 + message[2];
        for (int i = 2; i < pos; i++) crc += message[i];
        return crc;
    }

    unsigned crc16(unsigned crc, unsigned char* buf, size_t len) {
        while (len--) {
            crc ^= *buf++;
            for (int i = 0; i < 8; i++)
                crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        }
        return crc;
    }

    void parseStatus() {
        if (getChecksum(status, sizeof(status)) != status[2 + status[2]]) return;
        lastCRC = status[2 + status[2]];
        current_temperature = status[TEMPERATURE_OFFSET] / 2.0f;
        target_temperature = status[SET_POINT_OFFSET] + 16;
        first_status_received = true;

        // Update internal control states
        setHvacMode(getHvacMode());
        setFanSpeed(getFanSpeed());
        setTemperature(getTemperature());

        // Map to ESPHome climate values
        if (!getPower())
            mode = CLIMATE_MODE_OFF;
        else {
            switch (getHvacMode()) {
                case MODE_COOL: mode = CLIMATE_MODE_COOL; break;
                case MODE_HEAT: mode = CLIMATE_MODE_HEAT; break;
                case MODE_DRY: mode = CLIMATE_MODE_DRY; break;
                case MODE_FAN: mode = CLIMATE_MODE_FAN_ONLY; break;
                case MODE_AUTO: default: mode = CLIMATE_MODE_HEAT_COOL; break;
            }
            switch (getFanSpeed()) {
                case FAN_LOW: fan_mode = CLIMATE_FAN_LOW; break;
                case FAN_MID: fan_mode = CLIMATE_FAN_MEDIUM; break;
                case FAN_HIGH: fan_mode = CLIMATE_FAN_HIGH; break;
                case FAN_AUTO: default: fan_mode = CLIMATE_FAN_AUTO; break;
            }
        }
        publish_state();
    }

public:
    Haier() : PollingComponent(5000) {}

    void setup() override {
        Serial.begin(9600);
        delay(1000);
        Serial.write(initialization_1, sizeof(initialization_1));
        delay(1000);
        Serial.write(initialization_2, sizeof(initialization_2));
    }

    void loop() override {
        byte data[47];
        if (Serial.available() > 0) {
            if (Serial.read() != 255) return;
            if (Serial.read() != 255) return;
            data[0] = 255; data[1] = 255;
            Serial.readBytes(data + 2, sizeof(data) - 2);
            if (data[COMMAND_OFFSET] == RESPONSE_POLL) {
                memcpy(status, data, sizeof(status));
                parseStatus();
            }
        }
    }

    void update() override { Serial.write(poll, sizeof(poll)); }

protected:
    ClimateTraits traits() override {
        auto traits = climate::ClimateTraits();
        traits.set_supported_modes({CLIMATE_MODE_HEAT_COOL, CLIMATE_MODE_HEAT, CLIMATE_MODE_COOL, CLIMATE_MODE_DRY, CLIMATE_MODE_FAN_ONLY, CLIMATE_MODE_OFF});
        traits.set_supported_fan_modes({CLIMATE_FAN_AUTO, CLIMATE_FAN_LOW, CLIMATE_FAN_MEDIUM, CLIMATE_FAN_HIGH});
        traits.set_visual_min_temperature(MIN_SET_TEMPERATURE);
        traits.set_visual_max_temperature(MAX_SET_TEMPERATURE);
        traits.set_visual_temperature_step(1.0f);
        traits.set_supports_current_temperature(true);
        traits.set_supported_swing_modes({CLIMATE_SWING_OFF, CLIMATE_SWING_BOTH, CLIMATE_SWING_VERTICAL, CLIMATE_SWING_HORIZONTAL});
        return traits;
    }

    void control(const ClimateCall &call) override {
        if (!first_status_received) return;
        if (call.get_mode().has_value()) {
            ClimateMode new_mode = *call.get_mode();
            switch (new_mode) {
                case CLIMATE_MODE_OFF: setPower(false); break;
                case CLIMATE_MODE_HEAT_COOL: setPower(true); setHvacMode(MODE_AUTO); break;
                case CLIMATE_MODE_HEAT: setPower(true); setHvacMode(MODE_HEAT); break;
                case CLIMATE_MODE_COOL: setPower(true); setHvacMode(MODE_COOL); break;
                case CLIMATE_MODE_DRY: setPower(true); setHvacMode(MODE_DRY); break;
                case CLIMATE_MODE_FAN_ONLY: setPower(true); setHvacMode(MODE_FAN); break;
                default: break;
            }
            sendData(control_command, sizeof(control_command));
            mode = new_mode;
            publish_state();
        }
        if (call.get_target_temperature().has_value()) {
            float temp = *call.get_target_temperature();
            setTemperature(temp - 16);
            sendData(control_command, sizeof(control_command));
            target_temperature = temp;
            publish_state();
        }
        if (call.get_fan_mode().has_value()) {
            switch (*call.get_fan_mode()) {
                case CLIMATE_FAN_LOW: setFanSpeed(FAN_LOW); break;
                case CLIMATE_FAN_MEDIUM: setFanSpeed(FAN_MID); break;
                case CLIMATE_FAN_HIGH: setFanSpeed(FAN_HIGH); break;
                case CLIMATE_FAN_AUTO: default: setFanSpeed(FAN_AUTO); break;
            }
            sendData(control_command, sizeof(control_command));
        }
    }
};
