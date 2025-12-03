# Monitor 6502 - Tang Nano 9K

🚀 **Monitor/Debugger interactivo** para CPU 6502 sobre FPGA Tang Nano 9K via UART.

Permite programar, depurar y ejecutar código en tiempo real a través de una interfaz de comandos estilo Wozmon.

## Características

- ✅ CPU 6502 @ 3.375 MHz en FPGA Tang Nano 9K
- ✅ Monitor interactivo via UART
- ✅ Lectura/escritura de memoria
- ✅ Carga de programas en hexadecimal
- ✅ Ejecución de código en cualquier dirección
- ✅ Desensamblador básico
- ✅ Análisis de memoria RAM (scan, test, mapa visual)
- ✅ Control de 6 LEDs
- ✅ Compilación con cc65

## Comandos del Monitor

Todo en **HEXADECIMAL** (addr=4 dígitos)

### Básicos
| Comando | Descripción |
|---------|-------------|
| `R addr` | Leer byte de memoria |
| `W addr val` | Escribir byte |
| `D addr len` | Dump memoria (hex+ASCII) |
| `L addr` | Cargar bytes hex (terminar con `.`) |
| `G addr` | Ejecutar código (GO) |
| `F addr len val` | Llenar memoria |
| `M addr [n]` | Desensamblar |

### Análisis de Memoria
| Comando | Descripción |
|---------|-------------|
| `I` | Info mapa de memoria |
| `S addr len` | Escanear memoria libre |
| `T addr len` | Test de RAM |
| `V` | Vista visual de RAM |

### Otros
| Comando | Descripción |
|---------|-------------|
| `H` / `?` | Ayuda |
| `Q` | Reiniciar monitor |

## Hardware Soportado

| Componente | Dirección | Descripción |
|------------|-----------|-------------|
| LEDs | $C001 | Puerto de salida para 6 LEDs (bits 0-5) |
| LED Config | $C003 | Configuración: 0=salida, 1=entrada |
| UART Data | $C020 | TX/RX datos |
| UART Status | $C021 | Estado (TX_READY, RX_VALID) |

## Estructura del Proyecto

```
├── src/
│   ├── main.c              # Programa principal
│   └── simple_vectors.s    # Vectores de interrupción 6502
├── libs/
│   ├── monitor/            # Monitor interactivo (incluido)
│   └── uart/               # Librería UART (repo separado)
├── config/
│   └── fpga.cfg            # Configuración del linker cc65
├── scripts/
│   └── bin2rom3.py         # Conversor BIN → VHDL
├── build/                  # Archivos compilados (generado)
├── output/                 # ROM generada (generado)
└── makefile                # Compilación con cc65
```

## Instalación

### Requisitos
- [cc65](https://cc65.github.io/) instalado en `D:\cc65`
- Python 3 para el script de conversión
- Librería UART en `libs/uart/` (clonar de repo separado)

### Compilar
```bash
make
```

### Cargar en FPGA
Copiar `output/rom.vhd` al proyecto FPGA y sintetizar.

## Ejemplo de Uso

```
##### 6502 SYSTEM READY #####
Iniciando Monitor...

================================
  MONITOR 6502 v1.0
  Tang Nano 9K @ 3.375 MHz
================================
Escribe H para ayuda

>D 8000 20
8000: A9 C0 8D 03 C0 A9 00 8D  01 C0 20 ...

>L 0200
:A9 3F 8D 01 C0 60.
Cargados 0005 bytes

>G 0200
Ejecutando en $0200...
Retorno de $0200
```

## Mapa de Memoria

| Región | Dirección | Tamaño | Descripción |
|--------|-----------|--------|-------------|
| Zero Page | $0002-$00FF | 254 bytes | Variables rápidas |
| RAM | $0100-$3DFF | ~15 KB | RAM principal |
| Stack | $3E00-$3FFF | 512 bytes | Pila del sistema |
| ROM | $8000-$9FF9 | 8 KB | Código del programa |
| Vectores | $9FFA-$9FFF | 6 bytes | NMI, RESET, IRQ |
| I/O | $C000-$C0FF | 256 bytes | Puertos de E/S |

**RAM libre para programas:** `$0200-$3DFF` (~15 KB)

## Dependencias

- [cc65](https://cc65.github.io/) - Compilador C para 6502
- Python 3 - Para bin2rom3.py
- Librería UART (repo separado)
- FPGA Tang Nano 9K

## Licencia

MIT
