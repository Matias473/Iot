#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "DHT.h"

#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char* ssid = "MEGACABLE-2.4G-AAAB";
const char* password = "hPf2aWjVHq";

// URLs API
const char* URL_TEMP = "http://192.168.100.18:3000/api/temperatura";
const char* URL_HUM = "http://192.168.100.18:3000/api/humedad";

// Tiempo de espera entre lecturas
const unsigned long INTERVALO = 5 * 60 * 1000;  // 5 minutos en milisegundos
unsigned long ultimoEnvio = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Configurar hora local
  configTime(-6 * 3600, 0, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora sincronizada");
}

void loop() {
  if (millis() - ultimoEnvio >= INTERVALO) {
    ultimoEnvio = millis();

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

void enviarDato(const char* url, const char* tipo, float valor, const char* hora) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String unidad = (String(tipo) == "temperatura") ? "°C" : "%";

    String json = "{\"valor\":" + String(valor) +
                  ",\"unidad\":\"" + unidad +
                  "\",\"hora\":\"" + String(hora) +
                  "\",\"ubicacion\":\"Cabaña1\"}";

    int codigo = http.POST(json);
    Serial.printf("Enviado %s: %.2f %s a las %s. Código %d\n", tipo, valor, unidad.c_str(), hora, codigo);
    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }
}