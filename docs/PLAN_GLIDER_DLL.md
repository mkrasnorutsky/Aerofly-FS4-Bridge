# Plan: AeroflyGlider DLL — Integración con XCSoar / SeeYou Navigator

> **Documento de contexto para Claude.**
> Este archivo describe el objetivo, la arquitectura y todos los detalles técnicos
> necesarios para implementar el DLL de planeadores desde cero, en una nueva sesión.

---

## 1. Objetivo

Crear un nuevo DLL llamado **`AeroflyGlider.dll`** que integre directamente Aerofly FS4
con las apps de navegación de planeadores **XCSoar** y **SeeYou Navigator (Naviter)**,
sin ningún programa intermedio externo.

El DLL se instala exactamente igual que los otros DLLs del proyecto:

```
%USERPROFILE%\Documents\Aerofly FS 4\external_dll\
    ├── AeroflyGlider.dll
    └── AeroflyGlider.ini    ← archivo de configuración del usuario
```

---

## 2. Repositorio y base de código

- **Repositorio:** `jlgabriel/Aerofly-FS4-Bridge`
- **Rama de desarrollo:** crear nueva rama tipo `claude/glider-dll-<id>`
- **Base de código de referencia:** `simplified/aerofly_reader_dll.cpp`

El DLL de planeadores debe tomar el `simplified/aerofly_reader_dll.cpp` como punto
de partida, ya que comparte la misma arquitectura:

- `DllMain` con `DLL_PROCESS_ATTACH` / `DLL_PROCESS_DETACH`
- Callbacks `CreateInterface` / `DestroyInterface` / `Update` del SDK de Aerofly
- Macros `HANDLE_DOUBLE` / `HANDLE_VECTOR3D` para registrar mensajes
- TCP server con hilo separado (no bloquea el hilo principal de Aerofly)
- Sanitización `safe_double()` contra NaN/Inf

**Diferencia clave:** en lugar de emitir JSON, emite sentencias **NMEA 0183** de texto plano.

---

## 3. Cómo funciona la integración NMEA

### 3.1 Protocolo de conexión

Tanto XCSoar como SeeYou Navigator pueden conectarse a un servidor TCP que emite
sentencias NMEA de texto. El DLL actúa como ese servidor:

```
Aerofly FS4
    │  tm_external_message (50-60 Hz)
    ▼
AeroflyGlider.dll
    │
    │  NMEA sentences por TCP (configurable, default 4 Hz)
    ▼
XCSoar (Device: TCP Client)          ← se conecta a localhost:<nmea_port>
SeeYou Navigator (Network GPS)       ← se conecta a localhost:<nmea_port>
```

### 3.2 Configuración en XCSoar

En XCSoar → Config → Devices → New Device:
- **Driver:** LXNAV (para $LXWP0) o NMEA (para sentencias básicas)
- **Port:** TCP Client
- **IP Address:** `127.0.0.1`
- **Port Number:** (valor configurado en AeroflyGlider.ini, default `4353`)

### 3.3 Configuración en SeeYou Navigator (Naviter)

En SeeYou → Settings → GPS → Add → Network (TCP):
- **IP:** `127.0.0.1`
- **Port:** (mismo valor del INI)

---

## 4. Sentencias NMEA a emitir

Todas las sentencias NMEA terminan en `\r\n` y llevan checksum `*XX`.

### 4.1 $GPRMC — Posición, velocidad y rumbo (obligatoria)

```
$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n
```

| Campo | Fuente Aerofly |
|-------|---------------|
| Hora UTC | timestamp del DLL → HHMMSS.ss |
| Estado | `A` (Active) si `data_valid`, `V` si no |
| Latitud | `Aircraft.Latitude` (rad → grados+minutos DDMM.MMMMM) |
| N/S | según signo de latitud |
| Longitud | `Aircraft.Longitude` (rad → DDDMM.MMMMM, normalizar 0-360 → -180/+180) |
| E/W | según signo de longitud |
| Velocidad | `Aircraft.GroundSpeed` (m/s → knots) |
| Track | `Aircraft.TrueHeading` (rad → grados) |
| Fecha | fecha actual del sistema → DDMMYY |
| Variación magnética | calcular = TrueHeading - MagneticHeading (rad → grados) |

### 4.2 $GPGGA — Fix GPS con altitud (obligatoria para altitud)

```
$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n
```

