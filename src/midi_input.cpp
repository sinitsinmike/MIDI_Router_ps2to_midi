#include "midi_input.h"
#include <Arduino.h>
#include "midi_output.h"
#include "hardware/uart.h"

// ==============================
// Настройки MIDI IN
// ==============================
#define MIDI_RX_PIN 5
#define MIDI_BAUD 31250

// ------------------------------
// Внутренние переменные
// ------------------------------
static uint8_t runningStatus = 0;   // Running Status поддержка
static uint8_t data1 = 0;           // Буфер первого байта
static bool waiting_data1 = false;  // Флаги состояния
static bool waiting_data2 = false;

bool midiThruEnabled = true;        // Флаг разрешения MIDI Thru

// ======================================================
// Инициализация MIDI входа
// ======================================================
void setup_midi_input() {
  uart_init(uart1, MIDI_BAUD);
  gpio_set_function(MIDI_RX_PIN, GPIO_FUNC_UART);
  uart_set_hw_flow(uart1, false, false);
  uart_set_format(uart1, 8, 1, UART_PARITY_NONE);

  // MIDI использует инверсный сигнал (активный LOW)
  uart_set_inverse(uart1, UART_INVERSE_RX);

  Serial.println("[MIDI-IN] Initialized at 31250 baud");
}

// ======================================================
// Основная задача — вызывать из loop()
// ======================================================
void midi_in_task() {
  while (uart_is_readable(uart1)) {
    uint8_t b = uart_getc(uart1);
    process_midi_input(b);
  }
}

// ======================================================
// Парсер входящего MIDI потока
// ======================================================
void process_midi_input(uint8_t b) {
  // Если пришёл статус-байт
  if (b & 0x80) {
    runningStatus = b;
    waiting_data1 = true;
    waiting_data2 = false;
    return;
  }

  // Running Status — используем последний статус
  if (runningStatus == 0)
    return; // нет активного статуса

  uint8_t type = runningStatus & 0xF0;

  if (type == 0xC0 || type == 0xD0) {
    // Одно-байтовые команды (Program Change / Channel Pressure)
    handle_midi_event(runningStatus, b, 0);
    waiting_data1 = false;
    waiting_data2 = false;
  } else {
    // Двух-байтовые (Note, CC и т.д.)
    if (waiting_data1) {
      data1 = b;
      waiting_data1 = false;
      waiting_data2 = true;
    } else if (waiting_data2) {
      handle_midi_event(runningStatus, data1, b);
      waiting_data1 = true;
      waiting_data2 = false;
    }
  }
}

// ======================================================
// Обработка MIDI события (Note, CC, PC...)
// ======================================================
void handle_midi_event(uint8_t st, uint8_t d1, uint8_t d2) {
  uint8_t type = st & 0xF0;
  uint8_t ch = (st & 0x0F) + 1;

  // ----- Отладка -----
#ifdef DEBUG_MIDI
  Serial.printf("[MIDI-IN] 0x%02X %d %d (ch%d)\n", st, d1, d2, ch);
#endif

  // ----- Обработка -----
  switch (type) {
    case 0x90: // Note On
      if (d2 > 0) {
        if (midiThruEnabled) {
          send_midi_usb(st, d1, d2);
          send_midi_uart(st, d1, d2);
          for (int i = 0; i < 10; i++) send_midi_pio(i, st, d1, d2);
        }
      } else {
        if (midiThruEnabled) {
          send_midi_usb(0x80 | ((ch - 1) & 0x0F), d1, 0);
          send_midi_uart(0x80 | ((ch - 1) & 0x0F), d1, 0);
          for (int i = 0; i < 10; i++) send_midi_pio(i, 0x80 | ((ch - 1) & 0x0F), d1, 0);
        }
      }
      break;

    case 0x80: // Note Off
      if (midiThruEnabled) {
        send_midi_usb(st, d1, d2);
        send_midi_uart(st, d1, d2);
        for (int i = 0; i < 10; i++) send_midi_pio(i, st, d1, d2);
      }
      break;

    case 0xB0: // Control Change
      if (midiThruEnabled) {
        send_midi_usb(st, d1, d2);
        send_midi_uart(st, d1, d2);
        for (int i = 0; i < 10; i++) send_midi_pio(i, st, d1, d2);
      }
      break;

    case 0xC0: // Program Change
    case 0xD0: // Channel Pressure
      if (midiThruEnabled) {
        send_midi_usb(st, d1, 0);
        send_midi_uart(st, d1, 0);
        for (int i = 0; i < 10; i++) send_midi_pio(i, st, d1, 0);
      }
      break;

    default:
      // Прочие статусы (Pitch Bend, Aftertouch и т.п.)
      if (midiThruEnabled) {
        send_midi_usb(st, d1, d2);
        send_midi_uart(st, d1, d2);
        for (int i = 0; i < 10; i++) send_midi_pio(i, st, d1, d2);
      }
      break;
  }
}

// ======================================================
// Дополнительные функции управления
// ======================================================
void midi_in_set_thru(bool enabled) {
  midiThruEnabled = enabled;
  Serial.printf("[MIDI-IN] Thru %s\n", enabled ? "enabled" : "disabled");
}

bool midi_in_get_thru() {
  return midiThruEnabled;
}