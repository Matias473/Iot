#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>
#include <ESP32Servo.h>
#include "DHT.h"

// Pines RC522
#define RST_PIN 26
#define SS_PIN  13

// Pines LEDs y sensores
#define LED_ACCESO     4
#define LED_DENEGADO  23
#define LED_MOVIMIENTO 15
#define PIN_TRIG 5
#define PIN_ECHO 18
#define BOTON_PIN 25

// Servo
#define SERVO_PIN 21
Servo servo;

// Sensor DHT11
#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char* ssid = "MEGACABLE-2.4G-AAAB";
const char* password = "hPf2aWjVHq";

// URLs
const char* URL_ACCESO = "http://192.168.100.18:3000/api/acceso";
const char* URL_MOVIMIENTO = "http://192.168.100.18:3000/api/movimiento";
const char* URL_TEMP = "http://192.168.100.18:3000/api/temperatura";
const char* URL_HUM = "http://192.168.100.18:3000/api/humedad";

// RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);
byte LecturaUID[4];
byte Usuario1[4] = {0x8A, 0x30, 0x10, 0x0E};
byte Usuario2[4] = {0xEF, 0x81, 0x19, 0x24};

// Temporizadores
unsigned long ultimaDeteccion = 0;
const unsigned long TIEMPO_ESPERA = 5000;
unsigned long ultimoEnvioDHT = 0;
const unsigned long INTERVALO_DHT = 5 * 60 * 1000;

void setup() {
  Serial.begin(115200);

  // Pines
  pinMode(LED_ACCESO, OUTPUT);
  pinMode(LED_DENEGADO, OUTPUT);
  pinMode(LED_MOVIMIENTO, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(BOTON_PIN, INPUT_PULLUP);

  digitalWrite(LED_ACCESO, LOW);
  digitalWrite(LED_DENEGADO, LOW);
  digitalWrite(LED_MOVIMIENTO, LOW);

  // Servo y DHT
  servo.attach(SERVO_PIN);
  servo.write(0);
  dht.begin();

  // RFID
  SPI.begin(14, 19, 27, 13);
  mfrc522.PCD_Init();

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Hora NTP
  configTime(-6 * 3600, 0, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora sincronizada");
}

void loop() {
  detectarMovimiento();
  leerTarjeta();
  verificarBoton();
  verificarDHT();
}

void detectarMovimiento() {
  long duracion = leerSensorUltrasonico(PIN_TRIG, PIN_ECHO);
  float distancia = duracion * 0.01723;

  if (distancia < 10) {
    digitalWrite(LED_MOVIMIENTO, HIGH);
    if (millis() - ultimaDeteccion > TIEMPO_ESPERA) {
      ultimaDeteccion = millis();
      enviarMovimiento(distancia);
    }
  } else {
    digitalWrite(LED_MOVIMIENTO, LOW);
  }
}

void leerTarjeta() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;

  String uid_str = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    LecturaUID[i] = mfrc522.uid.uidByte[i];
    if (mfrc522.uid.uidByte[i] < 0x10) uid_str += "0";
    uid_str += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid_str.toUpperCase();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener hora NTP");
    return;
  }
  char hora_str[9];
  strftime(hora_str, sizeof(hora_str), "%H:%M:%S", &timeinfo);

  String usuario;
  bool acceso = false;

  if (comparaUID(LecturaUID, Usuario1)) {
    usuario = "Usuario 1";
    acceso = true;
  } else if (comparaUID(LecturaUID, Usuario2)) {
    usuario = "Usuario 2";
    acceso = true;
  } else {
    usuario = "Desconocido";
    acceso = false;
  }

  Serial.printf("UID: %s\t%s a las %s\n", uid_str.c_str(), acceso ? "Acceso permitido" : "Acceso denegado", hora_str);
  digitalWrite(acceso ? LED_ACCESO : LED_DENEGADO, HIGH);
  delay(1000);
  digitalWrite(acceso ? LED_ACCESO : LED_DENEGADO, LOW);

  enviarAcceso(uid_str, usuario, hora_str);

  if (acceso) {
    servo.write(90);
    Serial.println("Servo: puerta abierta");
  }

  mfrc522.PICC_HaltA();
}

void verificarBoton() {
  static bool botonPresionado = false;
  if (digitalRead(BOTON_PIN) == LOW && !botonPresionado) {
    botonPresionado = true;
    servo.write(0);
    Serial.println("Botón: puerta cerrada manualmente");
  } else if (digitalRead(BOTON_PIN) == HIGH && botonPresionado) {
    botonPresionado = false;
  }
}

void verificarDHT() {
  if (millis() - ultimoEnvioDHT >= INTERVALO_DHT) {
    ultimoEnvioDHT = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Error al leer DHT11");
      return;
    }

    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char hora[9];
    strftime(hora, sizeof(hora), "%H:%M:%S", &timeinfo);

    enviarDato(URL_TEMP, "temperatura", t, hora);
    enviarDato(URL_HUM, "humedad", h, hora);
  }
}

long leerSensorUltrasonico(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  return pulseIn(echo, HIGH);
}

bool comparaUID(byte lectura[], byte usuario[]) {
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (lectura[i] != usuario[i]) return false;
  }
  return true;
}

void enviarAcceso(String uid, String usuario, String hora) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URL_ACCESO);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"uid\":\"" + uid + "\",\"usuario\":\"" + usuario + "\",\"hora\":\"" + hora + "\"}";
    int codigo = http.POST(json);
    Serial.print("Acceso enviado. Código HTTP: ");
    Serial.println(codigo);
    http.end();
  }
}

void enviarMovimiento(float distancia) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  char hora_str[9];
  strftime(hora_str, sizeof(hora_str), "%H:%M:%S", &timeinfo);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URL_MOVIMIENTO);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"evento\":\"Movimiento detectado\",\"distancia\":\"" + String(distancia) +
                  "\",\"hora\":\"" + hora_str + "\"}";
    int codigo = http.POST(json);
    Serial.print("Movimiento enviado. Código HTTP: ");
    Serial.println(codigo);
    http.end();
  }
}

void enviarDato(const char* url, const char* tipo, float valor, const char* hora) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String unidad = (String(tipo) == "temperatura") ? "°C" : "%";
    String json = "{\"valor\":" + String(valor) +
                  ",\"unidad\":\"" + unidad +
                  "\",\"hora\":\"" + hora +
                  "\",\"ubicacion\":\"Cabaña1\"}";

    int codigo = http.POST(json);
    Serial.printf("Enviado %s: %.2f %s a las %s. Código %d\n", tipo, valor, unidad.c_str(), hora, codigo);
    http.end();
  }
}