| Campo | Fuente Aerofly |
|-------|---------------|
| Hora UTC | timestamp del DLL |
| Lat/Lon | igual que $GPRMC |
| Fix quality | `1` (GPS fix) siempre que data_valid |
| Nº satélites | fijo: `08` (valor decorativo) |
| HDOP | fijo: `1.0` |
| Altitud MSL | `Aircraft.Altitude` (metros, no convertir — NMEA usa metros) |
| Unidad | `M` |
| Separación geoide | fijo: `0.0,M` |

> **IMPORTANTE:** `Aircraft.Altitude` en el DLL se almacena en **feet** (ya convertida
> en aerofly_reader_dll.cpp línea 190). Para NMEA hay que usar metros.
> Convertir: `alt_m = alt_ft * 0.3048`

### 4.3 $LXWP0 — LX Navigation (variométro + velocidad, clave para planeadores)

Esta es la sentencia más importante para XCSoar como app de planeadores.
XCSoar la usa para el variométro, velocidad indicada y viento.

```
$LXWP0,Y,nnn.n,aaaa.a,vv.vv,wddd,wsss,0,0,hdg,0*XX\r\n
```

Formato detallado:
```
$LXWP0,<LogEnable>,<IAS_kmh>,<PressAlt_m>,<Vario_ms>,<WindDir>,<WindSpd_kmh>,0,0,<Hdg_deg>,0*<CSUM>
```

| Campo | Descripción | Fuente Aerofly |
|-------|-------------|---------------|
| LogEnable | `Y` siempre | fijo |
| IAS_kmh | Velocidad indicada en km/h | `Aircraft.IndicatedAirspeed` (m/s × 3.6) |
| PressAlt_m | Altitud barométrica en metros | `Aircraft.Altitude` (ft × 0.3048) |
| Vario_ms | Velocidad vertical en m/s | `Aircraft.VerticalSpeed` (ya en m/s) |
| WindDir | Dirección del viento en grados | `Aircraft.Wind` vector → calcular dirección |
| WindSpd_kmh | Velocidad del viento en km/h | `Aircraft.Wind` vector → magnitud × 3.6 |
| 0,0 | G-force, No usado | fijos en `0` |
| Hdg_deg | Rumbo verdadero en grados | `Aircraft.TrueHeading` (rad → grados) |
| 0 | No usado | fijo |

**Cálculo del viento desde el vector `Aircraft.Wind`:**
```cpp
// Aircraft.Wind es un Vector3d en metros/segundo (ejes del mundo)
// wind.x = componente Este, wind.y = componente Norte, wind.z = componente arriba
double wind_speed_ms = sqrt(wind.x*wind.x + wind.y*wind.y);
double wind_dir_rad  = atan2(-wind.x, -wind.y);  // dirección DE donde viene
double wind_dir_deg  = fmod(wind_dir_rad * 180.0 / M_PI + 360.0, 360.0);
double wind_speed_kmh = wind_speed_ms * 3.6;
```

### 4.4 $PGRMZ — Altitud barométrica Garmin (compatibilidad extra)

```
$PGRMZ,1234,f,2*XX\r\n
```

| Campo | Fuente |
|-------|--------|
| Altitud | `Aircraft.Altitude` (feet, sin convertir) |
| Unidad | `f` (feet) |
| Fix type | `2` (2D fix) |

XCSoar y SeeYou reconocen esta sentencia como altitud barométrica de respaldo.

### 4.5 Orden y frecuencia de emisión

En cada "tick" (según `nmea_rate_hz` del config), emitir en este orden:
1. `$GPRMC`
2. `$GPGGA`
3. `$LXWP0`
4. `$PGRMZ` (opcional, controlado por config)

---

## 5. Formato del archivo de configuración

### 5.1 Nombre y ubicación

El DLL obtiene su propia ruta mediante `GetModuleFileNameA(hInstance, ...)` en
`DLL_PROCESS_ATTACH`, luego reemplaza la extensión `.dll` por `.ini`.

```
%USERPROFILE%\Documents\Aerofly FS 4\external_dll\AeroflyGlider.ini
```

Si el archivo **no existe**, el DLL debe **crearlo automáticamente** con todos los
valores por defecto y comentarios explicativos en inglés.

### 5.2 Contenido completo del AeroflyGlider.ini

