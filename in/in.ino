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

  Serial.println("LoRa приёмник готов");
  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "LoRa Приёмник готов");
  u8g2.sendBuffer();
}
void loop() {
  // Проверка наличия входящего пакета
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedMessage = "";
    int rssiSum = 0;

    // Чтение сообщения
    while (LoRa.available()) {
      receivedMessage += (char)LoRa.read();
    }

    i++;

    // Усреднение RSSI
    for (int i = 0; i < RSSI_SAMPLES; i++) {
      rssiSum += LoRa.packetRssi();  // Получаем RSSI для каждого пакета
      delay(50);  // Короткая задержка между измерениями
    }
    int rssiAvg = rssiSum / RSSI_SAMPLES;  // Среднее значение RSSI

    Serial.print("Получено сообщение: ");
    Serial.println(receivedMessage);

    // Измерение напряжения на пине
    float voltage = analogRead(VOLTAGE_PIN) * (3.3 / 4095.0);  // Преобразование значения АЦП в напряжение
    voltage *= 2.15;  // Умножаем на коэффициент делителя напряжения

    Serial.print("Напряжение: ");
    Serial.println(voltage);

    Serial.print("Средний RSSI: ");
    Serial.println(rssiAvg);


    // Отображение сообщения, напряжения, среднего RSSI
    u8g2.clearBuffer();
    u8g2.setCursor(0, 15);
    u8g2.print(receivedMessage.c_str());

    u8g2.setCursor(0, 35);
    u8g2.print("Напряжение:");
    u8g2.print(voltage, 2);  // Вывод напряжения с 2 знаками после запятой
    u8g2.print(" В");

    u8g2.setCursor(0, 45);
    u8g2.print("Средний RSSI:");
    u8g2.print(rssiAvg);


    u8g2.setCursor(0, 60);
    u8g2.print("Пакет : ");
    u8g2.print(i);  //Отображаем количество отправленных сообщений 

    u8g2.sendBuffer();  // Отправляем данные на дисплей
  }
}