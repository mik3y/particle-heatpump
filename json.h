/**
 * Utilities for serializing stuff to JSON.
 */

#include <Particle.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "HeatPump.h"

void jsonToString(JsonDocument* doc, String* result);
void serializeToJson(heatpumpSettings *settings, JsonDocument* doc);
void serializeToJson(heatpumpStatus *status, JsonDocument* doc);
void serializeToJsonString(heatpumpSettings *settings, String* result);
void serializeToJsonString(heatpumpStatus *status, String* result);
