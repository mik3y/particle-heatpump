#include "json.h"

void jsonToString(JsonDocument* doc, String* result) {
  serializeJson(*doc, *result);
}

void serializeToJson(heatpumpSettings *settings, JsonDocument* doc) {
  (*doc)["power"] = settings->power;
  (*doc)["mode"] = settings->mode;
  (*doc)["temperature_c"] = settings->temperature;
  (*doc)["fan"] = settings->fan;
  (*doc)["vane"] = settings->vane;
  (*doc)["wide_vane"] = settings->wideVane;
} 

void serializeToJson(heatpumpStatus *status, JsonDocument* doc) {
  (*doc)["room_temperature_c"] = status->roomTemperature;
  (*doc)["is_operating"] = status->operating;
  (*doc)["compressor_frequency"] = status->compressorFrequency;
} 

void serializeToJsonString(heatpumpSettings *settings, String* result) {
  StaticJsonDocument<1024> doc;
  serializeToJson(settings, &doc);
  jsonToString(&doc, result);
}

void serializeToJsonString(heatpumpStatus *status, String* result) {
  StaticJsonDocument<1024> doc;
  serializeToJson(status, &doc);
  jsonToString(&doc, result);
}
