#pragma once

#include <stdint.h>


/*

  if (dwell > 63.157) cannot hit target

  shot delay = 63.157 - dwell


  need to write python preprocessor to take index.html and palce it inbetween rawliteral for index hpp
*/

const char* PARAM_INPUT = "value";

typedef enum fire_mode
{
  semi = 0,
  automatic = 1,
  burst = 2,
  dmr = 3,
} fire_mode_e;

typedef enum mode
{
  auto_rpm = 0,
  manual = 1,
} mode_e;

typedef struct settings
{
  uint8_t mode;
  uint8_t fire_mode;
  float dwell_step;
  float delay_step; 
  float dwell_ms;
  float delay_ms;
  float auto_delay_ms;
  uint64_t dwell_micros;
  uint64_t delay_micros;
  float firerate_rps;
  float firerate_rpm;
  float firerate_rpm_target;
  float firerate_rpm_target_step; 
  int32_t burst_count;

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

settings_t default_settings{
    .mode = 0,
    .fire_mode = 0,
    .dwell_step = 1,
    .delay_step = 1,
    .dwell_ms = 3,
    .delay_ms = 20,
    .dwell_micros = 0,
    .delay_micros = 0,
    .firerate_rps = 0,
    .firerate_rpm = 0,
    .firerate_rpm_target = 900,
    .firerate_rpm_target_step = 10,
    .burst_count = 3};
