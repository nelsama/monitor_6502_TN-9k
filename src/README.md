# Código Fuente - 6502 LED Light Show

## Archivos

| Archivo | Descripción |
|---------|-------------|
| `main.c` | **Programa principal** - Juego de luces LED |
| `simple_vectors.s` | Vectores de interrupción para 6502 |

## main.c - Juego de Luces LED

Programa que controla 6 LEDs con efectos visuales.

### Registros de Hardware

| Registro | Dirección | Descripción |
|----------|-----------|-------------|
| `PORT_SALIDA_LED` | $C001 | Datos de salida (bits 0-5 = LEDs) |
| `CONF_PORT_SALIDA_LED` | $C003 | Configuración: 0=salida, 1=entrada |

### Efectos Implementados

1. **Knight Rider**: LED que recorre ida y vuelta
2. **Llenado**: LEDs se llenan desde extremos al centro
3. **Alternado**: Parpadeo alternado (010101 ↔ 101010)
4. **Contador**: Contador binario de 0 a 63

### Configuración Inicial

```c
CONF_PORT_SALIDA_LED = 0xC0;  // bits 7,6=entrada, bits 5-0=salida
PORT_SALIDA_LED = 0x00;       // LEDs apagados
```

### Compilación

```bash
make
```

## simple_vectors.s

Define los vectores de interrupción del 6502:

| Vector | Dirección | Función |
|--------|-----------|----------|
| NMI | $9FFA | Retorno inmediato (RTI) |
| RESET | $9FFC | Apunta a $8000 (inicio ROM) |
| IRQ | $9FFE | Retorno inmediato (RTI) |

## Hardware Requerido

- 6502 CPU @ 3.375 MHz en FPGA Tang Nano
- 6 LEDs conectados a bits 0-5 de $C001
- Resistencias limitadoras para LEDs
