#include "midi_output.h"
#include <Arduino.h>
#include "Adafruit_TinyUSB.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "midi_uart_tx.pio.h"

// ======================================================
// Конфигурация интерфейсов
// ======================================================

// --- USB MIDI ---
Adafruit_USBD_MIDI usb_midi;

// --- DIN MIDI (UART1 TX) ---
#define DIN_TX_PIN 4
#define MIDI_BAUD 31250

// --- 10 TRS MIDI OUT (PIO) ---
const uint midi_tx_pins[10] = {6,8,10,12,14,16,18,20,22,26};
PIO pio_a = pio0;
PIO pio_b = pio1;
uint sm_ports[10];

// ======================================================
// Инициализация USB, UART и PIO
// ======================================================
void setup_midi_output() {
  // 1️⃣ USB MIDI
  usb_midi.begin();
  Serial.println("[MIDI] USB interface ready");

  // 2️⃣ UART1 (DIN)
  uart_init(uart1, MIDI_BAUD);
  gpio_set_function(DIN_TX_PIN, GPIO_FUNC_UART);
  Serial.println("[MIDI] DIN TX on GP4");

  // 3️⃣ PIO TX (10 TRS портов)
  uint offset0 = pio_add_program(pio_a, &midi_uart_tx_program);
  uint offset1 = pio_add_program(pio_b, &midi_uart_tx_program);

  for (int i = 0; i < 10; i++) {
    PIO pio = (i < 5) ? pio_a : pio_b;
    uint sm = pio_claim_unused_sm(pio, true);
    sm_ports[i] = sm;

    midi_uart_tx_program_init(
      pio,
      sm,
      (i < 5 ? offset0 : offset1),
      midi_tx_pins[i],
      MIDI_BAUD
    );
  }

  Serial.println("[MIDI] 10x TRS PIO ports initialized");
  Serial.println("[MIDI] Output system ready\n");
}

// ======================================================
// Отправка MIDI сообщений
// ======================================================

// --- USB MIDI ---
void send_midi_usb(uint8_t status, uint8_t data1, uint8_t data2) {
  uint8_t msg[3] = {status, data1, data2};
  usb_midi.write(msg, 3);
  usb_midi.flush();
}

// --- DIN UART ---
void send_midi_uart(uint8_t status, uint8_t data1, uint8_t data2) {
  uart_putc_raw(uart1, status);
  uart_putc_raw(uart1, data1);
  uart_putc_raw(uart1, data2);
}

// --- TRS PIO port ---
void send_midi_pio(uint8_t port, uint8_t status, uint8_t data1, uint8_t data2) {
  if (port >= 10) return;
  PIO pio = (port < 5) ? pio_a : pio_b;
  uint sm = sm_ports[port];

  // Отправляем байты в PIO FIFO
  pio_sm_put_blocking(pio, sm, status);
  pio_sm_put_blocking(pio, sm, data1);
  pio_sm_put_blocking(pio, sm, data2);
}

// ======================================================
// Вспомогательные функции
// ======================================================

// --- Отправить NoteOn на все порты ---
void noteOn_all(uint8_t note, uint8_t vel) {
  uint8_t st = 0x90;
  send_midi_usb(st, note, vel);
  send_midi_uart(st, note, vel);
  for (int i = 0; i < 10; i++) send_midi_pio(i, st, note, vel);
}

// --- Отправить NoteOff на все порты ---
void noteOff_all(uint8_t note) {
  uint8_t st = 0x80;
  send_midi_usb(st, note, 0);
  send_midi_uart(st, note, 0);
  for (int i = 0; i < 10; i++) send_midi_pio(i, st, note, 0);
}

// --- Отправить Control Change ---
void cc_all(uint8_t cc, uint8_t val, uint8_t ch = 1) {
  uint8_t st = 0xB0 | ((ch - 1) & 0x0F);
  send_midi_usb(st, cc, val);
  send_midi_uart(st, cc, val);
  for (int i = 0; i < 10; i++) send_midi_pio(i, st, cc, val);
}

// --- Program Change ---
void programChange_all(uint8_t prog, uint8_t ch = 1) {
  uint8_t st = 0xC0 | ((ch - 1) & 0x0F);
  send_midi_usb(st, prog, 0);
  send_midi_uart(st, prog, 0);
  for (int i = 0; i < 10; i++) send_midi_pio(i, st, prog, 0);
}

// ======================================================
// Тестовая функция
// ======================================================
void test_midi_outputs() {
  Serial.println("[MIDI] Test: Sending C Major chord...");
  noteOn_all(60, 100);
  noteOn_all(64, 100);
  noteOn_all(67, 100);
  delay(500);
  noteOff_all(60);
  noteOff_all(64);
  noteOff_all(67);
  Serial.println("[MIDI] Test complete.\n");
}