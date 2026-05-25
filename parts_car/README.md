# 🤖 Carro Robótico Autónomo con ESP32

## 📋 Descripción General

Este es un **carro robótico inteligente basado en ESP32** que puede operar en dos modos:
- **Modo Manual**: Control desde navegador web con interfaz gráfica
- **Modo Autónomo**: Navegación inteligente con evitación de obstáculos y mapeo espacial

El robot utiliza **3 sensores ultrasónicos** para detectar obstáculos, **servo para escaneo**, **relé para control de ventilador/purificador**, y un **sensor MQ135** para detección de gases (simulado o real).

---

## ✨ Características Principales

✅ **Control Manual Web**: Botones y joystick en navegador  
✅ **Modo Autónomo Inteligente**: 8 estados de máquina con lógica avanzada  
✅ **Mapeo Espacial**: Sistema de grid 20x20 con confianza de celdas  
✅ **Detección de Atrapamiento**: Prevención automática de loops infinitos  
✅ **Velocidad Adaptativa**: Ajuste dinámico según obstáculos  
✅ **Filtrado de Sensores**: Media exponencial para suavizar lecturas  
✅ **Servo Escaneo**: 3 puntos (izq, centro, der) para decisiones inteligentes  
✅ **Histéresis Inteligente**: Evita oscilaciones entre estados  
✅ **MQ135 Real/Simulado**: Toggle entre lectura real y simulación  

---

## 🏗️ Estructura del Proyecto

```
parts_car/
├── src/
│   ├── main.cpp                    # Punto de entrada, setup() y loop()
│   ├── webserver_setup.cpp         # Rutas HTTP y servidor AsyncWeb
│   ├── modo_autonomo.cpp           # Máquina de 8 estados autónoma
│   ├── motores.cpp                 # Control PWM de motores
│   ├── sensores.cpp                # Lectura de ultrasónicos y MQ135
│   ├── servo_rele.cpp              # Control de servo y relé
│   └── mapa_espacios.cpp           # Sistema de mapeo 2D inteligente
│
├── include/
│   ├── main.h                      # (si aplica)
│   ├── webserver_setup.h           # Declaraciones de servidor
│   ├── modo_autonomo.h             # Estados y funciones autónomas
│   ├── motores.h                   # API de motores
│   ├── sensores.h                  # API de sensores
│   ├── servo_rele.h                # API de servo y relé
│   └── mapa_espacios.h             # API de mapeo
│
├── data/                           # Archivos web estáticos (SPIFFS)
│   ├── index.html                  # Página principal
│   ├── control.html                # Interfaz de control manual
│   ├── style.css                   # Estilos
│   └── script.js                   # Lógica frontend
│
├── platformio.ini                  # Configuración de compilación
└── README.md                       # Este archivo
```

---

## 🔌 Conexiones de Hardware

### 🎯 Motor Driver (H-Bridge)

| Función | Pin ESP32 | Notas |
|---------|-----------|-------|
| IN1 (Motor A Dir) | GPIO 25 | Control dirección motor izquierdo |
| IN2 (Motor A Dir) | GPIO 26 | Control dirección motor izquierdo |
| IN3 (Motor B Dir) | GPIO 32 | Control dirección motor derecho |
| IN4 (Motor B Dir) | GPIO 33 | Control dirección motor derecho |
| ENA (Motor A PWM) | GPIO 27 | Control velocidad motor izquierdo |
| ENB (Motor B PWM) | GPIO 14 | Control velocidad motor derecho |

**Alimentación**: H-Bridge requiere 5-12V separados, GND común con ESP32

### 📡 Sensores Ultrasónicos (3x HC-SR04)

| Sensor | TRIG | ECHO | Orientación |
|--------|------|------|-------------|
| Frontal | GPIO 5 | GPIO 18 | Adelante |
| Izquierdo | GPIO 19 | GPIO 21 | Lateral izquierdo |
| Derecho | GPIO 22 | GPIO 23 | Lateral derecho |

**Alimentación**: 5V, GND común

### 🎛️ Servo Motor

| Parámetro | Valor |
|-----------|-------|
| Pin | GPIO 13 |
| Alimentación | 5V |
| Ángulos | 0-180° |
| Posiciones | 40°(Izq) / 90°(Centro) / 140°(Der) |

### 🔴 Relé (Ventilador/Purificador)

| Parámetro | Valor |
|-----------|-------|
| Pin | GPIO 17 |
| Lógica | Activo en BAJO (LOW=ON, HIGH=OFF) |
| Alimentación | 5V |

### 🌡️ Sensor MQ135 (Calidad del aire)

| Parámetro | Valor |
|-----------|-------|
| Pin ADC | GPIO 34 |
| Alimentación | 5V |
| Rango | 0-4095 (ADC 12-bit) |

---

## 🧠 Sistema de Modo Autónomo

### Estados de la Máquina (8 estados)

