#include <Arduino.h>
#include <WiFi.h>
#include "webserver_setup.h"
#include "sensores.h"
#include "motores.h"
#include "modo_autonomo.h"
#include "servo_rele.h"
#include "mapa_espacios.h"

// --- Configuración WiFi ---
const char* ssid = "RobotESP32";
const char* password = "12345678";

void setup() {
  Serial.begin(115200);
  delay(1000);  // Esperar a que se estabilice la comunicación serial
  
  Serial.println("\n\n═══════════════════════════════════════");
  Serial.println("🤖 CARRO ROBÓTICO AUTÓNOMO ESP32");
  Serial.println("═══════════════════════════════════════");

  // --- WiFi como punto de acceso ---
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.printf("📶 WiFi AP iniciado: %s\n", ssid);
  Serial.printf("🌐 IP: %s\n", IP.toString().c_str());
  Serial.printf("🔑 Contraseña: %s\n\n", password);

  // --- Inicialización de hardware ---
  Serial.println("⚙️ Inicializando hardware...");
  
  inicializarMotores();
  Serial.println("  ✓ Motores inicializados");
  
  iniciarSensores();
  Serial.println("  ✓ Sensores ultrasónicos listos");
  
  iniciarServoYRele();
  Serial.println("  ✓ Servo y relé inicializados");
  
  iniciarMQ135();
  Serial.println("  ✓ MQ135 inicializado");
  
  // 🆕 Inicializar mapa espacial
  inicializarMapa();
  Serial.println("  ✓ Sistema de mapeo inicializado");

  // --- Servidor web ---
  setupWebServer();

  Serial.println("\n✅ Configuración completa y lista para operar");
  Serial.println("═══════════════════════════════════════\n");
}

void loop() {
  // ⚠️ AsyncWebServer NO necesita server.handleClient()
  
  // 🧠 Ejecuta solo si el modo autónomo está activado
  if (estaAutonomoActivo()) {
    ejecutarModoAutonomo(MODE_AUTO);
  } 
  else {
    // En modo manual, mostrar estado de sensores cada 2 segundos
    static unsigned long ultimoPrint = 0;
    if (millis() - ultimoPrint > 2000) {
      ultimoPrint = millis();

      long front = medirDistanciaFront();
      long left  = medirDistanciaLeft();
      long right = medirDistanciaRight();
      int mq = leerMQ135();

      Serial.print("📊 Sensores | Frontal: ");
      Serial.print(front);
      Serial.print(" cm | Izquierdo: ");
      Serial.print(left);
      Serial.print(" cm | Derecho: ");
      Serial.print(right);
      Serial.print(" cm | MQ135: ");
      Serial.println(mq);
    }
  }
}
