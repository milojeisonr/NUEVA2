#ifndef MAPA_ESPACIOS_H
#define MAPA_ESPACIOS_H

#include <Arduino.h>

// ========== SISTEMA DE MAPEO PARA NAVEGACIÓN INTELIGENTE ==========
// Mapea espacios visitados, zonas peligrosas y rutas libres

// Grid de mapeo (20x20 = 400 cm x 400 cm)
#define MAPA_GRID_SIZE 20
#define MAPA_CELDA_TAMAÑO 20  // cm por celda

// Tipos de celda
#define CELDA_DESCONOCIDA  0
#define CELDA_LIBRE        1
#define CELDA_PELIGROSA    2
#define CELDA_BLOQUEADA    3
#define CELDA_VISITADA     4

// --- Estructura del mapa ---
typedef struct {
  uint8_t tipo;           // Tipo de celda (arriba)
  uint8_t confianza;      // 0-100 qué tan segura es la clasificación
  uint16_t visitasCount;  // Cuántas veces visitada
  unsigned long ultimaVez; // Millis de última actualización
} CeldaMapa;

// --- Variables globales del mapa ---
extern CeldaMapa mapa[MAPA_GRID_SIZE][MAPA_GRID_SIZE];
extern int robotX;      // Posición X actual (0-19)
extern int robotY;      // Posición Y actual (0-19)
extern int robotDireccion;  // 0=norte, 1=este, 2=sur, 3=oeste

// --- Funciones del mapa ---
void inicializarMapa();
void actualizarMapaDesdeDistancias(long distFront, long distLeft, long distRight);
void marcarCeldaVisitada(int x, int y, uint8_t tipo, uint8_t confianza);
void actualizarPosicionRobot(int nuevoX, int nuevoY);
void rotarRobot(int nuevoDir);

// --- Análisis del mapa ---
int contarCeldasLibres();
int contarCeldasPeligrosas();
bool existeRutaAlternativa(int distFront, int distLeft, int distRight);
int calcularAreaLibre(int distancia);
uint8_t getConfianzaCelda(int x, int y);
void imprimirMapa();

// --- Limpieza ---
void limpiarMapa();
void resetearConfianza();  // Disminuir confianza de celdas antiguas

#endif
