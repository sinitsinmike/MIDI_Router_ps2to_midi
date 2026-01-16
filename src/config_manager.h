#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

// ==========================
// Конфигурация хранения
// ==========================
#define MAX_PRESETS 3
#define CONFIG_PATH "/config_current.json"

// ==========================
// Структура маппинга клавиши
// ==========================
struct KeyConfig {
  String type;    // "note" или "cc"
  int value;      // номер ноты или CC
  String port;    // "USB", "DIN", "A"…"J"
  int channel;    // MIDI-канал (1–16)
};

// ==========================
// Глобальные переменные
// ==========================
extern DynamicJsonDocument configDoc;

// ==========================
// Функции управления
// ==========================
void setup_config();
void create_default_config();
void save_current();
void load_current();
void save_preset(uint8_t id);
void load_preset(uint8_t id);
void print_config_summary();
KeyConfig get_key_config(const String &hid);