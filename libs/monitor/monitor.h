/**
 * MONITOR.H - Monitor/Debugger para 6502 via UART
 * 
 * Interfaz de comandos para programación y debug del 6502
 * 
 * Comandos disponibles:
 *   R [addr]        - Leer byte de memoria
 *   W addr byte     - Escribir byte en memoria
 *   D addr len      - Dump de memoria (hex)
 *   L addr          - Cargar bytes en memoria (modo carga)
 *   G addr          - Ejecutar código en dirección (GO/RUN)
 *   F addr len val  - Fill: llenar memoria con valor
 *   M addr          - Ver memoria como desensamblado básico
 *   H               - Ayuda
 *   ?               - Ayuda
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>

/* Tamaño máximo del buffer de entrada */
#define MON_BUFFER_SIZE  64

/* Códigos de retorno */
#define MON_OK           0
#define MON_ERROR        1
#define MON_EXIT         2

/* Estructura para pasar código a ejecutar */
typedef void (*code_ptr)(void);

/* ============================================
 * FUNCIONES PRINCIPALES
 * ============================================ */

/**
 * Inicializar el monitor
 */
void monitor_init(void);

/**
 * Ejecutar el monitor (bucle principal)
 * Retorna cuando el usuario escribe 'Q' o error fatal
 */
void monitor_run(void);

/**
 * Procesar un solo comando
 * @param cmd Cadena con el comando
 * @return MON_OK, MON_ERROR, o MON_EXIT
 */
uint8_t monitor_process_cmd(char *cmd);

/* ============================================
 * FUNCIONES DE MEMORIA
 * ============================================ */

/**
 * Leer un byte de memoria
 */
uint8_t mon_read_byte(uint16_t addr);

/**
 * Escribir un byte en memoria
 */
void mon_write_byte(uint16_t addr, uint8_t value);

/**
 * Dump de memoria en formato hex
 */
void mon_dump(uint16_t addr, uint16_t len);

/**
 * Llenar memoria con un valor
 */
void mon_fill(uint16_t addr, uint16_t len, uint8_t value);

/* ============================================
 * FUNCIONES DE UTILIDAD
 * ============================================ */

/**
 * Convertir cadena hex a uint16
 */
uint16_t mon_hex_to_u16(const char *str);

/**
 * Convertir cadena hex a uint8
 */
uint8_t mon_hex_to_u8(const char *str);

/**
 * Imprimir byte en hexadecimal
 */
void mon_print_hex8(uint8_t val);

/**
 * Imprimir word en hexadecimal
 */
void mon_print_hex16(uint16_t val);

/**
 * Imprimir nueva línea
 */
void mon_newline(void);

/**
 * Saltar a una dirección y ejecutar
 */
void mon_execute(uint16_t addr);

#endif /* MONITOR_H */
