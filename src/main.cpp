#include <Arduino.h>
#include "config.h"
#include "BluetoothSerial.h"

// change to BLE serial

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;


uint32_t time_to_stop = 0;
bool test_running = false;
volatile uint32_t sensor_pulses = 0;

void handleInterrupt();

void setup() {
  pinMode(VALVE_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  digitalWrite(VALVE_PIN, LOW);
  
  Serial.begin(921600);
  SerialBT.begin("ESP32test");
  attachInterrupt(SENSOR_PIN, handleInterrupt, RISING);

  while (!SerialBT.available()) {
    Serial.println("Waiting for connection");
    delay(1000);
  }

  digitalWrite(BUILTIN_LED, HIGH);
  delay(1000);
  digitalWrite(BUILTIN_LED, LOW);
  delay(1000);
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read();
    if (command == 's' && !test_running) {
      SerialBT.println("Starting test");
      test_running = true;
      digitalWrite(VALVE_PIN, HIGH);
      time_to_stop = millis() + TEST_TIME;
    }
  }
  
  if (test_running && millis() > time_to_stop) {
    SerialBT.println("Stopping test");
    test_running = false;
    digitalWrite(VALVE_PIN, LOW);
    SerialBT.print("Pulses: ");
    SerialBT.println(sensor_pulses);
    sensor_pulses = 0;
  }
}

void handleInterrupt() {
  if (test_running) sensor_pulses++;
}