```ini
# AeroflyGlider.ini
# Configuration file for AeroflyGlider DLL
# Restart Aerofly FS4 after making changes.
# Lines starting with # are comments.

[network]
# TCP port for NMEA output
# XCSoar: Config > Devices > TCP Client > this port
# SeeYou Navigator: Settings > GPS > Network > this port
nmea_port = 4353

# How many times per second to send NMEA sentences (1-20 Hz)
# XCSoar works well with 4 Hz. Higher rates use more CPU.
nmea_rate_hz = 4

# Maximum simultaneous clients (XCSoar + SeeYou at the same time)
max_clients = 4

[nmea]
# Which NMEA sentences to emit (true/false)
emit_gprmc  = true
emit_gpgga  = true
emit_lxwp0  = true    # Vario + IAS for XCSoar energy display
emit_pgrmz  = true    # Barometric altitude fallback

# Talker ID for standard sentences (GP = GPS, GN = GNSS combined)
# GP is recommended for maximum compatibility
talker_id = GP

[glider]
# Altitude reference for $LXWP0 pressure altitude
# Options: msl (mean sea level) or agl (above ground level)
altitude_reference = msl

# Wind vector source
# Options: sim (Aircraft.Wind from sim) or none (omit wind from LXWP0)
wind_source = sim

# If true, emit $LXWP0 even when on ground (for testing)
emit_on_ground = false

[debug]
# Log NMEA sentences to a file (slows performance, for debugging only)
log_to_file   = false
log_filename  = AeroflyGlider_debug.log
```

### 5.3 Parser del INI en C++

Implementar un parser **sin dependencias externas**, solo Win32 API + STL:

```cpp
struct GliderConfig {
    // [network]
    int    nmea_port        = 4353;
    int    nmea_rate_hz     = 4;
    int    max_clients      = 4;

    // [nmea]
    bool   emit_gprmc       = true;
    bool   emit_gpgga       = true;
    bool   emit_lxwp0       = true;
    bool   emit_pgrmz       = true;
    std::string talker_id   = "GP";

    // [glider]
    bool   alt_msl          = true;    // false = agl
    bool   wind_from_sim    = true;    // false = omit
    bool   emit_on_ground   = false;

    // [debug]
    bool   log_to_file      = false;
    std::string log_filename = "AeroflyGlider_debug.log";
};
```

El parser debe:
- Ignorar líneas que empiezan con `#`
- Ignorar secciones `[nombre]` (solo para organización visual)
- Parsear líneas con formato `clave = valor` o `clave=valor`
- Ignorar espacios alrededor de `=`
- Usar valor por defecto si la clave no existe o tiene valor inválido

---

## 6. Estructura del DLL

### 6.1 Archivos a crear

```
glider/
├── CMakeLists.txt
├── README.md
├── build.bat
├── aerofly_glider_dll.cpp     ← archivo principal, basado en simplified/aerofly_reader_dll.cpp
└── examples/
    ├── test_nmea_reader.py    ← script Python para verificar salida NMEA
    └── test_xcsoar_sim.py     ← simula lo que haría XCSoar
```

### 6.2 Estructura interna del .cpp

Seguir exactamente la misma estructura de `simplified/aerofly_reader_dll.cpp`:

```cpp
// 1. Includes y pragmas
// 2. safe_double(), get_time_us(), HANDLE_DOUBLE macros
// 3. load_config() — lee AeroflyGlider.ini
// 4. write_default_config() — crea el INI si no existe
// 5. MESSAGE_LIST macro con los mensajes de Aerofly necesarios
// 6. struct GliderData { ... }  (datos del sim)
// 7. class NmeaBuilder { ... }  (construye sentencias NMEA)
//    - nmea_checksum(const char* sentence)
//    - build_gprmc(...)
//    - build_gpgga(...)
//    - build_lxwp0(...)
//    - build_pgrmz(...)
// 8. class TcpNmeaServer { ... } (servidor TCP, adaptado de TCPDataServer)
// 9. class AeroflyGliderInterface { ... } (implementa tm_external_interface)
//    - Init()
//    - Update()
// 10. CreateInterface() / DestroyInterface() / DllMain()
```

### 6.3 Mensajes del SDK de Aerofly necesarios

Estos son los mensajes de `tm_external_message` que hay que registrar
(todos ya presentes en `simplified/aerofly_reader_dll.cpp`):

```
Aircraft.Latitude           (Radiant)
Aircraft.Longitude          (Radiant)
Aircraft.Altitude           (Meter)        ← DLL convierte a feet para almacenar
Aircraft.Height             (Meter)        ← DLL convierte a feet
Aircraft.Pitch              (Radiant)
Aircraft.Bank               (Radiant)
Aircraft.TrueHeading        (Radiant)
Aircraft.MagneticHeading    (Radiant)
Aircraft.IndicatedAirspeed  (MeterPerSecond)
Aircraft.GroundSpeed        (MeterPerSecond)
Aircraft.VerticalSpeed      (MeterPerSecond)
Aircraft.Wind               (Vector3d, MeterPerSecond)
Aircraft.OnGround           (None, boolean 0/1)
Aircraft.Name               (String)
Aircraft.NearestAirportIdentifier (String)
```

