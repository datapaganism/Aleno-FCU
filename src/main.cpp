
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <stdint.h>


#include "shared.hpp"
#include "index.hpp"

// GLOBALS
bool trigger_pulled = false;
bool fired = false;
settings_t settings;
uint32_t rps_delay_microseconds;

bool lastSteadyState = LOW;       // the previous steady state from the input pin
bool lastFlickerableState = LOW;  // the previous flickerable state from the input pin
unsigned long lastDebounceTime = 0;   

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


uint32_t rpm_to_rps_delay(int32_t rpm)
{
  return uint32_t(1000 / (float(rpm) / 60));
}

int32_t rps_delay_to_rpm(int32_t rps_delay)
{
  return (rps_delay * 1000 * 600);
}

void update_rps_delay(int32_t rpm)
{
  settings.rpm = rpm;
  rps_delay_microseconds = rpm_to_rps_delay(rpm) * 1000;
}

int32_t calc_max_rpm(float dwell_microseconds)
{
  int32_t actual_delay;

  for (size_t rpm = max_rpm; rpm > min_rpm; rpm -= step_rpm)
  {
    actual_delay = rpm_to_rps_delay(rpm) - dwell_microseconds;
    if (actual_delay > min_cycle_time_ms)
    {
      return rpm;
    }
  }

  return min_rpm;
}

AsyncWebServer server(80);

void setupServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "text/html", index_html, processor);
    Serial.printf("Client Connected\n");  
  });

  server.on("/semi_delay", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.semi_delay_ms = value.toInt();
    }
    request->send(200, "text/plain", "OK"); 
  });

  server.on("/burst_slider", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.burst_count = value.toInt();
    }
    request->send(200, "text/plain", "OK"); 
  });

  server.on("/rpm_slider", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.rpm = value.toInt();
      update_rps_delay(settings.rpm);
      Serial.printf("MAX RPM CALC %i\n", calc_max_rpm(settings.dwell_microseconds));
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/dwell_slider", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      settings.dwell_microseconds =  uint32_t(value.toFloat() * 1000);
      Serial.printf("MAX RPM CALC %i\n", calc_max_rpm(settings.dwell_microseconds));
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/fire_mode_slider", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String value;
    if (request->hasParam(PARAM_INPUT))
    {
      value = request->getParam(PARAM_INPUT)->value();
      int32_t val = value.toInt();
      if (val < fire_mode_e::LAST)
      {
        settings.fire_mode = fire_mode_e(val);
        Serial.printf("set fire_mode: %i\n",settings.fire_mode);
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/save_config", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.printf("SAVING CONFIG\n");
    EEPROM.put(settings_address,settings);
    EEPROM.commit();
    request->send(200, "text/plain", "OK");
  });

  server.onNotFound([&](AsyncWebServerRequest *request)
  {
    request->send(200, "text/html", index_html, processor);
  });

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
  Serial.printf("Soft AP IP address: %s\n",IP.toString());
#else
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  
#endif

  if (!MDNS.begin("alenofcu")) {   // Set the hostname to "alenofcu.local"
      Serial.println("Error setting up MDNS responder!");
      while(1) {
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
}

void setup()
{
  Serial.begin(MONITOR_SPEED);

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
  update_rps_delay(settings.rpm);

  setup_wifi();


  setupServer();

#if AP_MODE == 1
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", IP);
#endif


  // Start server
  server.begin();
}

void fire_mosfet(uint32_t off_delay_microseconds)
{
  digitalWrite(led_builtin_pin, LOW);
  digitalWrite(mosfet_pin, HIGH);
  delayMicroseconds(settings.dwell_microseconds);
  
  digitalWrite(led_builtin_pin, HIGH);
  digitalWrite(mosfet_pin, LOW);
  delayMicroseconds(off_delay_microseconds - settings.dwell_microseconds);

  Serial.printf("Dwell micros %i, off_delay micros %i\n", settings.dwell_microseconds, (off_delay_microseconds - settings.dwell_microseconds) );
}

void process_trigger(bool trigger_pulled)
{
  if (!trigger_pulled)
  {
    fired = false;
    return;
  }

  if (fired)
  {
    return;
  }

  Serial.printf("firing fire_mode: %i, ", settings.fire_mode);


  if (settings.fire_mode == fire_mode_e::burst)
  {
    for (int i = 0; i < settings.burst_count; i++)
    {
      fire_mosfet(rps_delay_microseconds);
      fired = true;
    }
    delayMicroseconds((settings.semi_delay_ms * 1000) - rps_delay_microseconds);
    return;
  }

  if (settings.fire_mode == fire_mode_e::automatic)
  {
    fire_mosfet(rps_delay_microseconds);
    return;
  }

  if (settings.fire_mode == fire_mode_e::semi)
  {
    fire_mosfet(settings.semi_delay_ms * 1000);
    fired = true;
    return;
  }

  if (settings.fire_mode == fire_mode_e::dmr)
  {
    fire_mosfet(settings.dmr_delay_ms * 1000);
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
