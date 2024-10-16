#pragma once

#include "shared.hpp"
#include <Arduino.h>

extern settings_t settings;

String processor(const String &var)
{

  if (var == "DELAY-TEXT")
  {
    return String(settings.delay_ms);
  }

  if (var == "DWELL-TEXT")
  {
    return String(settings.dwell_ms);
  }

  if (var == "FIRERATE-TEXT")
  {
    return String(settings.firerate_rpm);
  }

  if (var == "BURSTCOUNT-TEXT")
  {
    return String(settings.burst_count);
  }

  if (var == "RPMTARGET-TEXT")
  {
    return String(settings.firerate_rpm_target);
  }


  return String();
}
