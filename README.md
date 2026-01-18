# RP2040 MIDI Router Pro
⚙️ Как работает
	1.	Нажми “Connect” → выбери COM-порт Pico.
	
	2.	Таблица заполнится конфигом с устройства.
	
	3.	Можно нажать “Add HID”, вписать 0x1E, выбрать note, 60, порт USB, канал 1.
	
	4.	Можно отметить строки чекбоксом и удалить их через “Delete”.
	
	5.	Нажми “Save” — Pico сохранит JSON в LittleFS.
	
	6.	Переключайся между пресетами 1–3.

	Собери проект в PlatformIO:

	pio run -t upload
    pio run -t uploadfs
