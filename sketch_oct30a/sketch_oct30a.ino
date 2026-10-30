#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCK 18
#define MISO 19
#define MOSI 23
#define NSS 5
#define RST -1
#define DIO0 -1

#define i2c_Address 0x3c 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte MI_ID = 2; 
const byte DESTINO_ID = 1; 

String descifrarAtbash(String mensaje) {
  String mensajeDescifrado = "";
  for (int i = 0; i < mensaje.length(); i++) {
    char c = mensaje[i];
    if (c >= 'a' && c <= 'z') {
      mensajeDescifrado += (char)('z' - (c - 'a'));
    } else if (c >= 'A' && c <= 'Z') {
      mensajeDescifrado += (char)('Z' - (c - 'A'));
    } else {
      mensajeDescifrado += c;
    }
  }
  return mensajeDescifrado;
}

void procesarMensaje(int packetSize) {
  if (packetSize == 0) return;

  String paquete = "";
  while (LoRa.available()) {
    paquete += (char)LoRa.read();
  }

  int primerSeparador = paquete.indexOf('|');
  int segundoSeparador = paquete.indexOf('|', primerSeparador + 1);
  
  if (primerSeparador == -1 || segundoSeparador == -1) {
    Serial.println("Formato de paquete incorrecto: " + paquete);
    return; 
  }

  String strOrigen = paquete.substring(0, primerSeparador);
  String strDestino = paquete.substring(primerSeparador + 1, segundoSeparador);
  String mensajeCifrado = paquete.substring(segundoSeparador + 1);

  int idOrigen = strOrigen.toInt();
  int destinatario = strDestino.toInt();

  if (destinatario != MI_ID) { 
    Serial.println("Mensaje ignorado (para ID " + strDestino + ")");
    return;
  }

  String mensajeDescifrado = descifrarAtbash(mensajeCifrado);
  int rssi = LoRa.packetRssi();

  Serial.println("--- Nuevo Mensaje Recibido ---");
  Serial.println("Paquete completo: " + paquete); // Ej: 1|2|HVMHLI ZXGREL
  Serial.println("De ID: " + String(idOrigen));
  Serial.println("Mensaje descifrado: " + mensajeDescifrado);
  Serial.println("RSSI: " + String(rssi));
  Serial.println("------------------------------");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("De ID: ");
  display.println(idOrigen);
  
  display.println("Msj original:");
  display.println(mensajeDescifrado);
  
  display.print("RSSI: ");
  display.println(rssi);
  display.display();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Iniciando Receptor LoRa...");

  delay(250); 
  if(!display.begin(i2c_Address, true)) {
    Serial.println("Fallo en la asignacion de OLED");
    while(1); 
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Receptor Iniciando...");
  display.display();

  SPI.begin(SCK, MISO, MOSI, NSS);
  LoRa.setPins(NSS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Error al iniciar LoRa.");
    display.println("Error LoRa");
    display.display();
    while (1); 
  }
  
  Serial.println("LoRa iniciado correctamente. Esperando mensajes...");
  display.println("Esperando msjs...");
  display.display();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    procesarMensaje(packetSize);
  }
}







