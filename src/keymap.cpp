#include "keymap.h"
#include "midi_output.h"
#include "config_manager.h"

// --- простейшая таблица клавиш HID USB Keyboard Set 2 ---
// (сканкоды клавиатуры, типичные для CH376S HID клавиатуры)
struct {
  uint8_t hid;
  uint8_t note;
} defaultMap[] = {
  {0x1D, 60}, // Z -> C4
  {0x1B, 62}, // X -> D4
  {0x06, 64}, // C -> E4
  {0x19, 65}, // V -> F4
  {0x05, 67}, // B -> G4
  {0x11, 69}, // N -> A4
  {0x10, 71}, // M -> B4
  {0x36, 72}, // , -> C5
  {0x37, 74}, // . -> D5
  {0x38, 76}  // / -> E5
};

bool prevState[256] = {false};

// вспомогательная функция для поиска дефолтного маппинга
uint8_t get_default_note(uint8_t hid) {
  for (auto &m : defaultMap)
    if (m.hid == hid)
      return m.note;
  return 0; // нет соответствия
}

void handle_hid_code(uint8_t hid_code) {
  static unsigned long lastTime[256];
  bool pressed = !prevState[hid_code];
  prevState[hid_code] = !prevState[hid_code];

  unsigned long now = millis();
  if (now - lastTime[hid_code] < 5) return; // антидребезг
  lastTime[hid_code] = now;

  // найти конфигурацию в JSON
  char keyHex[6];
  sprintf(keyHex, "0x%02X", hid_code);
  KeyConfig cfg = get_key_config(String(keyHex));

  // если конфиг не найден, используем дефолтную карту
  if (cfg.value == 0) {
    cfg.type = "note";
    cfg.value = get_default_note(hid_code);
    cfg.port = "USB";
    cfg.channel = 1;
  }

  if (cfg.value == 0) return; // пропустить нераспознанные клавиши

  uint8_t status = (cfg.type == "note")
                     ? (pressed ? 0x90 : 0x80)
                     : 0xB0;

  // выбор куда отправлять
  if (cfg.port == "USB") {
    send_midi_usb(status | ((cfg.channel - 1) & 0x0F),
                  cfg.value,
                  pressed ? 127 : 0);
  }
  else if (cfg.port == "DIN") {
    send_midi_uart(status | ((cfg.channel - 1) & 0x0F),
                   cfg.value,
                   pressed ? 127 : 0);
  }
  else {
    uint8_t portIndex = cfg.port[0] - 'A'; // A=0, B=1 ...
    if (portIndex < 10)
      send_midi_pio(portIndex,
                    status | ((cfg.channel - 1) & 0x0F),
                    cfg.value,
                    pressed ? 127 : 0);
  }
}