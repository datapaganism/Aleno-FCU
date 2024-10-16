
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <stdint.h>
#include <algorithm>
#include <cstdlib>


#include "shared.hpp"
#include "index.hpp"

// GLOBALS
bool trigger_pulled = false;
bool fired = false;
settings_t settings;
// uint32_t rps_delay_microseconds;

bool lastSteadyState = LOW;      // the previous steady state from the input pin
bool lastFlickerableState = LOW; // the previous flickerable state from the input pin
unsigned long lastDebounceTime = 0;



void dump_settings()
{
  Serial.printf("------------------------------------\n");
  Serial.printf("mode %i\n", settings.mode);
  Serial.printf("fire_mode %i\n", settings.fire_mode);
  Serial.printf("dwell_step %f\n", settings.dwell_step);
  Serial.printf("delay_step %f\n", settings.delay_step);
  Serial.printf("dwell_ms %f\n", settings.dwell_ms);
  Serial.printf("delay_ms %f\n", settings.delay_ms);
  Serial.printf("dwell_micros %i\n", settings.dwell_micros);
  Serial.printf("delay_micros %i\n", settings.delay_micros);
  Serial.printf("firerate_rps %f\n", settings.firerate_rps);
  Serial.printf("firerate_rpm %f\n", settings.firerate_rpm);
  Serial.printf("firerate_rpm_target %f\n", settings.firerate_rpm_target);
  Serial.printf("burst_count %i\n", settings.burst_count);
  Serial.printf("\n");
}

#if AP_MODE == 1
const char *ssid = "alenofcu";
const char *password = "alenofcu";

DNSServer dnsServer;
IPAddress IP;

// class CaptivePortalHandler : public AsyncWebHandler
// {
// public:
//   CaptivePortalHandler() {}
//   virtual ~CaptivePortalHandler() {}

//   bool canHandle(__unused AsyncWebServerRequest* request) {
//     // request->addInterestingHeader("ANY");
//     return true;
//   }

//   void handleRequest(AsyncWebServerRequest *request)
//   {

//     request->send_P(200, "text/html", index_html, processor);
//   }
// };

#else
const char *ssid = "";
const char *password = "";
#endif


void recalc_settings()
{
  settings.dwell_micros = settings.dwell_ms * 1000;

  if (settings.mode == mode_e::auto_rpm)
  {
    settings.auto_delay_ms = (60000 / settings.firerate_rpm_target);
    settings.delay_micros = settings.auto_delay_ms * 1000;
    settings.firerate_rpm = (60000 / (settings.auto_delay_ms));
    settings.firerate_rps = (1000 / (settings.auto_delay_ms));
  }
  else
  if (settings.mode == mode_e::manual)
  {
    settings.delay_micros = settings.delay_ms * 1000;
    settings.firerate_rpm = (60000 / (settings.delay_ms));
    settings.firerate_rps = (1000 / (settings.delay_ms));
  }
}

AsyncWebServer server(80);

void setupServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    request->send(200, "text/html", index_html, processor);
    Serial.printf("Client Connected\n"); });

  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.mode = value.toInt();
      recalc_settings();

    }
    request->send(200, "text/plain", String(settings.mode)); });

  server.on("/fire_mode", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.fire_mode = value.toInt();
    }
    request->send(200, "text/plain", String(settings.fire_mode)); });

  server.on("/dwell-step", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.dwell_step = value.toFloat();
    }
    request->send(200, "text/plain", String(settings.dwell_step)); });

  server.on("/delay-step", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.delay_step = value.toFloat();
    }
    request->send(200, "text/plain", String(settings.delay_step)); });

  server.on("/dwell-minus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.dwell_ms += (settings.dwell_step * value.toInt());
      settings.dwell_ms = std::clamp(settings.dwell_ms, (float)min_dwell_time_ms, (float)max_dwell_time_ms);
      recalc_settings();
    }
    request->send(200, "text/plain", String(settings.dwell_ms)); });

  server.on("/dwell-plus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.dwell_ms += (settings.dwell_step * value.toInt());
      settings.dwell_ms = std::clamp(settings.dwell_ms, (float)min_dwell_time_ms, (float)max_dwell_time_ms);
      recalc_settings();
    }
    request->send(200, "text/plain", String(settings.dwell_ms)); });

  server.on("/delay-minus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.delay_ms += (settings.delay_step * value.toInt());
      settings.delay_ms = std::clamp(settings.delay_ms, (float)min_cycle_time_ms, (float)100);
      recalc_settings();

    }
    request->send(200, "text/plain", String(settings.delay_ms)); });

  server.on("/delay-plus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.delay_ms += (settings.delay_step * value.toInt());
      settings.delay_ms = std::clamp(settings.delay_ms, (float)min_cycle_time_ms, (float)100);
      recalc_settings();
    }
    request->send(200, "text/plain",  String(settings.delay_ms)); });

  server.on("/rpmtarget-step", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.firerate_rpm_target_step = value.toFloat();
    }
    request->send(200, "text/plain", String(settings.firerate_rpm_target_step)); });

  server.on("/rpmtarget-minus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.firerate_rpm_target += (settings.firerate_rpm_target_step * value.toInt());
      settings.firerate_rpm_target = std::clamp(settings.firerate_rpm_target, (float)min_rpm, (float)max_rpm);
      recalc_settings();

    }
    request->send(200, "text/plain", String(settings.firerate_rpm_target)); });

  server.on("/rpmtarget-plus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.firerate_rpm_target += (settings.firerate_rpm_target_step * value.toInt());
      settings.firerate_rpm_target = std::clamp(settings.firerate_rpm_target, (float)min_rpm, (float)max_rpm);
      recalc_settings();
    }
    request->send(200, "text/plain",  String(settings.firerate_rpm_target)); });

  server.on("/burstcount-minus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.burst_count +=  value.toInt();
      settings.burst_count = std::clamp(settings.burst_count, min_burst, max_burst);
    }
    request->send(200, "text/plain", String(settings.burst_count)); });

  server.on("/burstcount-plus", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.burst_count +=  value.toInt();
      settings.burst_count = std::clamp(settings.burst_count, min_burst, max_burst);
    }
    request->send(200, "text/plain", String(settings.burst_count)); });


    server.on("/firerate", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    request->send(200, "text/plain", String(settings.firerate_rpm)); });


    server.on("/save_config", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.printf("SAVING CONFIG\n");
    EEPROM.put(settings_address,settings);
    EEPROM.commit();
    request->send(200, "text/plain", "OK"); });

  // server.onNotFound([&](AsyncWebServerRequest *request)
  // {
  //   request->send(200, "text/html", index_html, processor);
  // });
}

