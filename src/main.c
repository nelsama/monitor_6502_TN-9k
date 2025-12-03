/**
 * main.c - Monitor/Debugger 6502 via UART
 * 
 * Hardware:
 *   - 6502 CPU @ 3.375 MHz en FPGA Tang Nano
 *   - 6 LEDs conectados a los bits 0-5 de PORT_SALIDA_LED ($C001)
 *   - UART para comunicación serial
 * 
 * Este programa inicia un monitor interactivo que permite:
 *   - Leer/escribir memoria
 *   - Cargar programas en hexadecimal
 *   - Ejecutar código desde cualquier dirección
 *   - Desensamblar código
 * 
 * Compilar: make
 */

#include <stdint.h>
#include "../libs/uart/uart.h"
#include "../libs/monitor/monitor.h"

/* Registro de salida de LEDs (6 bits inferiores) */
#define PORT_SALIDA_LED      (*(volatile uint8_t*)0xC001)

/* Registro de configuración: 0=salida, 1=entrada */
#define CONF_PORT_SALIDA_LED (*(volatile uint8_t*)0xC003)

/* ============================================================================
 * PROGRAMA PRINCIPAL
 * ============================================================================ */

int main(void) {
    
    /* Configurar los 6 bits inferiores como salidas (0 = salida) */
    CONF_PORT_SALIDA_LED = 0xC0;  /* bits 7,6 = entrada, bits 5-0 = salida */
    
    /* Apagar todos los LEDs inicialmente */
    PORT_SALIDA_LED = 0x00;
    
    /* Inicializar UART */
    uart_init();
    
    /* Mensaje de bienvenida */
    uart_puts("\r\n");
    uart_puts("##### 6502 SYSTEM READY #####\r\n");
    uart_puts("Iniciando Monitor...\r\n");
    
    /* Iniciar y ejecutar el monitor en bucle */
    monitor_init();
    
    while (1) {
        monitor_run();
        uart_puts("Reiniciando monitor...\r\n");
    }
    
    return 0;
}
