#include "webserial.h"
#include "config_manager.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

String inputBuffer;

void setup_webserial() {
  Serial.println("[WebSerial] Ready");
}

void webserial_task() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        process_web_command(inputBuffer);
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
    }
  }
}

void send_json_config() {
  String json;
  serializeJson(configDoc, json);
  Serial.println(json);
}

void process_web_command(String cmd) {
  cmd.trim();
  if (cmd.startsWith("GET_CONFIG")) {
    send_json_config();
  }
  else if (cmd.startsWith("SAVE_CONFIG")) {
    int space = cmd.indexOf(' ');
    if (space > 0) {
      String json = cmd.substring(space + 1);
      DynamicJsonDocument doc(8192);
      DeserializationError err = deserializeJson(doc, json);
      if (!err) {
        configDoc = doc;
        save_current();
        Serial.println("{\"ok\":\"config_saved\"}");
      } else {
        Serial.printf("{\"error\":\"JSON parse fail: %s\"}\n", err.c_str());
      }
    }
  }
  else if (cmd.startsWith("SAVE_PRESET")) {
    int id = cmd.substring(cmd.lastIndexOf(' ')+1).toInt();
    save_preset(id);
    Serial.printf("{\"ok\":\"preset_saved_%d\"}\n", id);
  }
  else if (cmd.startsWith("LOAD_PRESET")) {
    int id = cmd.substring(cmd.lastIndexOf(' ')+1).toInt();
    load_preset(id);
    send_json_config();
  }
  else {
    Serial.printf("{\"warn\":\"unknown_command\",\"cmd\":\"%s\"}\n", cmd.c_str());
  }
}