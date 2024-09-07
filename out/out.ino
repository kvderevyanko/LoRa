#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>

// Пины для LoRa
#define LORA_SS 18      // Pin CS
#define LORA_RST 23     // Pin RST
#define LORA_DIO0 26    // Pin DI0
#define LORA_MOSI 27    // Pin MOSI
#define LORA_MISO 19    // Pin MISO
#define LORA_SCLK 5     // Pin SCLK

// Пин для измерения напряжения
#define VOLTAGE_PIN 35  // АЦП пин для измерения напряжения
#define RSSI_SAMPLES 5  // Количество измерений для усреднения

int i = 0;

// Настройки экрана (SSD1306 128x64 через I2C)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);

void setup() {
  Serial.begin(115200);
  // Инициализация дисплея
  u8g2.begin();
  u8g2.enableUTF8Print();  // Включаем поддержку UTF-8 для вывода русского текста
  
  // Установка шрифта с поддержкой кириллицы
  u8g2.setFont(u8g2_font_6x12_t_cyrillic);
  
  // Инициализация LoRa
  SPI.begin(LORA_SCLK, LORA_MISO, LORA_MOSI, LORA_SS);  // Настраиваем SPI с указанными пинами
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);           // Настройка пинов для LoRa
  if (!LoRa.begin(433E6)) {                             // Частота 433 МГц для SX1278
    Serial.println("Не удалось запустить LoRa модуль");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "Ошибка LoRa");
    u8g2.sendBuffer();
    while (1);
  }

  Serial.println("LoRa передатчик готов");

  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "LoRa передатчик готов");
  u8g2.sendBuffer();

  // Настройка генератора случайных чисел
  randomSeed(analogRead(0));
}

void loop() {
  // Генерация случайного сообщения
  String message = "Message #" + String(random(1, 1000));  // Случайное сообщение от 1 до 1000

  Serial.print("Отправка сообщения: ");
  Serial.println(message);

  // Отправка сообщения через LoRa
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  i++;

  // Измерение напряжения на пине
  float voltage = analogRead(VOLTAGE_PIN) * (3.3 / 4095.0);  // Преобразование значения АЦП в напряжение
  voltage *= 2.26;  // Умножаем на коэффициент делителя напряжения
  
  Serial.print("Напряжение: ");
  Serial.println(voltage);

  // Отображение сообщения на OLED дисплее
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10);
  u8g2.print("Отправлено сообщение:");
  u8g2.setCursor(0, 25);
  u8g2.print(message);
  
  u8g2.setCursor(0, 45);
  u8g2.print("Напряжение:");
  u8g2.print(voltage, 2);  // Вывод напряжения с 2 знаками после запятой
  u8g2.print(" В");

  u8g2.setCursor(0, 60);
  u8g2.print("Пакет : ");
  u8g2.print(i);  //Отображаем количество отправленных сообщений 
  u8g2.sendBuffer();

  delay(5000);  // Задержка 5 секунд
}
