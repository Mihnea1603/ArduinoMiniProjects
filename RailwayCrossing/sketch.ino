#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Servo.h>

const int arrivalButton = 25, gatePin = 10, crossingLED = 5,
          arrivalTimeMin = 3000, arrivalTimeMax = 5000,
          crossingTimeMin = 2000, crossingTimeMax = 4000, gateTime = 2000;

Servo gateServo;
SemaphoreHandle_t arrivalSemaphore, crossingSemaphore;

void arrivalTask(void *pvParameters) {
  while (1) {
    xSemaphoreTake(arrivalSemaphore, portMAX_DELAY);
    if (digitalRead(arrivalButton) == LOW) {
      Serial.println("Train detected");
      vTaskDelay(pdMS_TO_TICKS(arrivalTimeMin - gateTime));
      Serial.println("Gate closing");
      for (int angle = 1; angle <= 90; angle++) {
        gateServo.write(angle);
        vTaskDelay(pdMS_TO_TICKS(gateTime / 90));
      }
      Serial.println("Gate closed");
      int arrivalTime = random(arrivalTimeMin, arrivalTimeMax);
      vTaskDelay(pdMS_TO_TICKS(arrivalTime - arrivalTimeMin));
      Serial.print("Arrival time: ");
      Serial.print(arrivalTime);
      Serial.println("ms");
      xSemaphoreGive(crossingSemaphore);
    }
    else {
      xSemaphoreGive(arrivalSemaphore);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void crossingTask(void *pvParameters) {
  while (1) {
    xSemaphoreTake(crossingSemaphore, portMAX_DELAY);
    int crossingTime = random(crossingTimeMin, crossingTimeMax);
    digitalWrite(crossingLED, HIGH);
    vTaskDelay(pdMS_TO_TICKS(crossingTime));
    digitalWrite(crossingLED, LOW);
    Serial.print("Crossing time: ");
    Serial.print(crossingTime);
    Serial.println("ms");

    gateServo.write(0);
    Serial.println("Gate opening");
    for (int angle = 89; angle >= 0; angle--) {
      gateServo.write(angle);
      vTaskDelay(pdMS_TO_TICKS(gateTime / 90));
    }
    Serial.println("Gate opened");
    xSemaphoreGive(arrivalSemaphore);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(arrivalButton, INPUT_PULLUP);
  pinMode(crossingLED, OUTPUT);
  gateServo.attach(gatePin);
  gateServo.write(0);

  arrivalSemaphore = xSemaphoreCreateBinary();
  crossingSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(arrivalSemaphore);

  xTaskCreate(arrivalTask, "Arrival", 128, NULL, 1, NULL);
  xTaskCreate(crossingTask, "Crossing", 128, NULL, 1, NULL);
}

void loop() {}