#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Dirección MAC del emisor (primera ESP32)
uint8_t broadcastAddress[] = {0x48, 0xe7, 0x29, 0x94, 0x0c, 0x88};

// Estructura de datos para enviar y recibir
typedef struct struct_message {
  char letter;
} struct_message;

struct_message receivedData;
char lastSentLetter = '\0'; // Variable para rastrear la última letra enviada

// Callback que se ejecutará cuando los datos sean recibidos
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  // Imprime la letra recibida por el puerto serie de Arduino IDE
  Serial.print("Letra recibida en Arduino IDE: ");
  Serial.println(receivedData.letter);

  // Imprime la letra recibida por el puerto serie de la ESP32
  Serial.print("Letra recibida en ESP32: ");
  Serial.println(receivedData.letter);

  // Enviar la letra por el puerto serial solo si ha cambiado
  if (receivedData.letter != lastSentLetter) {
    Serial.write(receivedData.letter);
    lastSentLetter = receivedData.letter;
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Conexión ESP-NOW exitosa!");
  } else {
    Serial.println("Error en la conexión ESP-NOW");
  }
}

void setup() {
  Serial.begin(115200);

  // Configura el dispositivo como estación Wi-Fi
  WiFi.mode(WIFI_STA);

  // Inicializa ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  // Registra el callback de recepción
  esp_now_register_recv_cb(OnDataRecv);

  // Agrega el emisor
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error al añadir el emisor");
    return;
  }

  // Registra el callback de envío
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  // No es necesario agregar ninguna lógica aquí, ya que el callback OnDataRecv maneja la recepción de datos
}
