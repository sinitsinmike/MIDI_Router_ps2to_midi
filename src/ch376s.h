#pragma once
#include <Arduino.h>

// Инициализация и опрос CH376S
void setup_ch376s();
void ch376s_task();

// Включение отладочного вывода (необязательно)
// #define DEBUG_CH376S