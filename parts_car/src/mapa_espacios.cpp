#include "mapa_espacios.h"
#include <string.h>

// ========== VARIABLES GLOBALES DEL MAPA ==========
CeldaMapa mapa[MAPA_GRID_SIZE][MAPA_GRID_SIZE];
int robotX = 10;       // Centro del mapa
int robotY = 10;
int robotDireccion = 0; // 0=norte

// --- Historiales de confianza ---
static unsigned long ultimaLimpiezaConfianza = 0;
const unsigned long TIEMPO_DECAY_CONFIANZA = 30000;  // 30 seg

// ========== INICIALIZACIÓN ==========
void inicializarMapa() {
  // Llenar mapa con celdas desconocidas
  for (int x = 0; x < MAPA_GRID_SIZE; x++) {
    for (int y = 0; y < MAPA_GRID_SIZE; y++) {
      mapa[x][y].tipo = CELDA_DESCONOCIDA;
      mapa[x][y].confianza = 0;
      mapa[x][y].visitasCount = 0;
      mapa[x][y].ultimaVez = 0;
    }
  }
  
  // Marcar posición inicial como visitada
  marcarCeldaVisitada(robotX, robotY, CELDA_VISITADA, 100);
  
  Serial.println("📍 Mapa inicializado (20x20, 20cm/celda)");
}

// ========== ACTUALIZACIÓN DEL MAPA ==========
void actualizarMapaDesdeDistancias(long distFront, long distLeft, long distRight) {
  unsigned long now = millis();
  
  // Decay de confianza cada 30 segundos
  if (now - ultimaLimpiezaConfianza > TIEMPO_DECAY_CONFIANZA) {
    resetearConfianza();
    ultimaLimpiezaConfianza = now;
  }

  // Calcular direcciones relativas
  // Norte (adelante), Este (derecha), Oeste (izquierda)
  int dirAdelante = robotDireccion;
  int dirDerecha = (robotDireccion + 1) % 4;
  int dirIzquierda = (robotDireccion + 3) % 4;

  // Offset de posición según dirección
  int offsetX[] = {0, 1, 0, -1};  // norte, este, sur, oeste
  int offsetY[] = {1, 0, -1, 0};

  // ===== Marcar celdas detectadas por distancia frontal =====
  if (distFront < 200) {
    int celdas = distFront / MAPA_CELDA_TAMAÑO;
    int celdaX = robotX + (offsetX[dirAdelante] * celdas);
    int celdaY = robotY + (offsetY[dirAdelante] * celdas);
    
    if (celdaX >= 0 && celdaX < MAPA_GRID_SIZE && celdaY >= 0 && celdaY < MAPA_GRID_SIZE) {
      if (distFront < 15) {
        marcarCeldaVisitada(celdaX, celdaY, CELDA_BLOQUEADA, 95);
      } else if (distFront < 30) {
        marcarCeldaVisitada(celdaX, celdaY, CELDA_PELIGROSA, 80);
      } else {
        marcarCeldaVisitada(celdaX, celdaY, CELDA_LIBRE, 70);
      }
    }
  }

  // ===== Marcar celdas laterales =====
  if (distLeft < 200) {
    int celdas = distLeft / MAPA_CELDA_TAMAÑO;
    int celdaX = robotX + (offsetX[dirIzquierda] * celdas);
    int celdaY = robotY + (offsetY[dirIzquierda] * celdas);
    
    if (celdaX >= 0 && celdaX < MAPA_GRID_SIZE && celdaY >= 0 && celdaY < MAPA_GRID_SIZE) {
      uint8_t tipo = (distLeft < 20) ? CELDA_BLOQUEADA : CELDA_LIBRE;
      marcarCeldaVisitada(celdaX, celdaY, tipo, 60);
    }
  }

  if (distRight < 200) {
    int celdas = distRight / MAPA_CELDA_TAMAÑO;
    int celdaX = robotX + (offsetX[dirDerecha] * celdas);
    int celdaY = robotY + (offsetY[dirDerecha] * celdas);
    
    if (celdaX >= 0 && celdaX < MAPA_GRID_SIZE && celdaY >= 0 && celdaY < MAPA_GRID_SIZE) {
      uint8_t tipo = (distRight < 20) ? CELDA_BLOQUEADA : CELDA_LIBRE;
      marcarCeldaVisitada(celdaX, celdaY, tipo, 60);
    }
  }
}

// ========== MARCAR CELDA ==========
void marcarCeldaVisitada(int x, int y, uint8_t tipo, uint8_t confianza) {
  if (x < 0 || x >= MAPA_GRID_SIZE || y < 0 || y >= MAPA_GRID_SIZE) return;
  
  // Fusionar confianza (promediar si existe)
  if (mapa[x][y].confianza > 0) {
    mapa[x][y].confianza = (mapa[x][y].confianza + confianza) / 2;
  } else {
    mapa[x][y].confianza = confianza;
  }
  
  mapa[x][y].tipo = tipo;
  mapa[x][y].visitasCount++;
  mapa[x][y].ultimaVez = millis();
}

