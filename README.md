# 6502 Template - Tang Nano 9K

🚀 **Proyecto base/template** para desarrollo en CPU 6502 sobre FPGA Tang Nano 9K.

Usa este proyecto como punto de partida para crear tus propias aplicaciones con el procesador 6502.

## Características

- ✅ CPU 6502 @ 3.375 MHz en FPGA Tang Nano 9K
- ✅ Control de 6 LEDs 
- ✅ Comunicación UART para debug
- ✅ Compilación con cc65
- ✅ Estructura lista para expandir

## Hardware Soportado

| Componente | Dirección | Descripción |
|------------|-----------|-------------|
| PORT_SALIDA_LED | $C001 | Puerto de salida para 6 LEDs (bits 0-5) |
| CONF_PORT_SALIDA_LED | $C003 | Configuración: 0=salida, 1=entrada |
| UART | - | Comunicación serial para debug |

## Estructura del Proyecto

```
├── src/
│   ├── main.c              # Programa principal (edita aquí tu código)
│   └── simple_vectors.s    # Vectores de interrupción 6502
├── libs/                   # Librerías externas (UART, I2C, etc.)
├── config/
│   └── fpga.cfg            # Configuración del linker cc65
├── scripts/
│   └── bin2rom3.py         # Conversor BIN → VHDL
├── build/                  # Archivos compilados (generado)
├── output/                 # ROM generada (generado)
└── makefile                # Compilación con cc65
```

## Cómo Usar este Template

1. **Clona o descarga** este repositorio
2. **Edita** `src/main.c` con tu código
3. **Agrega librerías** en la carpeta `libs/` según necesites
4. **Compila** con `make`
5. **Carga** `output/rom.vhd` en tu proyecto FPGA

## Compilación

### Requisitos previos
- [cc65](https://cc65.github.io/) instalado en `D:\cc65`
- Python 3 para el script de conversión

### Compilar
```bash
make
```

### Limpiar
```bash
make clean
```

### Cargar en FPGA
Copiar `output/rom.vhd` al proyecto FPGA y sintetizar.

## Ejemplo Incluido

El `main.c` incluye un ejemplo básico que:
- Inicializa el puerto de LEDs
- Inicializa la UART
- Alterna el encendido/apagado de LEDs
- Envía mensajes por UART para debug

```c
while (1) {
    encendido(10000);   // Enciende LEDs + mensaje UART
    apagado(10000);     // Apaga LEDs + mensaje UART
}
```

## Mapa de Memoria

| Región | Dirección | Tamaño | Descripción |
|--------|-----------|--------|-------------|
| Zero Page | $0002-$00FF | 254 bytes | Variables rápidas |
| RAM | $0100-$3DFF | ~15 KB | RAM principal |
| Stack | $3E00-$3FFF | 512 bytes | Pila del sistema |
| ROM | $8000-$9FF9 | 8 KB | Código del programa |
| Vectores | $9FFA-$9FFF | 6 bytes | NMI, RESET, IRQ |
| I/O | $C000-$C003 | 4 bytes | Puertos de E/S |

## Requisitos

- [cc65](https://cc65.github.io/) - Compilador C para 6502
- Python 3 - Para el script bin2rom3.py
- FPGA Tang Nano 9K

## Licencia

MIT
