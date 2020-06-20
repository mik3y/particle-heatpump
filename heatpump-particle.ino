/**
 * heatpump-particle: Firmware app for controlling a Mitsubishi heatpump
 * with a Particle.io device.
 */

#include "config.h"
#include "json.h"

#include <HeatPump.h>
#include <MQTT.h>
#include <Arduino.h>

#if HEATPUMP_DEBUG
SerialLogHandler logHandler;
#endif

#define FIRMWARE_VERSION "v0.1.0"

#define POWER_ON "ON"
#define POWER_OFF "OFF"

#define MODE_HEAT "HEAT"
#define MODE_COOL "COOL"
#define MODE_FAN "FAN"
#define MODE_DRY "DRY"
#define MODE_AUTO "AUTO"

#define FAN_SPEED_1 "1"
#define FAN_SPEED_2 "2"
#define FAN_SPEED_3 "3"
#define FAN_SPEED_4 "4"
#define FAN_SPEED_AUTO "AUTO"
#define FAN_SPEED_QUIET "QUIET"

#define VANE_DIRECTION_1 "1"
#define VANE_DIRECTION_2 "2"
#define VANE_DIRECTION_3 "3"
#define VANE_DIRECTION_4 "4"
#define VANE_DIRECTION_5 "5"
#define VANE_DIRECTION_SWING "SWING"
#define VANE_DIRECTION_AUTO "AUTO"

#define WIDE_VANE_DIRECTION_VERY_LEFT "<<"
#define WIDE_VANE_DIRECTION_LEFT "<"
#define WIDE_VANE_DIRECTION_MIDDLE "|"
#define WIDE_VANE_DIRECTION_RIGHT ">"
#define WIDE_VANE_DIRECTION_VERY_RIGHT ">>"
#define WIDE_VANE_DIRECTION_SPREAD "<>"
#define WIDE_VANE_DIRECTION_SWING "SWING"

#define MQTT_TOPIC_DEVICE_ALL "all"

HeatPump hp;
unsigned long lastPeriodicReport;
MQTT* mqttClient = NULL;

retained String mqttBrokerHost = DEFAULT_MQTT_BROKER_HOST;
retained int mqttBrokerPort = DEFAULT_MQTT_BROKER_PORT;
retained String deviceName = "heatpump-unknown";

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

void logEvent(String msg) {
  Log.info(msg);
  Particle.publish("heatpump/log", (char *) msg.c_str(), PRIVATE);
}

void logEvent(char * msg) {
  logEvent(String(msg));
}

void triggerMqttReconnectIfNeeded() {
  int disconnected = 0;
  int created = 0;
  if (mqttClient != NULL) {
    disconnected = 1;
    delete mqttClient;
    mqttClient = NULL;
  }
  if (!mqttBrokerHost.equals("")) {
    created = 1;
    mqttClient = new MQTT((char *) mqttBrokerHost.c_str(), mqttBrokerPort, mqttCallback);
  }

  if (disconnected && created) {
    logEvent(String::format("MQTT broker settings changed, reconnecting: %s @ %s:%i", deviceName.c_str(), mqttBrokerHost, mqttBrokerPort));
  } else if (created) {
    logEvent(String::format("MQTT broker settings changed, connecting: %s @ %s:%i", deviceName.c_str(), mqttBrokerHost, mqttBrokerPort));
  } else if (disconnected) {
    logEvent("MQTT broker settings changed, no new host. Disconnected.");
  }
}

void handleParticleDeviceNameUpdate(const char *topic, const char *data) {
  char buf[strlen(data)+1];
  strcpy(buf, data);
  String newDeviceName = String(buf);
  if (deviceName.equals(newDeviceName)) {
    return;
  }
  deviceName = newDeviceName;
  logEvent(String::format("Name updated: %s", newDeviceName.c_str()));
  triggerMqttReconnectIfNeeded();
}

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

const char* getMqttBrokerHost() {
  return mqttBrokerHost.c_str();
}

int setMqttBrokerHost(String brokerHost) {
  mqttBrokerHost = brokerHost;
  triggerMqttReconnectIfNeeded();
  return 0;
}

