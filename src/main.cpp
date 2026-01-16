#include <Arduino.h>
#include "ch376s.h"
#include "midi_input.h"
#include "midi_output.h"
#include "keymap.h"
#include "webserial.h"
#include "config_manager.h"

#include <Adafruit_TinyUSB.h>
#include <LittleFS.h>

// ======================================================
// Глобальные настройки
// ======================================================
#define STATUS_LED 25
#define LOOP_INTERVAL_MS 1  // 1мс — стабильный цикл

unsigned long lastMillis = 0;

// ======================================================
// Инициализация системы
// ======================================================
void setup() {
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========== RP2040 MIDI Router ==========");
  Serial.println("Version 1.0.0 | CodeGPT Build\n");

  // --- LittleFS ---
  if (!LittleFS.begin()) {
    Serial.println("[FS] ❌ Failed to mount LittleFS");
  } else {
    Serial.println("[FS] ✅ Filesystem mounted");
  }

  // --- Конфигурация (JSON + пресеты) ---
  setup_config();

  // --- MIDI OUTPUT (USB + UART + PIO) ---
  setup_midi_output();

  // --- MIDI INPUT (DIN/TRS IN) ---
  setup_midi_input();

  // --- CH376S (USB Keyboard) ---
  setup_ch376s();

  // --- WebSerial (через USB CDC) ---
  setup_webserial();

  // --- Тестовый MIDI сигнал ---
  test_midi_outputs();

  Serial.println("[SYSTEM] ✅ Initialization complete");
  digitalWrite(STATUS_LED, HIGH);
}

// ======================================================
// Главный цикл
// ======================================================
void loop() {
  // WebSerial (JSON обмен)
  webserial_task();

  // Опрос CH376S (HID клавиатура)
  ch376s_task();

  // MIDI вход (DIN/TRS RX)
  midi_in_task();

  // LED heartbeat
  if (millis() - lastMillis >= 500) {
    lastMillis = millis();
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  }

  delay(LOOP_INTERVAL_MS);
}

// ======================================================
// Дополнительно: системные команды по Serial
// ======================================================
void serialEvent() {
  // Позволяет тестировать прямо из Serial Monitor
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "test") {
      test_midi_outputs();
    } 
    else if (cmd == "thru on") {
      midi_in_set_thru(true);
    } 
    else if (cmd == "thru off") {
      midi_in_set_thru(false);
    }
    else if (cmd == "config") {
      print_config_summary();
    } 
    else if (cmd.startsWith("preset")) {
      int id = cmd.substring(6).toInt();
      load_preset(id);
    }
    else {
      Serial.println("[CMD] Unknown command");
    }
  }
}