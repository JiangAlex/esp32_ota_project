#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== DEBUG: ESP32 Starting ===");
  Serial.println("Basic setup working...");
  
  // Test basic Arduino functions
  Serial.printf("millis(): %lu\n", millis());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  
  Serial.println("=== DEBUG: Setup Complete ===");
}

void loop() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.printf("Loop running... millis: %lu, heap: %d\n", millis(), ESP.getFreeHeap());
  }
  delay(10);
}