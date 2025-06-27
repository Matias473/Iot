#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>  // Para obtener hora con NTP

// Pines RC522
#define RST_PIN 26
#define SS_PIN  13

// Pines para LEDs
#define LED_ACCESO    4
#define LED_DENEGADO 23

// WiFi y servidor
const char* ssid = "Nombre de la red";
const char* password = "contraseña";
const char* serverURL = "http://ip local:3000/api/acceso";

// Usuarios autorizados
byte Usuario1[4] = {0x8A, 0x30, 0x10, 0x0E}; //tarjeta
byte Usuario2[4] = {0xEF, 0x81, 0x19, 0x24}; //llavero

MFRC522 mfrc522(SS_PIN, RST_PIN);
byte LecturaUID[4];

void setup() {
  Serial.begin(115200);
  SPI.begin(14, 19, 27, 13);  // SCK, MISO, MOSI, SS
  mfrc522.PCD_Init();

  pinMode(LED_ACCESO, OUTPUT);
  pinMode(LED_DENEGADO, OUTPUT);
  digitalWrite(LED_ACCESO, LOW);
  digitalWrite(LED_DENEGADO, LOW);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi conectado");

  // Configurar hora vía NTP (zona horaria México -6 GMT)
  configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("⌛ Sincronizando hora NTP...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n Hora NTP obtenida correctamente");

  Serial.println(" Listo para escanear tarjeta o llavero...");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // Obtener UID
  String uid_str = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    LecturaUID[i] = mfrc522.uid.uidByte[i];
    if (mfrc522.uid.uidByte[i] < 0x10) uid_str += "0";
    uid_str += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid_str.toUpperCase();

  // Obtener hora actual como cadena
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ No se pudo obtener hora NTP");
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

  // LED y Serial
  Serial.printf("UID: %s\t%s a las %s\n", uid_str.c_str(), acceso ? "Acceso permitido" : "Acceso denegado", hora_str);
  digitalWrite(acceso ? LED_ACCESO : LED_DENEGADO, HIGH);
  delay(1000);
  digitalWrite(acceso ? LED_ACCESO : LED_DENEGADO, LOW);

  // Enviar a servidor
  enviarAcceso(uid_str, usuario, String(hora_str));

  mfrc522.PICC_HaltA();
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
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"uid\":\"" + uid + "\",\"usuario\":\"" + usuario + "\",\"hora\":\"" + hora + "\"}";

    int codigo = http.POST(json);
    Serial.print("Enviado a servidor. Código HTTP: ");
    Serial.println(codigo);

    http.end();
  } else {
    Serial.println("No conectado a WiFi.");
  }
}