```
AS_IDLE
  ↓
AS_NAVIGATE ← (estado principal)
  ├→ AS_ALERT (obstáculo mediano)
  │  └→ AS_CRITICAL (obstáculo muy cercano)
  │     └→ AS_SCANNING (escanear alternativas)
  │
  ├→ AS_SCANNING (3 puntos: izq/centro/der)
  │  └→ AS_TURNING (ejecutar giro)
  │
  ├→ AS_BACKING (retroceder 400ms)
  │
  └→ AS_ESCAPE (giro 180° si atrapado)
```

### Parámetros de Distancia

| Zona | Distancia | Acción |
|------|-----------|--------|
| **Libre** | > 45 cm | Navegar a máxima velocidad |
| **Alerta** | 30-45 cm | Reducir velocidad 20% |
| **Crítica** | 15-30 cm | Reducir velocidad 50% |
| **Bloqueada** | < 15 cm | Escanear o retroceder |

### Detección de Atrapamiento

- **Registra cada giro** (izquierda/derecha)
- **Detecta 3+ giros consecutivos** del mismo lado
- **Ejecuta escape 180°** automáticamente
- **Resetea contador** cuando navega recto

---

## 🛣️ Rutas Web del Servidor

### 📄 Archivos Estáticos
- `GET /` → index.html
- `GET /index.html` → Página principal
- `GET /control.html` → Interfaz control manual

### 🎮 Control Manual
- `GET /forward?vel=50` → Adelante (50%)
- `GET /backward?vel=50` → Atrás (50%)
- `GET /left?vel=50` → Girar izquierda
- `GET /right?vel=50` → Girar derecha
- `GET /stop` → Detener motores
- `GET /toggleControl?state=on|off` → Habilitar/deshabilitar control

### 📊 Velocidades
- `GET /getVelocidad` → Obtener velocidad manual actual
- `GET /setVelocidad?valor=75` → Establecer velocidad manual
- `GET /getSpeeds` → Obtener manual + autónoma
- `GET /setSpeedLevel?mode=manual|autonomo&level=1|2|3` → Preset velocidad

### 🤖 Modo Autónomo
- `GET /setMode?mode=autonomo|manual` → Cambiar modo
- `GET /startAutonomo` → Iniciar navegación autónoma
- `GET /stopAutonomo` → Detener navegación
- `GET /setPresetAutonomo?nivel=1|2|3` → Establecer límite máximo
  - Nivel 1: 30% máximo (🐢 Lento)
  - Nivel 2: 50% máximo (🚗 Normal)
  - Nivel 3: 80% máximo (🚀 Rápido)
- `GET /setVelocidadAutonomo?valor=60` → Velocidad actual (respeta límite)
- `GET /getVelocidadesAutonomo` → Info velocidades

### 📡 Sensores
- `GET /ultrasonic` → {"front": cm, "left": cm, "right": cm}
- `GET /mq135` → {"mq135": valor}
- `GET /debugAutono` → JSON completo de estado autónomo

### 🎛️ Actuadores
- `GET /ventilador?state=on|off` → Encender/apagar ventilador
- `GET /getVentilador` → Estado actual ventilador
- `GET /setRealMode?active=1|0` → Modo real/simulación MQ135

### 🚨 Emergencias
- `GET /emergencyStop` → Parada total de emergencia

---

## 📚 API de Funciones Principales

### motores.h
```c
void inicializarMotores();           // Setup de PWM
void moverAdelante(int porcentaje);  // Navegar adelante
void moverAtras(int porcentaje);     // Navegar atrás
void girarIzquierda(int porcentaje); // Giro fuerte izq
void girarDerecha(int porcentaje);   // Giro fuerte der
void girarSuaveIzquierda();          // Corrección suave
void girarSuaveDerecha();            // Corrección suave
void detenerMotores();               // Stop
void frenarSuave();                  // Desaceleración gradual
void setVelocidad(int pct);          // Establecer velocidad (0-100%)
int getVelocidad();                  // Obtener velocidad actual
void setPresetSpeed(bool auton, int level);  // Presets (1..3)
```

### sensores.h
```c
void iniciarSensores();              // Setup ultrasónicos
long medirDistanciaFront();           // Lectura frontal
long medirDistanciaLeft();            // Lectura izquierda
long medirDistanciaRight();           // Lectura derecha
void guardarGiro(int dir);            // Registrar giro (0,1,2)
int leerMQ135();                      // Lectura de gas
void iniciarMQ135();                  // Setup MQ135
```

### modo_autonomo.h
```c
void ejecutarModoAutonomo(int mode); // Máquina de estados principal
void iniciarModoAutonomo();           // Comenzar autonomía
void detenerModoAutonomo();           // Parar autonomía
bool estaAutonomoActivo();            // ¿Está activa?
void setPresetVelocidadAutonoma(int preset);  // Límite máximo
void setVelocidadAutonoma(int porcentaje);    // Velocidad actual
int getVelocidadAutonoma();           // Obtener velocidad actual
int getVelocidadMaximaAutonoma();     // Obtener límite
String getAutonomoDebug();            // JSON debug
```

