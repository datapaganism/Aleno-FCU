#pragma once

#include "shared.hpp"
#include <Arduino.h>

extern settings_t settings;

String processor(const String& var)
{
  if (var == "SLIDERVALUE_DWELL"){
    return String(settings.dwell_microseconds / 1000);
  }

  if (var == "SLIDERVALUE_RPM"){
    return String(settings.rpm);
  }

  if (var == "SLIDERVALUE_BURST"){
    return String(settings.burst_count);
  }

  if (var == "SLIDERVALUE_FIRE_MODE"){
    return String(settings.fire_mode);
  }



  if (var == "DWELL_MIN"){
    return String(min_dwell_time_ms);
  }

  if (var == "DWELL_MAX"){
    return String(max_dwell_time_ms);
  }

  if (var == "DWELL_STEP"){
    return String(step_dwell_time_ms);
  }


  if (var == "RPM_MIN"){
    return String(min_rpm);
  }

  if (var == "RPM_MAX"){
    return String(max_rpm);
  }

  if (var == "RPM_STEP"){
    return String(step_rpm);
  }


  if (var == "BURST_MIN"){
    return String(min_burst);
  }

  if (var == "BURST_MAX"){
    return String(max_burst);
  }

  if (var == "BURST_STEP"){
    return String(step_burst);
  }

  if (var == "FIRE_MODE_MIN"){
    return String(fire_mode_e::semi);
  }

  if (var == "FIRE_MODE_MAX"){
    return String(int(fire_mode_e::LAST - 1));
  }

  if (var == "FIRE_MODE_STEP"){
    return String(1);
  }


  return String();
}
