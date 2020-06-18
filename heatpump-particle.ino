#include <HeatPump.h>
#include "heatpump-particle.h"
#include "MQTT.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#define HEATPUMP_DEBUG 1

#if HEATPUMP_DEBUG
SerialLogHandler logHandler;
#endif

HeatPump hp;
unsigned long lastPeriodicReport;
MQTT mqttClient("192.168.1.2", 1883, mqttCallback);

bool getPowerSettingBool() {
  return hp.getPowerSettingBool();
}

const char* getModeSetting() {
  return hp.getModeSetting();
}

float getTemperature() {
  return hp.getTemperature();
}

const char* getFanSpeed() {
  return hp.getFanSpeed();
}

const char* getVaneSetting() {
  return hp.getVaneSetting();
}

const char* getWideVaneSetting() {
  return hp.getWideVaneSetting();
}

float getRoomTemperature() {
  return hp.getRoomTemperature();
}

const bool getOperating() {
  return hp.getOperating();
}

const bool isConnected() {
  return hp.isConnected();
}

int setPower(String modeString) {
  if (modeString.equals("ON")) {
    hp.setPowerSetting((bool)true);
    return 0;
  }

  if (modeString.equals("OFF")) {
    hp.setPowerSetting((bool)false);
    return 0;
  }

  return -1;
}

String toJson(heatpumpSettings *settings) {
  DynamicJsonDocument doc(1024);
  doc["power"] = settings->power;
  doc["mode"] = settings->mode;
  doc["temperature_c"] = settings->temperature;
  doc["fan"] = settings->fan;
  doc["vane"] = settings->vane;
  doc["wide_vane"] = settings->wideVane;
  String result;
  serializeJson(doc, result);
  return result;
} 

String toJson(heatpumpStatus *status) {
  DynamicJsonDocument doc(1024);
  doc["room_temperature_c"] = status->roomTemperature;
  doc["is_operating"] = status->operating;
  doc["compressor_frequency"] = status->compressorFrequency;
  String result;
  serializeJson(doc, result);
  return result;
} 

int setTemperatureC(String tempString) {
  float value = tempString.toFloat();
  if (value < 16 || value > 31) {
    return -1;
  }

  hp.setTemperature(value);
  return 0;
}

void setup() {
  Log.info("Starting ...");
  RGB.control(true);

  mqttClient.connect("sparkclient_" + String(Time.now()));
  if (mqttClient.isConnected()) {
    Log.info("Subscribed!");
    mqttClient.subscribe("heatpump/control");
  }

  Particle.variable("power", getPowerSettingBool);
  Particle.variable("mode", getModeSetting);
  Particle.variable("temperature_c", getTemperature);
  Particle.variable("fan_speed", getFanSpeed);
  Particle.variable("vane_setting", getVaneSetting);
  Particle.variable("wide_vane_setting", getWideVaneSetting);

  Particle.variable("room_temperature_c", getRoomTemperature);
  Particle.variable("is_operating", getOperating);
  Particle.variable("is_connected", isConnected);

  Particle.function("setPower", setPower);
  Particle.function("setTemperatureC", setTemperatureC);

  hp.setSettingsChangedCallback(hpSettingsChanged);
  hp.setStatusChangedCallback(hpStatusChanged);
  hp.setPacketCallback(hpPacketDebug);
  hp.enableAutoUpdate();
  hp.enableExternalUpdate();
  hp.connect(&Serial1);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    Log.info("Got MQTT message: %s", p);

    if (!strcmp(p, "RED"))
        RGB.color(255, 0, 0);
    else if (!strcmp(p, "GREEN"))
        RGB.color(0, 255, 0);
    else if (!strcmp(p, "BLUE"))
        RGB.color(0, 0, 255);
    else
        RGB.color(255, 255, 255);
    delay(1000);
}

void hpSettingsChanged() {
  heatpumpSettings newSettings = hp.getSettings();
  const char* settingsJson = toJson(&newSettings).c_str();
  Log.info("--> hpSettingsChanged %s", settingsJson);
  Particle.publish("heatpump/settings-changed", settingsJson);
}

void hpStatusChanged(heatpumpStatus newStatus) {
  const char* statusJson = toJson(&newStatus);
  Log.info("--> hpStatusChanged %s", statusJson);
  Particle.publish("heatpump/status-changed", statusJson);
  lastPeriodicReport = millis();
}

void hpPacketDebug(byte* packet, unsigned int length, char* packetDirection) {
  Log.info("--> hpPacketDebug");
}

void loop() {
  hp.sync();

  if (mqttClient.isConnected()) {
    mqttClient.loop();
  }

  if (lastPeriodicReport == 0 || (millis() - lastPeriodicReport) > PERIODIC_REPORT_INTERVAL_MILLIS) {
    hpStatusChanged(hp.getStatus());
  }
}