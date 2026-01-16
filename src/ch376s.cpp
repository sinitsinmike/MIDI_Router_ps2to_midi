#pragma once
#include <stdint.h>

// ======================================================
// ИНИЦИАЛИЗАЦИЯ И ОСНОВНЫЕ ФУНКЦИИ
// ======================================================

/**
 * @brief Инициализация всех MIDI интерфейсов:
 *  - USB (TinyUSB)
 *  - DIN MIDI (UART1 TX)
 *  - 10 TRS MIDI OUT через PIO
 */
void setup_midi_output();

/**
 * @brief Отправить произвольное MIDI сообщение через USB
 */
void send_midi_usb(uint8_t status, uint8_t data1, uint8_t data2);

/**
 * @brief Отправить MIDI сообщение через DIN (UART1 TX)
 */
void send_midi_uart(uint8_t status, uint8_t data1, uint8_t data2);

/**
 * @brief Отправить MIDI сообщение на один из 10 TRS портов (через PIO)
 * 
 * @param port индекс TRS-порта (0–9)
 */
void send_midi_pio(uint8_t port, uint8_t status, uint8_t data1, uint8_t data2);

// ======================================================
// ДОПОЛНИТЕЛЬНЫЕ УТИЛИТЫ
// ======================================================

/**
 * @brief Отправить NoteOn на все интерфейсы и порты
 */
void noteOn_all(uint8_t note, uint8_t vel);

/**
 * @brief Отправить NoteOff на все интерфейсы и порты
 */
void noteOff_all(uint8_t note);

/**
 * @brief Отправить Control Change на все интерфейсы
 * 
 * @param cc Номер контроллера (0–127)
 * @param val Значение (0–127)
 * @param ch MIDI канал (1–16)
 */
void cc_all(uint8_t cc, uint8_t val, uint8_t ch = 1);

/**
 * @brief Отправить Program Change на все интерфейсы
 */
void programChange_all(uint8_t prog, uint8_t ch = 1);

/**
 * @brief Тестовая функция — посылает аккорд C мажор на все выходы
 */
void test_midi_outputs();