**Nota sobre almacenamiento:**
- `Aircraft.Altitude` y `Aircraft.Height` llegan en metros desde el SDK
- En el DLL simplified se almacenan ya convertidas a **feet** para simplificar el JSON
- Para NMEA necesitamos metros en algunas sentencias → convertir de vuelta: `ft * 0.3048`
- **Recomendación para el glider DLL:** almacenar en **metros** directamente,
  para simplificar la construcción de NMEA. Es un cambio respecto al simplified DLL.

---

## 7. Implementación del checksum NMEA

El checksum NMEA es el XOR de todos los bytes entre `$` y `*` (sin incluirlos):

```cpp
static uint8_t nmea_checksum(const char* sentence) {
    // sentence apunta al primer caracter DESPUÉS del '$'
    // calcular hasta el '*' o fin de cadena
    uint8_t csum = 0;
    while (*sentence && *sentence != '*') {
        csum ^= (uint8_t)(*sentence);
        ++sentence;
    }
    return csum;
}
```

Construcción típica de una sentencia:
```cpp
// Paso 1: construir el cuerpo sin $ ni checksum
char body[256];
snprintf(body, sizeof(body), "GPRMC,%s,A,...", time_str);

// Paso 2: calcular checksum
uint8_t cs = nmea_checksum(body);

// Paso 3: armar sentencia completa
char sentence[300];
snprintf(sentence, sizeof(sentence), "$%s*%02X\r\n", body, cs);
```

---

## 8. Conversiones de coordenadas para NMEA

NMEA usa formato `DDMM.MMMMM` (grados y minutos decimales), no grados decimales.

```cpp
// Convertir radianes a NMEA lat/lon
// lat_rad: Aircraft.Latitude en radianes
// lon_rad: Aircraft.Longitude en radianes (puede ser 0-2π, normalizar a -π/+π)

void rad_to_nmea_lat(double lat_rad, char* buf, size_t bufsize) {
    double lat_deg = lat_rad * 180.0 / M_PI;
    char   ns      = (lat_deg >= 0) ? 'N' : 'S';
    double abslat  = fabs(lat_deg);
    int    deg     = (int)abslat;
    double minutes = (abslat - deg) * 60.0;
    snprintf(buf, bufsize, "%02d%08.5f,%c", deg, minutes, ns);
}

void rad_to_nmea_lon(double lon_rad, char* buf, size_t bufsize) {
    double lon_deg = lon_rad * 180.0 / M_PI;
    // Normalizar de 0-360 a -180/+180
    if (lon_deg > 180.0) lon_deg -= 360.0;
    char   ew      = (lon_deg >= 0) ? 'E' : 'W';
    double abslon  = fabs(lon_deg);
    int    deg     = (int)abslon;
    double minutes = (abslon - deg) * 60.0;
    snprintf(buf, bufsize, "%03d%08.5f,%c", deg, minutes, ew);
}
```

---

## 9. Hora UTC para las sentencias NMEA

NMEA requiere hora UTC en formato `HHMMSS.ss`. Usar la hora del sistema:

```cpp
void get_utc_time_str(char* buf, size_t bufsize) {
    SYSTEMTIME st;
    GetSystemTime(&st);  // UTC
    snprintf(buf, bufsize, "%02d%02d%02d.00",
             st.wHour, st.wMinute, st.wSecond);
}

void get_utc_date_str(char* buf, size_t bufsize) {
    SYSTEMTIME st;
    GetSystemTime(&st);
    snprintf(buf, bufsize, "%02d%02d%02d",
             st.wDay, st.wMonth, st.wYear % 100);
}
```

---

## 10. Rate limiting del servidor NMEA

El DLL de Aerofly se llama a 50-60 Hz. NMEA solo debe emitirse a `nmea_rate_hz`.
Usar el mismo patrón de timestamp que en `simplified/aerofly_reader_dll.cpp`:

```cpp
// En Update():
uint64_t now_us = get_time_us();
uint64_t interval_us = 1000000ULL / config.nmea_rate_hz;

if (now_us - last_broadcast_us >= interval_us) {
    last_broadcast_us = now_us;
    broadcast_nmea();
}
```

---

