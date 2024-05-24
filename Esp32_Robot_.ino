#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

const int sensorizquierda = 35; // Pin del sensor KY-032 izquierda
const int sensorderecha   = 32; // sensor ky-032 derecha
const int in1 = 13; // Pin de control de dirección del motor 1 (IN1 del L298N)
const int in2 = 12; // Pin de control de dirección del motor 1 (IN2 del L298N)
const int in3 = 15; // Pin de control de dirección del motor 2 (IN3 del L298N)
const int in4 = 14; // Pin de control de dirección del motor 2 (IN4 del L298N)
const int en1 = 27; // Pin de habilitación del motor 1 (EN1 del L298N)
const int en2 = 26; // Pin de habilitación del motor 2 (EN2 del L298N)
const int sensor_linea = 34; // Pin del sensor TCRT5000

int digitalValue;
int deteccionizquierda = 0;
int deteccionderecha = 0;
char lastSentLetter = '\0'; // Mantiene el estado de la última letra enviada

// Dirección MAC del receptor
uint8_t broadcastAddress[] = {0x64, 0xb7, 0x08, 0x6e, 0xeb, 0x78};

// Estructura de datos para enviar
typedef struct struct_message {
  char letter; // Cambiado a un carácter
} struct_message;

// Crear una estructura struct_message llamada myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// Callback cuando los datos son enviados
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nÚltimo estado del paquete enviado:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entregado" : "Fallo en la entrega");
}
 
// Callback que se ejecutará cuando los datos sean recibidos
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // No necesitamos hacer nada con los datos recibidos en este caso
}

void detectorOponente(){

  deteccionizquierda = 0;
  deteccionderecha   = 0;
  
  int izquierda = digitalRead(sensorizquierda); // Lee el valor del sensor izquierda
  int derecha = digitalRead(sensorderecha); // Lee el valor del sensor derecha

  if (izquierda == HIGH) { // Si el sensor izquierda detecta un obstáculo
    Serial.println("¡Obstáculo detectado izquierda!");
    deteccionizquierda = 1;
  }
  if (derecha == HIGH) { // Si el sensor derecha detecta un obstáculo
    Serial.println("¡Obstáculo detectado derecha!");
    deteccionderecha = 1;
    
  }
}

void adelante_rueda1() {
  // Adelante con la rueda 1
  digitalWrite(en1, HIGH);
  digitalWrite(en2, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  /*if (lastSentLetter != 'a') {
    myData.letter = 'a'; // Enviar 'a' al mover la rueda 1
    lastSentLetter = 'a';
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }*/
}

void adelante_rueda2() {
  // Adelante con la rueda 2
  digitalWrite(en1, LOW);
  digitalWrite(en2, HIGH);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  /*if (lastSentLetter != 'b') {
    myData.letter = 'b'; // Enviar 'b' al mover la rueda 2
    lastSentLetter = 'b';
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }*/
}

void detener() {
  // Detener ambos motores
  digitalWrite(en1, LOW);
  digitalWrite(en2, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  deteccionizquierda = 0;
  deteccionderecha = 0;
}

void setup() {
  Serial.begin(115200);

  // Configurar el dispositivo como una estación Wi-Fi
  WiFi.mode(WIFI_STA);

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  // Una vez que ESPNow se haya inicializado correctamente, nos registraremos para el callback de envío
  esp_now_register_send_cb(OnDataSent);
 
  // Registrar el receptor
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
 
  // Añadir el receptor      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Error al añadir receptor");
    return;
  }
  // Una vez que ESPNow se haya inicializado correctamente, nos registraremos para el callback de recepción
  esp_now_register_recv_cb(OnDataRecv);
  
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(en1, OUTPUT);
  pinMode(en2, OUTPUT);
  pinMode(sensor_linea, INPUT); // Configura el pin del sensor de línea como entrada
  pinMode(sensorizquierda, INPUT); // Configura el pin del sensor de izquierda como entrada
  pinMode(sensorderecha, INPUT); // Configura el pin del sensor de derecha como entrada
}

void loop() {
  detectorOponente();
  
  // Si alguno de los sensores detecta algo, detener los motores y enviar el estado
  if (deteccionizquierda == 0) {
    detener();
    if (lastSentLetter != 'd') {
      myData.letter = 'd'; // Enviar 'd' al recibir la detección izquierda
      lastSentLetter = 'd';
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    }
  } else if(deteccionderecha == 0){
    detener();
    if (lastSentLetter != 'e') {
      myData.letter = 'e'; // Enviar 'e' al recibir la detección derecha
      lastSentLetter = 'e';
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    }
  } else {
    // Si ninguno de los sensores detecta algo, continuar moviendo el robot y enviar el estado
    digitalValue = digitalRead(sensor_linea);
    if (digitalValue == HIGH) {
      adelante_rueda1(); // Avanzar con la rueda 1
      if (lastSentLetter != 'a') {
        myData.letter = 'a'; // Enviar 'a' al mover la rueda 1
        lastSentLetter = 'a';
        esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      }
    } else if (digitalValue == LOW){
      adelante_rueda2(); // Avanzar con la rueda 2
      if (lastSentLetter != 'b') {
        myData.letter = 'b'; // Enviar 'b' al mover la rueda 2
        lastSentLetter = 'b';
        esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      }
    }
  
  
  delay(50); // Esperar 50 ms antes de la siguiente iteración
}
}