void init_eeprom(void)
{
  float eeprom_flag;
  EEPROM.get(settings_flag_address, eeprom_flag);
  Serial.printf("flag is %f\n", eeprom_flag);
  if (isnan(eeprom_flag) || (eeprom_flag != eeprom_flag_expected))
  {
    Serial.printf("EEPROM Flag is not initialized.\n");
    eeprom_flag = eeprom_flag_expected;
    EEPROM.put(settings_flag_address, eeprom_flag);
    EEPROM.put(settings_address, default_settings);
    EEPROM.commit();
  }
}

void setup_wifi(void)
{
#if AP_MODE == 1
  WiFi.softAP(ssid, password);
  IP = WiFi.softAPIP();
  Serial.printf("Soft AP IP address: %s\n", IP.toString());
#else
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

#endif

  if (!MDNS.begin("alenofcu"))
  { // Set the hostname to "alenofcu.local"
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
}

void setup()
{
  Serial.begin(MONITOR_SPEED);

  Serial.printf("Starting up\n");

  EEPROM.begin(512);

  pinMode(trigger_pin, INPUT_PULLUP);
  pinMode(trigger_pin_ground, OUTPUT);

  pinMode(mosfet_pin, OUTPUT);
  pinMode(mosfet_pin_ground, OUTPUT);

  pinMode(led_builtin_pin, OUTPUT);

  digitalWrite(trigger_pin_ground, LOW);
  digitalWrite(mosfet_pin_ground, LOW);

  init_eeprom();
  settings = EEPROM.get(settings_address, settings);
  // update_rps_delay(settings.rpm);

  setup_wifi();

  setupServer();

#if AP_MODE == 1
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", IP);
#endif

  // Start server
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
}

/// @brief Fires the mosfet for the duration of total_fire_time_micros, this is done by firing for the duration of dwell then (total_fire_time_micros - dwell)
/// @param total_fire_time_micros
void fire_mosfet(uint64_t total_fire_time_micros)
{
  digitalWrite(led_builtin_pin, LOW);
  digitalWrite(mosfet_pin, HIGH);
  delayMicroseconds(settings.dwell_micros);

  digitalWrite(led_builtin_pin, HIGH);
  digitalWrite(mosfet_pin, LOW);

  uint64_t final_delay = (total_fire_time_micros - settings.dwell_micros);
  delayMicroseconds(final_delay);

  Serial.printf("Dwell micros %i, Delay %i, off_dwell micros %i\n", settings.dwell_micros, settings.delay_micros, total_fire_time_micros);
}

void process_trigger(bool trigger_pulled)
{

  static int32_t burst_shots = 0;

  if (!trigger_pulled)
  {
    fired = false;
    burst_shots = 0;
    return;
  }

  if (fired)
  {
    return;
  }

  dump_settings();

  if (settings.fire_mode == fire_mode_e::burst)
  {
    fire_mosfet(settings.delay_micros);
    burst_shots++;
    if (burst_shots == settings.burst_count)
    {
      fired = true;
      // might not need this, but this will stop trigger spamming
      delayMicroseconds(settings.delay_micros*2);
      return;
    }
  }

  if (settings.fire_mode == fire_mode_e::automatic)
  {
    fire_mosfet(settings.delay_micros);
    return;
  }

  if (settings.fire_mode == fire_mode_e::semi)
  {
    fire_mosfet(settings.delay_micros);
    fired = true;
    return;
  }

  if (settings.fire_mode == fire_mode_e::dmr)
  {
    fire_mosfet(200000);
    fired = true;
    return;
  }
}
void loop()
{

#if AP_MODE == 1
  dnsServer.processNextRequest();
#endif

  trigger_pulled = !digitalRead(trigger_pin);

  if (trigger_pulled != lastFlickerableState)
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
    // save the the last flickerable state
    lastFlickerableState = trigger_pulled;
  }

  if ((millis() - lastDebounceTime) > debounce_delay_ms)
  {
    process_trigger(trigger_pulled);
    lastSteadyState = trigger_pulled;
  }
}