## 11. Referencia de archivos existentes

| Archivo | Relevancia |
|---------|-----------|
| `simplified/aerofly_reader_dll.cpp` | **Base de código** — copiar estructura completa |
| `simplified/CMakeLists.txt` | Base para el CMakeLists.txt del glider DLL |
| `simplified/build.bat` | Base para el build.bat del glider DLL |
| `simplified/README.md` | Referencia para documentar el nuevo DLL |
| `simplified/examples/simple_reader.py` | Referencia del patrón TCP de los ejemplos Python |

---

## 12. CMakeLists.txt del nuevo DLL

Basado en `simplified/CMakeLists.txt`, cambiar:
- `project(AeroflyReader)` → `project(AeroflyGlider)`
- `aerofly_reader_dll.cpp` → `aerofly_glider_dll.cpp`
- Output: `AeroflyGlider.dll`

---

## 13. Script de prueba Python (test_nmea_reader.py)

Crear en `glider/examples/` un script que se conecte al DLL y muestre las
sentencias NMEA crudas, para verificar que el formato es correcto antes de
conectar XCSoar:

```python
#!/usr/bin/env python3
"""Test: Connect to AeroflyGlider DLL and display raw NMEA sentences."""
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 4353))

print("Connected. Waiting for NMEA sentences...")
print("Press Ctrl+C to stop.")
print()

buffer = ""
try:
    while True:
        data = sock.recv(4096).decode('ascii', errors='replace')
        buffer += data
        while '\n' in buffer:
            line, buffer = buffer.split('\n', 1)
            line = line.strip()
            if line:
                print(line)
except KeyboardInterrupt:
    pass
finally:
    sock.close()
```

---

## 14. Notas importantes y trampas a evitar

1. **Latitud/Longitud en radianes:** El SDK de Aerofly entrega lat/lon en radianes.
   La longitud puede venir como 0-2π, hay que normalizar a -π/+π antes de convertir.

2. **Altitud en el simplified DLL:** El simplified DLL ya convierte `Aircraft.Altitude`
   de metros a feet al almacenarla. Para el glider DLL, mejor almacenar en metros.

3. **`Aircraft.Wind` puede ser cero:** Cuando el sim no tiene viento configurado,
   el vector será (0,0,0). Emitir igual en $LXWP0 (dirección 0, velocidad 0).

4. **Checksum NMEA:** Siempre en mayúsculas hexadecimales, dos dígitos, con `%02X`.

5. **Line ending NMEA:** Debe ser `\r\n`, no solo `\n`. Algunas apps son estrictas.

6. **Thread safety:** Los datos de vuelo se escriben desde el hilo de Aerofly y se
   leen desde el hilo del servidor TCP. Usar `std::mutex` o campos `std::atomic`
   igual que en el simplified DLL.

7. **Puerto 4353:** Es el puerto convencional para dispositivos NMEA en XCSoar,
   pero es configurable. El simplified DLL usa 12345 (JSON). El glider DLL debe
   usar un puerto diferente para que puedan correr simultáneamente.

8. **XCSoar espera reconexión:** Si XCSoar arranca antes que el DLL esté listo,
   intentará reconectarse. El servidor TCP del DLL debe aceptar conexiones entrantes
   continuamente.

9. **`emit_on_ground = false`:** Cuando el avión está en tierra, XCSoar puede
   mostrar datos extraños si recibe sentencias con velocidad 0 y variometro 0.
   La opción `emit_on_ground` permite controlar esto.

---

## 15. Resumen de conversiones de unidades

| Variable Aerofly | Unidad interna | Unidad para NMEA |
|-----------------|---------------|-----------------|
| `Aircraft.Latitude` | radianes | DDMM.MMMMM N/S |
| `Aircraft.Longitude` | radianes | DDDMM.MMMMM E/W |
| `Aircraft.Altitude` | metros (SDK) | metros ($GPGGA), feet ($PGRMZ) |
| `Aircraft.IndicatedAirspeed` | m/s | km/h ($LXWP0), knots ($GPRMC) |
| `Aircraft.GroundSpeed` | m/s | knots ($GPRMC) |
| `Aircraft.VerticalSpeed` | m/s | m/s ($LXWP0, sin convertir) |
| `Aircraft.TrueHeading` | radianes | grados 0-360 |
| `Aircraft.MagneticHeading` | radianes | grados 0-360 |
| `Aircraft.Wind` | Vector3d m/s | dirección grados + km/h magnitud |

---

*Documento creado: 2026-02-13*
*Versión: 1.0*
