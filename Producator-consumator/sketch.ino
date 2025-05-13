#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define N 5
int buffer[N], in, out;

SemaphoreHandle_t mutex, semPlin, semGol;

void taskProducator(void *pvParameters) {
  while (1) {
    xSemaphoreTake(semPlin, portMAX_DELAY);
    xSemaphoreTake(mutex, portMAX_DELAY);

    buffer[in % N] = in;
    Serial.print("A fost produs: ");
    Serial.println(in++);

    xSemaphoreGive(mutex);
    xSemaphoreGive(semGol);
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}

void taskConsumator(void *pvParameters) {
  while (1) {
    xSemaphoreTake(semGol, portMAX_DELAY);
    xSemaphoreTake(mutex, portMAX_DELAY);

    int w = buffer[out];
    out = (out + 1) % N;
    Serial.print("A fost consumat: ");
    Serial.println(w);

    xSemaphoreGive(mutex);
    xSemaphoreGive(semPlin);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Serial.begin(9600);

  mutex = xSemaphoreCreateMutex();
  semPlin = xSemaphoreCreateCounting(N, N);
  semGol = xSemaphoreCreateCounting(N, 0);

  xTaskCreate(taskProducator, "Producator", 128, NULL, 1, NULL);
  xTaskCreate(taskConsumator, "Consumator", 128, NULL, 1, NULL);
}

void loop() {}