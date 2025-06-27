#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>  // Para obtener hora NTP

// Pines del sensor
int pinLed = 15;
int pinEcho = 18;
int pinTrig = 5;

// WiFi
const char* ssid = "MEGACABLE-2.4G-AAAB";
const char* password = "hPf2aWjVHq";

// Servidor (Node.js)
const char* serverURL = "http://192.168.100.18:3000/api/movimiento";

// Tiempo de espera para evitar múltiples registros
unsigned long ultimaDeteccion = 0;
const unsigned long tiempoEspera = 5000;  // 5 segundos

void setup() {
  Serial.begin(115200);
  pinMode(pinLed, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi conectado");

  // Configurar hora NTP (zona México UTC-6)
  configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n Hora NTP sincronizada");
}

void loop() {
  long duracion = leerSensor(pinTrig, pinEcho);
  float distancia = duracion * 0.01723;

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  if (distancia < 10) {
    digitalWrite(pinLed, HIGH);
    
    if (millis() - ultimaDeteccion > tiempoEspera) {
      ultimaDeteccion = millis();
      enviarMovimiento(distancia);
    }
  } else {
    digitalWrite(pinLed, LOW);
  }

  delay(200);  // breve pausa
}

long leerSensor(int trig, int echo) {
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  pinMode(echo, INPUT);
  return pulseIn(echo, HIGH);
}

void enviarMovimiento(float distancia) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener la hora");
    return;
  }

  char hora_str[9];
  strftime(hora_str, sizeof(hora_str), "%H:%M:%S", &timeinfo);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"evento\":\"Movimiento detectado\",\"distancia\":\"" + String(distancia) + "\",\"hora\":\"" + String(hora_str) + "\"}";

    int codigo = http.POST(json);
    Serial.print("Enviado a servidor. Código HTTP: ");
    Serial.println(codigo);
    http.end();
  } else {
    Serial.println("Sin conexión WiFi");
  }
}
