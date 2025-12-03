# Monitor 6502 - Interfaz de Programación via UART

Monitor/debugger interactivo para el procesador 6502 a través de UART para Tang Nano 9K.

## Características

- ✅ Lectura/escritura de memoria
- ✅ Dump de memoria en formato hex+ASCII
- ✅ Carga de programas en hexadecimal
- ✅ Ejecución de código en cualquier dirección
- ✅ Desensamblador básico
- ✅ Fill de memoria
- ✅ Análisis de memoria RAM (scan, test, mapa visual)

## Formato de Parámetros

**Todo en HEXADECIMAL:**
- `addr` = dirección de 4 dígitos (ej: `0200`, `C001`, `8000`)
- `len` = longitud en bytes (ej: `10`=16, `100`=256, `1000`=4096)
- `val` = valor de 1 byte (ej: `FF`, `A9`, `00`)

## Comandos

### Comandos Básicos

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| **R** | `R addr` | Leer byte de memoria |
| **W** | `W addr val` | Escribir byte en memoria |
| **D** | `D addr len` | Dump de memoria (hex + ASCII) |
| **L** | `L addr` | Modo carga de bytes hex |
| **G** | `G addr` | Ejecutar código (GO/RUN) |
| **F** | `F addr len val` | Llenar memoria con valor |
| **M** | `M addr [n]` | Desensamblar n instrucciones |
| **Q** | `Q` | Salir del monitor (reinicia) |
| **H/?** | `H` | Mostrar ayuda |

### Comandos de Análisis de Memoria

| Comando | Sintaxis | Descripción |
|---------|----------|-------------|
| **I** | `I` | Información del sistema (mapa de memoria) |
| **S** | `S addr len` | Escanear memoria libre ($00 o $FF) |
| **T** | `T addr len` | Test de RAM (lectura/escritura) |
| **V** | `V` | Mapa visual de uso de RAM |

## Ejemplos de Uso

### Leer memoria
```
>R 0200
$0200 = $A9
```

### Escribir memoria
```
>W 0200 EA
$0200 <- $EA
```

### Dump de memoria
```
>D 8000 40
8000: A9 00 8D 01 C0 A9 C0 8D  03 C0 20 00 90 A2 FF CA  |..........  ....|
8010: D0 FD 8A 8D 01 C0 4C 06  80 00 00 00 00 00 00 00  |......L.........|
...
```

### Cargar programa en RAM
```
>L 0200
Modo carga en $0200 (terminar con '.')
:A9 05 8D 01 C0 60.
Cargados 0006 bytes
```

### Ejecutar código
```
>G 0200
Ejecutando en $0200...
Retorno de $0200
```

### Desensamblar
```
>M 8000
8000  A9 00     LDA# $00
8002  8D 01 C0  STAab $C001
8005  A9 C0     LDA# $C0
...
```

### Llenar memoria
```
>F 0300 100 EA
Filled $0300-$03FF con $EA
```

## Carga de Programas

El modo carga (`L addr`) permite introducir bytes en hexadecimal:

1. Ejecutar `L 0200` (dirección de inicio)
2. Escribir bytes separados por espacio: `A9 05 8D 01 C0 60`
3. Terminar con `.`

### Ejemplo: Programa que enciende LEDs

```
>L 0200
:A9 3F      ; LDA #$3F  - Cargar patrón
:8D 01 C0   ; STA $C001 - Escribir a LEDs  
:60         ; RTS       - Retornar
:.
>G 0200
```

## Integración

### En main.c

```c
#include "../libs/monitor/monitor.h"

int main(void) {
    CONF_PORT_SALIDA_LED = 0xC0;
    uart_init();
    
    // Iniciar monitor
    monitor_init();
    monitor_run();
    
    return 0;
}
```

### Makefile

Agregar la compilación del módulo monitor:

```makefile
MONITOR_DIR = $(LIB_DIR)/monitor
MONITOR_OBJ = $(BUILD_DIR)/monitor.o

$(MONITOR_OBJ): $(MONITOR_DIR)/monitor.c
    $(CC65) $(CFLAGS) -I$(UART_DIR) -o $(BUILD_DIR)/monitor.s $<
    $(CA65) -t none -o $@ $(BUILD_DIR)/monitor.s
```

## Notas Técnicas

- **Buffer**: 64 caracteres máximo por línea
- **RAM usable**: `$0200-$3DFF` (~15KB)
- **Ejecución**: El código debe terminar con `RTS` para retornar al monitor
- **Dependencia**: Requiere librería UART

## Hardware

- CPU 6502 @ 3.375 MHz
- FPGA Tang Nano 9K
- UART para comunicación serial

## Licencia

MIT