// ========== ACTUALIZAR POSICIÓN DEL ROBOT ==========
void actualizarPosicionRobot(int nuevoX, int nuevoY) {
  if (nuevoX < 0) nuevoX = 0;
  if (nuevoX >= MAPA_GRID_SIZE) nuevoX = MAPA_GRID_SIZE - 1;
  if (nuevoY < 0) nuevoY = 0;
  if (nuevoY >= MAPA_GRID_SIZE) nuevoY = MAPA_GRID_SIZE - 1;
  
  robotX = nuevoX;
  robotY = nuevoY;
  marcarCeldaVisitada(robotX, robotY, CELDA_VISITADA, 100);
}

void rotarRobot(int nuevoDir) {
  if (nuevoDir < 0) nuevoDir = 3;
  if (nuevoDir > 3) nuevoDir = 0;
  robotDireccion = nuevoDir;
}

// ========== ANÁLISIS DEL MAPA ==========
int contarCeldasLibres() {
  int cuenta = 0;
  for (int x = 0; x < MAPA_GRID_SIZE; x++) {
    for (int y = 0; y < MAPA_GRID_SIZE; y++) {
      if (mapa[x][y].tipo == CELDA_LIBRE && mapa[x][y].confianza > 50) {
        cuenta++;
      }
    }
  }
  return cuenta;
}

int contarCeldasPeligrosas() {
  int cuenta = 0;
  for (int x = 0; x < MAPA_GRID_SIZE; x++) {
    for (int y = 0; y < MAPA_GRID_SIZE; y++) {
      if ((mapa[x][y].tipo == CELDA_PELIGROSA || mapa[x][y].tipo == CELDA_BLOQUEADA) && 
          mapa[x][y].confianza > 50) {
        cuenta++;
      }
    }
  }
  return cuenta;
}

// ===== Detectar si existe ruta alternativa ======
bool existeRutaAlternativa(int distFront, int distLeft, int distRight) {
  // Si tiene opciones laterales significativas, existe alternativa
  int umbral = max(20, distFront / 2);
  return (distLeft > umbral || distRight > umbral);
}

// ===== Calcular área libre explorada =====
int calcularAreaLibre(int distancia) {
  // Estimar % del área explorada libre
  int totalCeldas = MAPA_GRID_SIZE * MAPA_GRID_SIZE;
  int celdas = distancia / MAPA_CELDA_TAMAÑO;
  int circuloArea = celdas * celdas * 3;  // Aproximación de círculo
  
  if (circuloArea > totalCeldas) return 100;
  return (circuloArea * 100) / totalCeldas;
}

uint8_t getConfianzaCelda(int x, int y) {
  if (x < 0 || x >= MAPA_GRID_SIZE || y < 0 || y >= MAPA_GRID_SIZE) return 0;
  return mapa[x][y].confianza;
}

// ===== Debug: Imprimir mapa simplificado =====
void imprimirMapa() {
  Serial.println("\n╔════ MAPA 20x20 (R=Robot) ════╗");
  for (int y = MAPA_GRID_SIZE - 1; y >= 0; y--) {
    Serial.print("║ ");
    for (int x = 0; x < MAPA_GRID_SIZE; x++) {
      if (x == robotX && y == robotY) {
        Serial.print("R");
      } else {
        char c;
        switch (mapa[x][y].tipo) {
          case CELDA_LIBRE:      c = '·'; break;
          case CELDA_PELIGROSA:  c = '◐'; break;
          case CELDA_BLOQUEADA:  c = '█'; break;
          case CELDA_VISITADA:   c = '*'; break;
          default:               c = ' '; break;
        }
        Serial.print(c);
      }
    }
    Serial.println(" ║");
  }
  Serial.println("╚═══════════════════════════════╝");
  Serial.printf("📊 Celdas libres: %d | Peligrosas: %d\n", 
    contarCeldasLibres(), contarCeldasPeligrosas());
}

// ===== Limpiar mapa =====
void limpiarMapa() {
  for (int x = 0; x < MAPA_GRID_SIZE; x++) {
    for (int y = 0; y < MAPA_GRID_SIZE; y++) {
      mapa[x][y].tipo = CELDA_DESCONOCIDA;
      mapa[x][y].confianza = 0;
      mapa[x][y].visitasCount = 0;
    }
  }
  robotX = 10;
  robotY = 10;
  robotDireccion = 0;
  Serial.println("🗑️ Mapa limpiado");
}

// ===== Decay de confianza (celdas antiguas menos confiables) =====
void resetearConfianza() {
  unsigned long now = millis();
  for (int x = 0; x < MAPA_GRID_SIZE; x++) {
    for (int y = 0; y < MAPA_GRID_SIZE; y++) {
      if (mapa[x][y].confianza > 0) {
        unsigned long tiempoTranscurrido = now - mapa[x][y].ultimaVez;
        // Reducir confianza 1% cada 10 segundos
        int reduccion = (tiempoTranscurrido / 10000) * 1;
        mapa[x][y].confianza = max(0, (int)mapa[x][y].confianza - reduccion);
      }
    }
  }
}
