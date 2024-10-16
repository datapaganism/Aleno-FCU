#pragma once

#include <stdint.h>


const char* PARAM_INPUT = "value";

typedef enum fire_mode
{
  semi = 0,
  automatic = 1,
  burst = 2,
  dmr = 3,
  LAST
} fire_mode_e;

typedef struct settings
{
  int32_t burst_count;
  uint32_t dwell_microseconds;
  int32_t rpm;
  int32_t semi_delay_ms;
  int32_t dmr_delay_ms;
  fire_mode_e fire_mode;
} settings_t;


// PINS

constexpr uint8_t trigger_pin_ground = 0;
constexpr uint8_t trigger_pin = 1;
constexpr uint8_t mosfet_pin = 4;
constexpr uint8_t mosfet_pin_ground = 3;
constexpr uint8_t led_builtin_pin = 8;


// CONFIG
constexpr int32_t debounce_delay_ms = 30;
constexpr int32_t min_cycle_time_ms = 20;

constexpr float min_dwell_time_ms = 3.0;
constexpr float max_dwell_time_ms = 20.0;
constexpr float step_dwell_time_ms = 0.25;

constexpr int32_t min_rpm = 100;
constexpr int32_t max_rpm = 1800;
constexpr int32_t step_rpm = 50;

constexpr int32_t min_burst = 1;
constexpr int32_t max_burst = 10;
constexpr int32_t step_burst = 1;

// MEM ADDRS
constexpr uint32_t settings_address = 0;
constexpr uint32_t settings_flag_address = (sizeof(settings_t) + 1);
constexpr float eeprom_flag_expected = 1.234;


// DEFAULT SETTINGS
settings_t default_settings = { 
  .burst_count = 3,
  .dwell_microseconds = 20,
  .rpm = 900,
  .semi_delay_ms = 140,
  .dmr_delay_ms = 2000,
  .fire_mode = fire_mode_e::semi
};