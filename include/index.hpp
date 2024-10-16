#pragma once
#include <Arduino.h>

#include "index_processor.hpp"

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Alen's FCU</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 300px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
  </style>
</head>
<body>
  <p>Dwell Time: <span id="dwell_text">%SLIDERVALUE_DWELL%</span> (ms)</p>
  <p><input type="range" onchange="updateSlider('dwell_slider', 'dwell_text')" id="dwell_slider" min="%DWELL_MIN%" max="%DWELL_MAX%" value="%SLIDERVALUE_DWELL%" step="%DWELL_STEP%" class="slider"></p>

  <p>RPM: <span id="rpm_text">%SLIDERVALUE_RPM%</span></p>
  <p><input type="range" onchange="updateSlider('rpm_slider', 'rpm_text')" id="rpm_slider" min="%RPM_MIN%" max="%RPM_MAX%" value="%SLIDERVALUE_RPM%" step="%RPM_STEP%" class="slider"></p>

  <p>Burst: <span id="burst_text">%SLIDERVALUE_BURST%</span></p>
  <p><input type="range" onchange="updateSlider('burst_slider', 'burst_text')" id="burst_slider" min="%BURST_MIN%" max="%BURST_MAX%" value="%SLIDERVALUE_BURST%" step="%BURST_STEP%" class="slider"></p>

  <p>Fire Mode: <span id="fire_mode_text">%SLIDERVALUE_FIRE_MODE%</span></p>
  <p><input type="range" onchange="updateSlider('fire_mode_slider', 'fire_mode_text')" id="fire_mode_slider" min="%FIRE_MODE_MIN%" max="%FIRE_MODE_MAX%" value="%SLIDERVALUE_FIRE_MODE%" step="%FIRE_MODE_STEP%" class="slider"></p>

  <button type="button" onclick="sendSaveConfig()">Save Config</button>

<script>

function sendSaveConfig() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/save_config", true);
  xhr.send();
}


function updateSlider(input_id, output_id) {
  var value = document.getElementById(input_id).value;
  document.getElementById(output_id).innerHTML = value;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/"+input_id+"?value="+value, true);
  xhr.send();
}


</script>
</body>
</html>
)rawliteral";