### servo_rele.h
```c
void iniciarServoYRele();             // Setup servo + relé
void moverServo(int angulo);          // Ángulo libre (0-180)
void moverServoIzquierda();           // Posición 40°
void moverServoCentro();              // Posición 90°
void moverServoDerecha();             // Posición 140°
void barridoServo();                  // Barrido completo
void encenderVentilador();            // ON relé
void apagarVentilador();              // OFF relé
void setVentilador(bool estado);      // Toggle
```

### mapa_espacios.h
```c
void inicializarMapa();               // Setup grid 20x20
void actualizarMapaDesdeDistancias(); // Fusionar sensores
void marcarCeldaVisitada(x, y, tipo, confianza);
int contarCeldasLibres();             // Análisis
int contarCeldasPeligrosas();         // Análisis
bool existeRutaAlternativa();         // ¿Hay salida?
void imprimirMapa();                  // Debug en serial
void limpiarMapa();                   // Reset
```

---

## 🚀 Cómo Usar

### 1️⃣ Instalación del Firmware

```bash
# Clonar/descargar proyecto
cd parts_car

# Compilar
pio run

# Subir a ESP32
pio run -t upload

# Subir archivos web (SPIFFS)
pio run -t uploadfs

# Monitorear puerto serial
pio device monitor --baud 115200
```

### 2️⃣ Conectar a WiFi

El robot crea un **Punto de Acceso (AP)**:
- **SSID**: `RobotESP32`
- **Contraseña**: `12345678`
- **IP**: `192.168.4.1`

Conectar desde dispositivo → Abrir navegador → `http://192.168.4.1`

### 3️⃣ Usar Interfaz Web

**Página Principal** (`/index.html`):
- Selector de Modo (Manual / Autónomo)
- Botones de Control
- Monitor de sensores en tiempo real
- Selector de velocidad (presets 1-3)

**Control Manual**:
1. Cambiar a "Modo Manual"
2. Seleccionar nivel de velocidad
3. Usar botones o joystick para mover
4. Botón STOP para detener

**Modo Autónomo**:
1. Cambiar a "Modo Autónomo"
2. Seleccionar nivel máximo (🐢 / 🚗 / 🚀)
3. Botón "INICIAR" para comenzar
4. Botón "DETENER" para parar

---

## 🛠️ Cambios Recientes (Actualización 2026-05-25)

✅ **motores.cpp**: Agregadas funciones faltantes
- `girarSuaveIzquierda()` - Corrección con 60% motor izq
- `girarSuaveDerecha()` - Corrección con 60% motor der
- `frenarSuave()` - Desaceleración gradual en 50ms steps

✅ **sensores.cpp**: Completadas variables y funciones
- `memoriaGiros[10]` - Buffer de giros históricos
- `guardarGiro(dir)` - Registra cada giro para detección
- `indiceMemoria` - Índice circular de giros

✅ **webserver_setup.cpp**: Mejorada lógica
- Función `setSpeedLevel` actualizada con validación
- Ruta `/getVentilador` nueva para estado
- Comentarios mejorados

✅ **sensores.h**: Declaraciones actualizadas
- Exporta variables `memoriaGiros`, `indiceMemoria`, `giroConsecutivos`
- Declara `guardarGiro()`

---

## ⚠️ Notas Técnicas

### Limitaciones Actuales
1. **pulseIn()** bloquea el loop durante mediciones
   - Impacto mínimo en ESP32, pero considerar interrupciones para futuro
   
2. **Fragmentación de Heap**
   - Uso intensivo de `String` en webserver
   - Monitorear memoria en uso prolongado

3. **SPIFFS limitado**
   - Máx ~1.5 MB para archivos web
   - Optimizar si se agregan recursos

### Recomendaciones
- 📌 Usar GND común entre todas las fuentes de alimentación
- 📌 Proteger ESP32 con filtros de ruido
- 📌 Calibrar sensores ultrasónicos antes de usar
- 📌 Testear en espacios abiertos primero (mínimo 1m×1m)
- 📌 Mantener batería entre 4.5V-5.2V para estabilidad

---

## 🐛 Depuración

### Monitor Serial
```
Serial.begin(115200);
// Ver todos los eventos del robot en tiempo real
```

### Ruta Debug
```
GET /debugAutono → JSON con estado completo
```

### Mapeo Visual
```c
imprimirMapa(); // Imprime grid ASCII en serial
// R = Robot, · = Libre, ◐ = Peligrosa, █ = Bloqueada, * = Visitada
```

---

## 📄 Licencia

Proyecto educativo - Libre para modificar y distribuir

---

## 👤 Autor

**milojeisonr** - 2026

---

## 🤝 Contribuciones

Si encuentras bugs o tienes mejoras:
1. Prueba los cambios localmente
2. Documenta el problema/solución
3. Sube un PR con descripción clara

---

**¡Feliz robótica! 🤖**