int setMqttBrokerPort(String brokerPort) {
  int intVal = (int) brokerPort.toInt();
  mqttBrokerPort = intVal;
  triggerMqttReconnectIfNeeded();
  return 0;
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

int setMode(String inputString) {
  if (inputString.equals(MODE_AUTO) ||
    inputString.equals(MODE_COOL) ||
    inputString.equals(MODE_DRY) ||
    inputString.equals(MODE_FAN) ||
    inputString.equals(MODE_HEAT)) {
      hp.setModeSetting(inputString);
      return 0;
  }
  return -1;
}

int setTemperatureC(String tempString) {
  float value = tempString.toFloat();
  if (value < 16 || value > 31) {
    return -1;
  }

  hp.setTemperature(value);
  return 0;
}

int setFanSpeed(String inputString) {
  if (inputString.equals(FAN_SPEED_1) ||
    inputString.equals(FAN_SPEED_2) ||
    inputString.equals(FAN_SPEED_3) ||
    inputString.equals(FAN_SPEED_4) ||
    inputString.equals(FAN_SPEED_AUTO) ||
    inputString.equals(FAN_SPEED_QUIET)) {
      hp.setFanSpeed(inputString);
      return 0;
  }
  return -1;
}

int setVaneDirection(String inputString) {
  if (inputString.equals(VANE_DIRECTION_1) ||
    inputString.equals(VANE_DIRECTION_2) ||
    inputString.equals(VANE_DIRECTION_3) ||
    inputString.equals(VANE_DIRECTION_4) ||
    inputString.equals(VANE_DIRECTION_5) ||
    inputString.equals(VANE_DIRECTION_SWING) ||
    inputString.equals(VANE_DIRECTION_AUTO)) {
      hp.setVaneSetting(inputString);
      return 0;
  }
  return -1;
}

int setWideVaneDirection(String inputString) {
  if (inputString.equals(WIDE_VANE_DIRECTION_VERY_LEFT) ||
    inputString.equals(WIDE_VANE_DIRECTION_LEFT) ||
    inputString.equals(WIDE_VANE_DIRECTION_MIDDLE) ||
    inputString.equals(WIDE_VANE_DIRECTION_RIGHT) ||
    inputString.equals(WIDE_VANE_DIRECTION_VERY_RIGHT) ||
    inputString.equals(WIDE_VANE_DIRECTION_SPREAD) ||
    inputString.equals(WIDE_VANE_DIRECTION_SWING)) {
      hp.setWideVaneSetting(inputString);
      return 0;
  }
  return -1;
}

void serviceMqtt() {
  if (mqttClient == NULL) {
    return;
  }

  if (!mqttClient->isConnected()) {
    mqttClient->connect(deviceName);
    if (mqttClient->isConnected()) {
      logEvent("Connected to MQTT broker.");
      publishAll();
      mqttClient->subscribe("heatpump/+/control/power");
      mqttClient->subscribe("heatpump/+/control/mode");
      mqttClient->subscribe("heatpump/+/control/temperature-c");
      mqttClient->subscribe("heatpump/+/control/fan-speed");
      mqttClient->subscribe("heatpump/+/control/vane-setting");
      mqttClient->subscribe("heatpump/+/control/wide-vane-setting");
    }
  }

  if (mqttClient->isConnected()) {
    mqttClient->loop();
  }
}

void setup() {
  logEvent("Starting up...");
  RGB.control(true);

  Particle.subscribe("particle/device/name", handleParticleDeviceNameUpdate);
  Particle.publish("particle/device/name", PRIVATE);

  Particle.variable("power", getPowerSettingBool);
  Particle.variable("mode", getModeSetting);
  Particle.variable("temperature_c", getTemperature);
  Particle.variable("fan_speed", getFanSpeed);
  Particle.variable("vane_setting", getVaneSetting);
  Particle.variable("wide_vane_setting", getWideVaneSetting);

  Particle.variable("room_temperature_c", getRoomTemperature);
  Particle.variable("is_operating", getOperating);
  Particle.variable("is_connected", isConnected);

  Particle.variable("mqtt_broker_host", mqttBrokerHost);
  Particle.variable("mqtt_broker_port", mqttBrokerPort);

  Particle.function("setPower", setPower);
  Particle.function("setMode", setMode);
  Particle.function("setTemperatureC", setTemperatureC);
  Particle.function("setFanSpeed", setFanSpeed);
  Particle.function("setVaneDirection", setVaneDirection);
  Particle.function("setWideVaneDirection", setWideVaneDirection);

  Particle.function("setMqttBrokerHost", setMqttBrokerHost);
  Particle.function("setMqttBrokerPort", setMqttBrokerPort);

  hp.setSettingsChangedCallback(hpSettingsChanged);
  hp.setStatusChangedCallback(hpStatusChanged);
  hp.setPacketCallback(hpPacketDebug);
  hp.enableAutoUpdate();
  hp.enableExternalUpdate();
  hp.connect(&Serial1);
  logEvent("Initialization complete!");
}

String getDeviceNameFromTopic(char* topicName) {
  String topic = String(topicName);
  int firstIdx = topic.indexOf("/");
  int secondIdx = topic.indexOf("/", firstIdx + 1);
  if (firstIdx < 0 || secondIdx <= firstIdx) {
    return String("");
  }
  return topic.substring(firstIdx + 1, secondIdx);
}

String getCommandNameFromTopic(char* topicName) {
  String topic = String(topicName);
  int lastIdx = topic.lastIndexOf("/");
  if (lastIdx < 0) {
    return String("");
  }
  return topic.substring(lastIdx + 1);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicDeviceName = getDeviceNameFromTopic(topic);
  if (!topicDeviceName.equals(deviceName) && !topicDeviceName.equals(MQTT_TOPIC_DEVICE_ALL)) {
    logEvent(String::format("RX MQTT [%s]: ignorming message meant for device %s", topic, topicDeviceName.c_str()));
    return;
  }

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = NULL;
  logEvent(String::format("RX MQTT [%s]: %s", topic, message));

  String commandName = getCommandNameFromTopic(topic);
  String command = String(message);
  if (commandName.equals("power")) {
    setPower(command);
  } else if (commandName.equals("mode")) {
    setMode(command);
  } else if (commandName.equals("temperature-c")) {
    setTemperatureC(command);
  } else if (commandName.equals("fan-speed")) {
    setFanSpeed(command);
  } else if (commandName.equals("vane-setting")) {
    setVaneDirection(command);
  } else if (commandName.equals("wide-vane-setting")) {
    setWideVaneDirection(command);
  }
}

void publishMqtt(const char* subtopicName, const char* value) {
  if (mqttClient == NULL) {
    return;
  }
  const char* topicName = String::format("heatpump/%s/%s", deviceName.c_str(), subtopicName).c_str();
  mqttClient->publish(topicName, value);
}

void publishMqtt(const char* subtopicName, String* message) {
  publishMqtt(subtopicName, message->c_str());
}

void publishStatus() {
  heatpumpStatus status = hp.getStatus();
  String jsonStr;
  serializeToJsonString(&status, &jsonStr);

  publishMqtt("status/room-temperature-c", String(status.roomTemperature).c_str());
  publishMqtt("status/is-operating", String(status.operating ? "true" : "false").c_str());
  publishMqtt("status/firmware-version", String(FIRMWARE_VERSION).c_str());
  publishMqtt("status", &jsonStr);

  Particle.publish("heatpump/status-changed", jsonStr.c_str(), PRIVATE);

  Log.info("--> Published status: %s", jsonStr.c_str());
}

void publishSettings() {
  heatpumpSettings settings = hp.getSettings();
  String jsonStr;
  serializeToJsonString(&settings, &jsonStr);

  publishMqtt("settings/power", settings.power);
  publishMqtt("settings/mode", settings.mode);
  publishMqtt("settings/temperature-c", String(settings.temperature).c_str());
  publishMqtt("settings/fan", settings.fan);
  publishMqtt("settings/vane", settings.vane);
  publishMqtt("settings/wide-vane", settings.wideVane);
  publishMqtt("settings/is-connected", String(settings.connected ? "true" : "false"));
  publishMqtt("settings", &jsonStr);

  Particle.publish("heatpump/settings-changed", jsonStr.c_str(), PRIVATE);

  Log.info("--> Published settings: %s", jsonStr.c_str());
}

void publishAll() {
  publishSettings();
  publishStatus();
  lastPeriodicReport = millis();
}

void hpSettingsChanged() {
  publishSettings();
}

void hpStatusChanged(heatpumpStatus newStatus) {
  publishStatus();
}

void hpPacketDebug(byte* packet, unsigned int length, char* packetDirection) {
  // Log.info("--> hpPacketDebug");
}

void loop() {
  hp.sync();
  serviceMqtt();

  if (lastPeriodicReport == 0 || (millis() - lastPeriodicReport) > PERIODIC_REPORT_INTERVAL_MILLIS) {
    publishAll();
  }
}