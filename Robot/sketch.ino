#include <Arduino_FreeRTOS.h>
#include <semphr.h>

const int nr_roti = 4;
struct Roata {
  float viteza = 0;
  int directie = 0;
  long timpMasurare = 0;
  void generareViteza() {
    viteza = random(-1000, 1001) / 100.0;
    directie = (viteza != 0) ? (viteza / abs(viteza)) : 0;
  }
} roti[nr_roti];

SemaphoreHandle_t mutex;

void TaskRoti(void *pvParameters) {
  int id = (int)pvParameters;
  while (1) {
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
      roti[id].generareViteza();
      roti[id].timpMasurare = millis();
      xSemaphoreGive(mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void TaskAfisare(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
      for (int i = 0; i < nr_roti; i++) {
        Serial.print("Roata ");
        Serial.print(i);
        Serial.print(": Viteza: ");
        Serial.print(roti[i].viteza);
        Serial.print(", Directie: ");
        Serial.print(roti[i].directie);
        Serial.print(", Timp: ");
        Serial.print(roti[i].timpMasurare);
        Serial.println("ms");
      }
      Serial.println();
      xSemaphoreGive(mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void setup() {
  Serial.begin(9600);
  mutex = xSemaphoreCreateMutex();

  for (int i = 0; i < nr_roti; i++) {
    xTaskCreate(TaskRoti, "TaskRoti", 128, (void *)i, 2, NULL);
  }
  xTaskCreate(TaskAfisare, "TaskAfisare", 128, NULL, 1, NULL);
}

void loop() {}