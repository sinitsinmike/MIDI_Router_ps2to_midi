#include "config_manager.h"
#include <LittleFS.h>

DynamicJsonDocument configDoc(8192);

// ======================================================
// Инициализация и базовая загрузка конфигурации
// ======================================================
void setup_config() {
  Serial.println("[CONFIG] Mounting LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println("[CONFIG] ❌ LittleFS mount failed");
    return;
  }

  if (!LittleFS.exists(CONFIG_PATH)) {
    Serial.println("[CONFIG] No config found. Creating default...");
    create_default_config();
    save_current();
  } else {
    load_current();
  }

  print_config_summary();
}

// ======================================================
// Создание дефолтного JSON-конфига
// ======================================================
void create_default_config() {
  configDoc.clear();

  // дефолтная карта клавиш (C4–E4)
  JsonObject n1 = configDoc.createNestedObject("0x1D");
  n1["type"] = "note";
  n1["value"] = 60;
  n1["port"] = "USB";
  n1["channel"] = 1;

  JsonObject n2 = configDoc.createNestedObject("0x1B");
  n2["type"] = "note";
  n2["value"] = 62;
  n2["port"] = "USB";
  n2["channel"] = 1;

  JsonObject n3 = configDoc.createNestedObject("0x06");
  n3["type"] = "note";
  n3["value"] = 64;
  n3["port"] = "USB";
  n3["channel"] = 1;
}

// ======================================================
// Сохранение текущего JSON в LittleFS
// ======================================================
void save_current() {
  File f = LittleFS.open(CONFIG_PATH, "w");
  if (!f) {
    Serial.println("[CONFIG] ❌ Save failed");
    return;
  }
  serializeJson(configDoc, f);
  f.close();
  Serial.println("[CONFIG] 💾 Saved current config");
}

// ======================================================
// Загрузка текущего JSON из LittleFS
// ======================================================
void load_current() {
  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) {
    Serial.println("[CONFIG] ❌ Load failed (file missing)");
    create_default_config();
    save_current();
    return;
  }

  DeserializationError err = deserializeJson(configDoc, f);
  f.close();

  if (err) {
    Serial.printf("[CONFIG] ⚠️ JSON Error: %s. Resetting to default.\n", err.c_str());
    create_default_config();
    save_current();
  } else {
    Serial.println("[CONFIG] ✅ Loaded current config");
  }
}

// ======================================================
// Управление пресетами
// ======================================================
void save_preset(uint8_t id) {
  if (id < 1 || id > MAX_PRESETS) return;
  String path = "/preset" + String(id) + ".json";

  File f = LittleFS.open(path, "w");
  if (!f) {
    Serial.printf("[CONFIG] ❌ Preset %d save failed\n", id);
    return;
  }
  serializeJson(configDoc, f);
  f.close();
  Serial.printf("[CONFIG] 💾 Preset %d saved (%s)\n", id, path.c_str());
}

void load_preset(uint8_t id) {
  if (id < 1 || id > MAX_PRESETS) return;
  String path = "/preset" + String(id) + ".json";

  if (!LittleFS.exists(path)) {
    Serial.printf("[CONFIG] ⚠️ Preset %d missing. Ignored.\n", id);
    return;
  }

  File f = LittleFS.open(path, "r");
  if (!f) {
    Serial.printf("[CONFIG] ❌ Cannot open preset %d\n", id);
    return;
  }

  configDoc.clear();
  DeserializationError err = deserializeJson(configDoc, f);
  f.close();

  if (err) {
    Serial.printf("[CONFIG] ⚠️ Error loading preset %d: %s\n", id, err.c_str());
  } else {
    Serial.printf("[CONFIG] ✅ Preset %d loaded\n", id);
    save_current(); // обновляем активный
  }
}

// ======================================================
// Вспомогательные функции
// ======================================================
KeyConfig get_key_config(const String &hid) {
  KeyConfig k{};
  if (configDoc.containsKey(hid)) {
    JsonObject obj = configDoc[hid].as<JsonObject>();
    k.type = obj["type"].as<String>();
    k.value = obj["value"].as<int>();
    k.port = obj["port"].as<String>();
    k.channel = obj["channel"].as<int>();
  } else {
    // fallback
    k.type = "note";
    k.value = 60;
    k.port = "USB";
    k.channel = 1;
  }
  return k;
}

void print_config_summary() {
  Serial.println("[CONFIG] Summary:");
  for (JsonPair kv : configDoc.as<JsonObject>()) {
    String key = kv.key().c_str();
    JsonObject o = kv.value().as<JsonObject>();
    Serial.printf("  HID %s → %s %d (Port %s, Ch %d)\n",
                  key.c_str(),
                  o["type"].as<const char*>(),
                  o["value"].as<int>(),
                  o["port"].as<const char*>(),
                  o["channel"].as<int>());
  }
}