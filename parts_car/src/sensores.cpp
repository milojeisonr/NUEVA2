#include <Arduino.h>
#include "sensores.h"
#include "motores.h"
#include "servo_rele.h"

// ===== VARIABLES GLOBALES =====
long distanciaFront = 0;
long distanciaLeft  = 0;
long distanciaRight = 0;

// 🆕 Memoria de giros para detección de atrapamiento
int memoriaGiros[10];
int indiceMemoria = 0;
int giroConsecutivos = 0;

// Memoria de obstáculos
int memoria[10];
int indiceMemoria_obstaculos = 0;

// Variables auxiliares
unsigned long ultimaReevaluacion = 0;

// ===== CONFIGURACIÓN INICIAL =====
void iniciarSensores() {
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);

  // Inicializar memoria de giros
  memset(memoriaGiros, 0, sizeof(memoriaGiros));
  indiceMemoria = 0;
  giroConsecutivos = 0;

  Serial.println("✅ Sensores ultrasónicos listos");
}

// ===== LECTURA DE DISTANCIA =====
long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 30000);
  long distancia = duracion * 0.034 / 2;

  if (distancia <= 0 || distancia > 400) distancia = 999;
  delay(60);

  Serial.print("📏 Pin TRIG: ");
  Serial.print(trigPin);
  Serial.print(" → ");
  Serial.print(distancia);
  Serial.println(" cm");

  return distancia;
}

long medirPromedio(int trig, int echo) {
  long total = 0;
  for (int i = 0; i < 3; i++) {
    total += medirDistancia(trig, echo);
    delay(60);
  }
  return total / 3;
}

// ===== MEDICIONES DIRECTAS =====
long medirDistanciaFront() { 
  distanciaFront = medirPromedio(TRIG_FRONT, ECHO_FRONT);
  return distanciaFront;
}

long medirDistanciaLeft() {
  distanciaLeft = medirPromedio(TRIG_LEFT, ECHO_LEFT);
  return distanciaLeft;
}

long medirDistanciaRight() {
  distanciaRight = medirPromedio(TRIG_RIGHT, ECHO_RIGHT);
  return distanciaRight;
}

// ===== AJUSTE DE VELOCIDAD SEGÚN OBSTÁCULO FRONTAL =====
void ajustarVelocidad(long dFront) {
  int pwmVal;
  if (dFront < 15) pwmVal = 255;                 // máximo (urgente)
  else if (dFront < 25) pwmVal = map(60,0,100,0,255); // reducir velocidad
  else pwmVal = getVelocidadPWM();

  ledcWrite(PWM_CH_A, pwmVal);
  ledcWrite(PWM_CH_B, pwmVal);
}

// ===== CORRECCIÓN DE TRAYECTORIA =====
void correccionTrayectoria(long dIzq, long dDer) {
  int pwm = getVelocidadPWM();
  if (dIzq - dDer > 5) {
    ledcWrite(PWM_CH_A, (int)(pwm * 0.8));
    ledcWrite(PWM_CH_B, pwm);
  } else if (dDer - dIzq > 5) {
    ledcWrite(PWM_CH_A, pwm);
    ledcWrite(PWM_CH_B, (int)(pwm * 0.8));
  }
}

// 🆕 ===== GUARDAR GIRO EN MEMORIA =====
// Registra cada giro (0=izq, 1=der, 2=centro)
void guardarGiro(int direccion) {
  memoriaGiros[indiceMemoria] = direccion;
  indiceMemoria = (indiceMemoria + 1) % 10;
  giroConsecutivos++;
  
  Serial.printf("📍 Giro registrado: %s (Total: %d)\n", 
    (direccion == 0) ? "IZQ" : (direccion == 1) ? "DER" : "CENTRO",
    giroConsecutivos);
}

// ===== GUARDAR MEMORIA DE OBSTÁCULOS =====
void guardarMemoria(int obstaculo) {
  memoria[indiceMemoria_obstaculos] = obstaculo;
  indiceMemoria_obstaculos = (indiceMemoria_obstaculos + 1) % 10;
}

// ===== DETECTAR ZONA BLOQUEADA =====
bool zonaBloqueada(int dir) {
  for (int i = 0; i < 10; i++) {
    if (memoria[i] == dir) return true;
  }
  return false;
}

// ===== ESCANEO DE ENTORNO CON SERVO =====
int escanearEntorno() {
  int mejorAngulo = 90;
  long mejorDist = 0;
  int delayServo = 70;

  for (int ang = 40; ang <= 140; ang += 10) {
    moverServo(ang);
    long d = medirDistancia(TRIG_FRONT, ECHO_FRONT);

    if (d < 10) delayServo = 200;
    else if (d < 25) delayServo = 120;
    else delayServo = 70;

    delay(delayServo);

    if (d > mejorDist) {
      mejorDist = d;
      mejorAngulo = ang;
    }
  }

  moverServo(90); // volver al centro
  Serial.printf("🔍 Mejor ángulo: %d° (distancia: %ld cm)\n", mejorAngulo, mejorDist);
  return mejorAngulo;
}

extern bool modoReal; // 👈 Importa la variable global

int leerMQ135() {
  if (modoReal) {
      int valor = analogRead(MQ135_PIN);
      Serial.print("🌡️ Lectura MQ135 (REAL): ");
      Serial.println(valor);
      return valor;
  } else {
      // Simulación de datos
      int simulado = random(200, 500);
      Serial.print("🧪 Lectura MQ135 (SIMULADA): ");
      Serial.println(simulado);
      return simulado;
  }
}

void iniciarMQ135() {
  // No necesita inicialización especial en ESP32 (ADC ya disponible),
  // pero dejamos la función para conservar la API y futuras configuraciones.
  Serial.println("✅ MQ135 inicializado (modo simulación por defecto)");
}
