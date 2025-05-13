#include <Arduino_FreeRTOS.h>
#include <semphr.h>

volatile int balanta = 0;
SemaphoreHandle_t sem_server, sem_depunere, sem_retragere;

void taskDepunere(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    //Asteapta semaforul pentru depunere
    if (xSemaphoreTake(sem_depunere, portMAX_DELAY) == pdTRUE) {
      Serial.println("Introdu suma de depus:");
      //Asteapta pana cand e introdusa suma
      while (Serial.available() == 0) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      //Citeste suma depusa si caracterul '\n'
      int sumaDepusa = Serial.parseInt();
      Serial.read();
      //Verifica daca suma depusa e valida
      if (sumaDepusa < 0) {
        Serial.println("Depunere invalida");
      }
      else {
        //Actualizeaza balanta
        balanta += sumaDepusa;
        Serial.print("Depunere: ");
        Serial.println(sumaDepusa);
      }
      xSemaphoreGive(sem_server); //Elibereaza semaforul pentru server
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void taskRetragere(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    //Asteapta semaforul pentru retragere
    if (xSemaphoreTake(sem_retragere, portMAX_DELAY) == pdTRUE) {
      Serial.println("Introdu suma de retras:");
      //Asteapta pana cand e introdusa suma
      while (Serial.available() == 0) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      //Citeste suma retrasa si caracterul '\n'
      int sumaRetrasa = Serial.parseInt();
      Serial.read();
      //Verifica daca suma retrasa e valida
      if (sumaRetrasa < 0) {
        Serial.println("Retragere invalida");
      }
      //Verifica daca balanta e suficienta
      else if (balanta >= sumaRetrasa) {
        //Actualizeaza balanta
        balanta -= sumaRetrasa;
        Serial.print("Retragere: ");
        Serial.println(sumaRetrasa);
      }
      else {
        Serial.println("Fonduri insuficiente");
      }
      xSemaphoreGive(sem_server); //Elibereaza semaforul pentru server
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void taskServer(void *pvParameters) {
  (void) pvParameters;
  while (1) {
    //Asteapta semaforul pentru server
    if (xSemaphoreTake(sem_server, portMAX_DELAY) == pdTRUE) {
      Serial.print("Balanta curenta: ");
      Serial.println(balanta);
      bool valid = 0;
      //Asteapta o cerere valida ('d' pentru depunere, 'r' pentru retragere)
      while (!valid) {
        while (Serial.available() == 0) {
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        //Citeste comanda si elibereaza bufferul
        char input = Serial.read();
        while (Serial.available() > 0) {
          Serial.read();
        }
        //Verifica daca cererea e valida
        if (input == 'd') {
          valid = 1;
          xSemaphoreGive(sem_depunere); //Elibereaza semaforul pentru depunere
        }
        else if (input == 'r') {
          valid = 1;
          xSemaphoreGive(sem_retragere); //Elibereaza semaforul pentru retragere
        }
        else {
          Serial.println("Cerere invalida");
        }
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  //Crearea semafoarelor
  sem_server = xSemaphoreCreateBinary();
  sem_depunere = xSemaphoreCreateBinary();
  sem_retragere = xSemaphoreCreateBinary();
  //Elibereaza initial semaforul serverului
  xSemaphoreGive(sem_server);
  //Crearea task-urilor
  xTaskCreate(taskServer, "Server", 128, NULL, 1, NULL);
  xTaskCreate(taskDepunere, "Depunere", 128, NULL, 1, NULL);
  xTaskCreate(taskRetragere, "Retragere", 128, NULL, 1, NULL);
}

void loop() {
}
