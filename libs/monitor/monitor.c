/**
 * MONITOR.C - Monitor/Debugger para 6502 via UART
 * 
 * Implementación de interfaz de comandos estilo Wozmon/Supermon
 */

#include "monitor.h"
#include "../uart/uart.h"

/* Buffer de entrada */
static char input_buffer[MON_BUFFER_SIZE];
static uint8_t input_pos;

/* Última dirección usada (para comandos continuos) */
static uint16_t last_addr = 0x0200;

/* Tabla de caracteres hex */
static const char hex_chars[] = "0123456789ABCDEF";

/* ============================================
 * FUNCIONES DE UTILIDAD - IMPRESIÓN
 * ============================================ */

void mon_newline(void) {
    uart_putc('\r');
    uart_putc('\n');
}

void mon_print_hex8(uint8_t val) {
    uart_putc(hex_chars[(val >> 4) & 0x0F]);
    uart_putc(hex_chars[val & 0x0F]);
}

void mon_print_hex16(uint16_t val) {
    mon_print_hex8((uint8_t)(val >> 8));
    mon_print_hex8((uint8_t)(val & 0xFF));
}

static void mon_print_space(void) {
    uart_putc(' ');
}

static void mon_prompt(void) {
    mon_newline();
    uart_putc('>');
}

static void mon_error(const char *msg) {
    uart_puts("ERR: ");
    uart_puts(msg);
    mon_newline();
}

static void mon_ok(void) {
    uart_puts("OK");
    mon_newline();
}

/* ============================================
 * FUNCIONES DE CONVERSIÓN
 * ============================================ */

/**
 * Convertir carácter hex a valor
 */
static uint8_t hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0xFF; /* Error */
}

/**
 * Verificar si es carácter hex válido
 */
static uint8_t is_hex_char(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}

uint8_t mon_hex_to_u8(const char *str) {
    uint8_t result = 0;
    uint8_t i;
    
    for (i = 0; i < 2 && str[i] != '\0'; i++) {
        if (!is_hex_char(str[i])) break;
        result = (result << 4) | hex_char_to_val(str[i]);
    }
    return result;
}

uint16_t mon_hex_to_u16(const char *str) {
    uint16_t result = 0;
    uint8_t i;
    
    for (i = 0; i < 4 && str[i] != '\0'; i++) {
        if (!is_hex_char(str[i])) break;
        result = (result << 4) | hex_char_to_val(str[i]);
    }
    return result;
}

/**
 * Parsear siguiente token hex de la cadena
 * Retorna puntero al siguiente espacio o fin de cadena
 */
static const char* parse_hex_token(const char *str, uint16_t *value) {
    *value = 0;
    
    /* Saltar espacios */
    while (*str == ' ') str++;
    
    /* Parsear hex */
    while (is_hex_char(*str)) {
        *value = (*value << 4) | hex_char_to_val(*str);
        str++;
    }
    
    return str;
}

/* ============================================
 * FUNCIONES DE MEMORIA
 * ============================================ */

uint8_t mon_read_byte(uint16_t addr) {
    return *((volatile uint8_t *)addr);
}

void mon_write_byte(uint16_t addr, uint8_t value) {
    *((volatile uint8_t *)addr) = value;
}

void mon_dump(uint16_t addr, uint16_t len) {
    uint16_t i;
    uint8_t j;
    uint8_t data[16];
    uint16_t row_addr;
    
    for (i = 0; i < len; i += 16) {
        row_addr = addr + i;
        
        /* Imprimir dirección */
        mon_print_hex16(row_addr);
        uart_puts(": ");
        
        /* Leer y mostrar bytes hex */
        for (j = 0; j < 16 && (i + j) < len; j++) {
            data[j] = mon_read_byte(row_addr + j);
            mon_print_hex8(data[j]);
            mon_print_space();
        }
        
        /* Padding si línea incompleta */
        while (j < 16) {
            uart_puts("   ");
            j++;
        }
        
        /* Mostrar ASCII */
        uart_putc('|');
        for (j = 0; j < 16 && (i + j) < len; j++) {
            if (data[j] >= 0x20 && data[j] < 0x7F) {
                uart_putc(data[j]);
            } else {
                uart_putc('.');
            }
        }
        uart_putc('|');
        mon_newline();
    }
    
    last_addr = addr + len;
}

void mon_fill(uint16_t addr, uint16_t len, uint8_t value) {
    uint16_t i;
    for (i = 0; i < len; i++) {
        mon_write_byte(addr + i, value);
    }
}

/* ============================================
 * EJECUCIÓN DE CÓDIGO
 * ============================================ */

void mon_execute(uint16_t addr) {
    code_ptr code = (code_ptr)addr;
    
    uart_puts("Ejecutando en $");
    mon_print_hex16(addr);
    uart_puts("...");
    mon_newline();
    
    /* Saltar a la dirección */
    code();
    
    /* Si retorna, mostrar mensaje */
    mon_newline();
    uart_puts("Retorno de $");
    mon_print_hex16(addr);
    mon_newline();
}

/* ============================================
 * MODO CARGA DE BYTES
 * ============================================ */

/**
 * Modo carga: recibe bytes hex separados por espacio
 * Termina con '.' o línea vacía
 */
static void mon_load_mode(uint16_t addr) {
    char c;
    uint8_t byte_val;
    uint8_t nibble_count = 0;
    uint16_t bytes_loaded = 0;
    
    uart_puts("Modo carga en $");
    mon_print_hex16(addr);
    uart_puts(" (terminar con '.')");
    mon_newline();
    uart_putc(':');
    
    byte_val = 0;
    
    while (1) {
        c = uart_getc();
        
        /* Terminar con punto */
        if (c == '.') {
            break;
        }
        
        /* Enter - nueva línea de entrada */
        if (c == '\r' || c == '\n') {
            mon_newline();
            uart_putc(':');
            continue;
        }
        
        /* Espacio - separador */
        if (c == ' ') {
            uart_putc(' ');
            continue;
        }
        
        /* Procesar hex */
        if (is_hex_char(c)) {
            uart_putc(c); /* Echo */
            byte_val = (byte_val << 4) | hex_char_to_val(c);
            nibble_count++;
            
            if (nibble_count == 2) {
                /* Byte completo - escribir */
                mon_write_byte(addr, byte_val);
                addr++;
                bytes_loaded++;
                byte_val = 0;
                nibble_count = 0;
            }
        }
    }
    
    mon_newline();
    uart_puts("Cargados ");
    mon_print_hex16(bytes_loaded);
    uart_puts(" bytes");
    mon_newline();
    
    last_addr = addr;
}

/* ============================================
 * DESENSAMBLADOR BÁSICO
 * ============================================ */

/* Tabla simplificada de mnemonics (solo instrucciones comunes) */
static const char* get_mnemonic(uint8_t opcode) {
    switch (opcode) {
        case 0x00: return "BRK";
        case 0x20: return "JSR";
        case 0x40: return "RTI";
        case 0x60: return "RTS";
        case 0x4C: return "JMP";
        case 0x6C: return "JMP()";
        case 0xA9: return "LDA#";
        case 0xA5: return "LDAzp";
        case 0xAD: return "LDAab";
        case 0xA2: return "LDX#";
        case 0xA0: return "LDY#";
        case 0x85: return "STAzp";
        case 0x8D: return "STAab";
        case 0x86: return "STXzp";
        case 0x84: return "STYzp";
        case 0xE8: return "INX";
        case 0xC8: return "INY";
        case 0xCA: return "DEX";
        case 0x88: return "DEY";
        case 0x18: return "CLC";
        case 0x38: return "SEC";
        case 0xD8: return "CLD";
        case 0xF8: return "SED";
        case 0x58: return "CLI";
        case 0x78: return "SEI";
        case 0xEA: return "NOP";
        case 0xAA: return "TAX";
        case 0xA8: return "TAY";
        case 0x8A: return "TXA";
        case 0x98: return "TYA";
        case 0x9A: return "TXS";
        case 0xBA: return "TSX";
        case 0x48: return "PHA";
        case 0x68: return "PLA";
        case 0x08: return "PHP";
        case 0x28: return "PLP";
        case 0x69: return "ADC#";
        case 0xE9: return "SBC#";
        case 0xC9: return "CMP#";
        case 0xE0: return "CPX#";
        case 0xC0: return "CPY#";
        case 0x29: return "AND#";
        case 0x09: return "ORA#";
        case 0x49: return "EOR#";
        case 0xD0: return "BNE";
        case 0xF0: return "BEQ";
        case 0x10: return "BPL";
        case 0x30: return "BMI";
        case 0x90: return "BCC";
        case 0xB0: return "BCS";
        case 0x50: return "BVC";
        case 0x70: return "BVS";
        default:   return "???";
    }
}

/* Bytes por instrucción (simplificado) */
static uint8_t get_instruction_len(uint8_t opcode) {
    /* Implied/Accumulator - 1 byte */
    if (opcode == 0x00 || opcode == 0x40 || opcode == 0x60 ||
        opcode == 0xE8 || opcode == 0xC8 || opcode == 0xCA ||
        opcode == 0x88 || opcode == 0x18 || opcode == 0x38 ||
        opcode == 0xD8 || opcode == 0xF8 || opcode == 0x58 ||
        opcode == 0x78 || opcode == 0xEA || opcode == 0xAA ||
        opcode == 0xA8 || opcode == 0x8A || opcode == 0x98 ||
        opcode == 0x9A || opcode == 0xBA || opcode == 0x48 ||
        opcode == 0x68 || opcode == 0x08 || opcode == 0x28) {
        return 1;
    }
    
    /* Immediate, Zero Page, Relative - 2 bytes */
    if ((opcode & 0x0F) == 0x09 || /* Immediate */
        (opcode & 0x0F) == 0x05 || /* Zero Page */
        (opcode & 0x0F) == 0x06 || /* Zero Page */
        (opcode & 0x1F) == 0x10 || /* Branches */
        opcode == 0xA2 || opcode == 0xA0 ||
        opcode == 0xE0 || opcode == 0xC0) {
        return 2;
    }
    
    /* Absolute, Indirect - 3 bytes */
    if (opcode == 0x20 || opcode == 0x4C || opcode == 0x6C ||
        (opcode & 0x0F) == 0x0D || /* Absolute */
        (opcode & 0x0F) == 0x0E) {
        return 3;
    }
    
    /* Por defecto asumir 2 bytes */
    return 2;
}

static void mon_disassemble(uint16_t addr, uint8_t lines) {
    uint8_t i, j, len;
    uint8_t opcode;
    uint8_t bytes[3];
    
    for (i = 0; i < lines; i++) {
        opcode = mon_read_byte(addr);
        len = get_instruction_len(opcode);
        
        /* Leer bytes de la instrucción */
        for (j = 0; j < len; j++) {
            bytes[j] = mon_read_byte(addr + j);
        }
        
        /* Imprimir dirección */
        mon_print_hex16(addr);
        uart_puts("  ");
        
        /* Imprimir bytes hex */
        for (j = 0; j < 3; j++) {
            if (j < len) {
                mon_print_hex8(bytes[j]);
            } else {
                uart_puts("  ");
            }
            mon_print_space();
        }
        
        /* Imprimir mnemonic */
        uart_puts(get_mnemonic(opcode));
        
        /* Imprimir operando si hay */
        if (len == 2) {
            uart_puts(" $");
            mon_print_hex8(bytes[1]);
        } else if (len == 3) {
            uart_puts(" $");
            mon_print_hex8(bytes[2]);
            mon_print_hex8(bytes[1]);
        }
        
        mon_newline();
        addr += len;
    }
    
    last_addr = addr;
}

/* ============================================
 * ANÁLISIS DE MEMORIA RAM
 * ============================================ */

/* Constantes del mapa de memoria */
#define RAM_START       0x0100
#define RAM_END         0x3DFF
#define ZP_START        0x0002
#define ZP_END          0x00FF
#define STACK_START     0x3E00
#define STACK_END       0x3FFF
#define ROM_START       0x8000
#define ROM_END         0x9FFF
#define IO_START        0xC000
#define IO_END          0xC0FF

/**
 * Imprimir número decimal (hasta 65535)
 */
static void mon_print_dec(uint16_t val) {
    char buf[6];
    uint8_t i = 0;
    
    if (val == 0) {
        uart_putc('0');
        return;
    }
    
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    
    while (i > 0) {
        uart_putc(buf[--i]);
    }
}

/**
 * Mostrar información del sistema (mapa de memoria)
 */
static void mon_info(void) {
    mon_newline();
    uart_puts("=== MAPA DE MEMORIA ===");
    mon_newline();
    mon_newline();
    
    uart_puts("Zero Page:  $0002-$00FF (");
    mon_print_dec(ZP_END - ZP_START + 1);
    uart_puts(" bytes)");
    mon_newline();
    
    uart_puts("RAM:        $0100-$3DFF (");
    mon_print_dec(RAM_END - RAM_START + 1);
    uart_puts(" bytes)");
    mon_newline();
    
    uart_puts("Stack:      $3E00-$3FFF (");
    mon_print_dec(STACK_END - STACK_START + 1);
    uart_puts(" bytes)");
    mon_newline();
    
    uart_puts("ROM:        $8000-$9FFF (~8 KB)");
    mon_newline();
    
    uart_puts("I/O:        $C000-$C0FF");
    mon_newline();
    mon_newline();
    
    uart_puts("RAM libre para programas:");
    mon_newline();
    uart_puts("  $0200-$3DFF (");
    mon_print_dec(0x3DFF - 0x0200 + 1);
    uart_puts(" bytes)");
    mon_newline();
}

/**
 * Escanear rango de memoria y contar bytes "libres" (00 o FF)
 * Muestra estadísticas y bloques libres
 */
static void mon_scan(uint16_t start, uint16_t end) {
    uint16_t addr;
    uint16_t free_00 = 0;
    uint16_t free_ff = 0;
    uint16_t used = 0;
    uint8_t val;
    uint16_t block_start = 0;
    uint8_t in_free_block = 0;
    uint8_t blocks_shown = 0;
    
    uart_puts("Escaneando $");
    mon_print_hex16(start);
    uart_puts("-$");
    mon_print_hex16(end);
    uart_puts("...");
    mon_newline();
    
    for (addr = start; addr <= end; addr++) {
        val = mon_read_byte(addr);
        
        if (val == 0x00) {
            free_00++;
            if (!in_free_block) {
                in_free_block = 1;
                block_start = addr;
            }
        } else if (val == 0xFF) {
            free_ff++;
            if (!in_free_block) {
                in_free_block = 1;
                block_start = addr;
            }
        } else {
            used++;
            /* Fin de bloque libre */
            if (in_free_block && (addr - block_start) >= 16) {
                if (blocks_shown < 8) { /* Limitar a 8 bloques */
                    uart_puts("  Libre: $");
                    mon_print_hex16(block_start);
                    uart_puts("-$");
                    mon_print_hex16(addr - 1);
                    uart_puts(" (");
                    mon_print_dec(addr - block_start);
                    uart_puts(" bytes)");
                    mon_newline();
                    blocks_shown++;
                }
            }
            in_free_block = 0;
        }
        
        /* Evitar overflow */
        if (addr == 0xFFFF) break;
    }
    
    /* Último bloque */
    if (in_free_block && (end - block_start + 1) >= 16 && blocks_shown < 8) {
        uart_puts("  Libre: $");
        mon_print_hex16(block_start);
        uart_puts("-$");
        mon_print_hex16(end);
        uart_puts(" (");
        mon_print_dec(end - block_start + 1);
        uart_puts(" bytes)");
        mon_newline();
    }
    
    mon_newline();
    uart_puts("Resultados:");
    mon_newline();
    uart_puts("  Bytes $00: ");
    mon_print_dec(free_00);
    mon_newline();
    uart_puts("  Bytes $FF: ");
    mon_print_dec(free_ff);
    mon_newline();
    uart_puts("  Bytes usados: ");
    mon_print_dec(used);
    mon_newline();
    uart_puts("  Total libre: ");
    mon_print_dec(free_00 + free_ff);
    uart_puts(" / ");
    mon_print_dec(end - start + 1);
    mon_newline();
}

/**
 * Prueba de RAM: escribir y leer para verificar que funciona
 * Usa un test simple: escribe valor, lee, verifica
 */
static void mon_test_ram(uint16_t start, uint16_t len) {
    uint16_t i;
    uint16_t addr;
    uint8_t original;
    uint8_t test_val;
    uint8_t read_val;
    uint16_t errors = 0;
    uint16_t ok = 0;
    
    uart_puts("Test RAM $");
    mon_print_hex16(start);
    uart_puts("-$");
    mon_print_hex16(start + len - 1);
    mon_newline();
    
    for (i = 0; i < len; i++) {
        addr = start + i;
        
        /* Guardar valor original */
        original = mon_read_byte(addr);
        
        /* Test 1: escribir $55 */
        test_val = 0x55;
        mon_write_byte(addr, test_val);
        read_val = mon_read_byte(addr);
        
        if (read_val != test_val) {
            errors++;
            if (errors <= 5) {
                uart_puts("  $");
                mon_print_hex16(addr);
                uart_puts(" W:");
                mon_print_hex8(test_val);
                uart_puts(" R:");
                mon_print_hex8(read_val);
                mon_newline();
            }
        } else {
            /* Test 2: escribir $AA (complemento) */
            test_val = 0xAA;
            mon_write_byte(addr, test_val);
            read_val = mon_read_byte(addr);
            
            if (read_val != test_val) {
                errors++;
                if (errors <= 5) {
                    uart_puts("  $");
                    mon_print_hex16(addr);
                    uart_puts(" W:");
                    mon_print_hex8(test_val);
                    uart_puts(" R:");
                    mon_print_hex8(read_val);
                    mon_newline();
                }
            } else {
                ok++;
            }
        }
        
        /* Restaurar valor original */
        mon_write_byte(addr, original);
    }
    
    mon_newline();
    if (errors == 0) {
        uart_puts("OK: ");
        mon_print_dec(ok);
        uart_puts(" bytes");
    } else {
        uart_puts("FAIL: ");
        mon_print_dec(errors);
        uart_puts("/");
        mon_print_dec(len);
    }
    mon_newline();
}

/**
 * Vista rápida de uso de memoria (mapa visual)
 */
static void mon_memmap(void) {
    uint16_t page;
    uint16_t addr;
    uint8_t used_count;
    uint8_t i;
    char symbol;
    
    uart_puts("Mapa de RAM (. = libre, # = usada, X = mixta)");
    mon_newline();
    uart_puts("Cada caracter = 256 bytes (1 pagina)");
    mon_newline();
    mon_newline();
    
    uart_puts("     0123456789ABCDEF");
    mon_newline();
    
    /* Páginas de RAM: $01-$3D */
    for (page = 0x01; page <= 0x3D; page++) {
        if ((page & 0x0F) == 0x01) {
            uart_puts("$");
            mon_print_hex8((uint8_t)page);
            uart_puts(": ");
        }
        
        /* Contar bytes usados en la página */
        used_count = 0;
        for (i = 0; i < 64; i++) { /* Muestrear cada 4 bytes */
            addr = (page << 8) | (i << 2);
            if (mon_read_byte(addr) != 0x00 && mon_read_byte(addr) != 0xFF) {
                used_count++;
            }
        }
        
        /* Determinar símbolo */
        if (used_count == 0) {
            symbol = '.';
        } else if (used_count >= 60) {
            symbol = '#';
        } else {
            symbol = 'X';
        }
        
        uart_putc(symbol);
        
        if ((page & 0x0F) == 0x00 || page == 0x3D) {
            mon_newline();
        }
    }
    
    mon_newline();
    uart_puts("ZP=$02-$FF  Stack=$3E-$3F");
    mon_newline();
}

/* ============================================
 * AYUDA
 * ============================================ */

static void mon_help(void) {
    mon_newline();
    uart_puts("=== MONITOR 6502 ===");
    mon_newline();
    uart_puts("Todo en HEX (addr=4dig)");
    mon_newline();
    uart_puts("--- BASICOS ---");
    mon_newline();
    uart_puts("R addr      | Leer byte");
    mon_newline();
    uart_puts("W addr val  | Escribir byte");
    mon_newline();
    uart_puts("D addr len  | Dump memoria");
    mon_newline();
    uart_puts("L addr      | Cargar hex (fin=.)");
    mon_newline();
    uart_puts("G addr      | Ejecutar codigo");
    mon_newline();
    uart_puts("F addr ln v | Fill memoria");
    mon_newline();
    uart_puts("M addr [n]  | Desensamblar");
    mon_newline();
    uart_puts("--- MEMORIA ---");
    mon_newline();
    uart_puts("I           | Info mapa mem");
    mon_newline();
    uart_puts("S addr len  | Scan mem libre");
    mon_newline();
    uart_puts("T addr len  | Test RAM");
    mon_newline();
    uart_puts("V           | Vista RAM");
    mon_newline();
    uart_puts("--- OTROS ---");
    mon_newline();
    uart_puts("H/?         | Ayuda");
    mon_newline();
    uart_puts("Q           | Salir");
    mon_newline();
    uart_puts("Ej: D 8000 40  F 0200 100 EA");
    mon_newline();
    uart_puts("RAM libre: $0200-$3DFF");
    mon_newline();
}

/* ============================================
 * PROCESAMIENTO DE COMANDOS
 * ============================================ */

uint8_t monitor_process_cmd(char *cmd) {
    char command;
    const char *ptr;
    uint16_t addr, len, val;
    
    /* Saltar espacios iniciales */
    while (*cmd == ' ') cmd++;
    
    /* Comando vacío */
    if (*cmd == '\0') return MON_OK;
    
    /* Obtener comando (primer carácter) */
    command = *cmd;
    if (command >= 'a' && command <= 'z') {
        command -= 32; /* Convertir a mayúscula */
    }
    
    ptr = cmd + 1;
    
    switch (command) {
        case 'R': /* Read byte */
            ptr = parse_hex_token(ptr, &addr);
            if (addr == 0 && ptr == cmd + 1) {
                addr = last_addr;
            }
            uart_putc('$');
            mon_print_hex16(addr);
            uart_puts(" = $");
            mon_print_hex8(mon_read_byte(addr));
            mon_newline();
            last_addr = addr + 1;
            break;
            
        case 'W': /* Write byte */
            ptr = parse_hex_token(ptr, &addr);
            ptr = parse_hex_token(ptr, &val);
            mon_write_byte(addr, (uint8_t)val);
            uart_putc('$');
            mon_print_hex16(addr);
            uart_puts(" <- $");
            mon_print_hex8((uint8_t)val);
            mon_newline();
            last_addr = addr + 1;
            break;
            
        case 'D': /* Dump */
            ptr = parse_hex_token(ptr, &addr);
            ptr = parse_hex_token(ptr, &len);
            if (len == 0) len = 64; /* Default 64 bytes */
            mon_dump(addr, len);
            break;
            
        case 'L': /* Load mode */
            ptr = parse_hex_token(ptr, &addr);
            if (addr == 0) addr = last_addr;
            mon_load_mode(addr);
            break;
            
        case 'G': /* Go/Execute */
            ptr = parse_hex_token(ptr, &addr);
            mon_execute(addr);
            break;
            
        case 'F': /* Fill */
            ptr = parse_hex_token(ptr, &addr);
            ptr = parse_hex_token(ptr, &len);
            ptr = parse_hex_token(ptr, &val);
            mon_fill(addr, len, (uint8_t)val);
            uart_puts("Filled $");
            mon_print_hex16(addr);
            uart_puts("-$");
            mon_print_hex16(addr + len - 1);
            uart_puts(" con $");
            mon_print_hex8((uint8_t)val);
            mon_newline();
            break;
            
        case 'M': /* Memory/Disassemble */
            ptr = parse_hex_token(ptr, &addr);
            if (addr == 0) addr = last_addr;
            ptr = parse_hex_token(ptr, &len);
            if (len == 0) len = 16;
            mon_disassemble(addr, (uint8_t)len);
            break;
            
        case 'I': /* Info - Mapa de memoria */
            mon_info();
            break;
            
        case 'S': /* Scan - Buscar memoria libre */
            ptr = parse_hex_token(ptr, &addr);
            ptr = parse_hex_token(ptr, &len);
            if (addr == 0) addr = 0x0200;  /* Default: inicio RAM usuario */
            if (len == 0) len = 0x3BFF;    /* Default: toda la RAM */
            if (addr + len > 0x3DFF) len = 0x3DFF - addr + 1;
            mon_scan(addr, addr + len - 1);
            break;
            
        case 'T': /* Test RAM */
            ptr = parse_hex_token(ptr, &addr);
            ptr = parse_hex_token(ptr, &len);
            if (addr == 0) addr = 0x0200;
            if (len == 0) len = 0x100;  /* Default: 256 bytes */
            mon_test_ram(addr, len);
            break;
            
        case 'V': /* Vista mapa de memoria */
            mon_memmap();
            break;
            
        case 'Q': /* Quit */
            uart_puts("Saliendo del monitor...");
            mon_newline();
            return MON_EXIT;
            
        case 'H':
        case '?':
            mon_help();
            break;
            
        default:
            mon_error("Comando desconocido. H=ayuda");
            break;
    }
    
    return MON_OK;
}

/* ============================================
 * ENTRADA DE LÍNEA
 * ============================================ */

static void mon_read_line(void) {
    char c;
    input_pos = 0;
    
    while (1) {
        c = uart_getc();
        
        /* Enter - fin de línea */
        if (c == '\r' || c == '\n') {
            input_buffer[input_pos] = '\0';
            mon_newline();
            return;
        }
        
        /* Backspace */
        if (c == 0x08 || c == 0x7F) {
            if (input_pos > 0) {
                input_pos--;
                uart_putc(0x08); /* Cursor atrás */
                uart_putc(' ');  /* Borrar carácter */
                uart_putc(0x08); /* Cursor atrás */
            }
            continue;
        }
        
        /* Escape - cancelar línea */
        if (c == 0x1B) {
            input_pos = 0;
            input_buffer[0] = '\0';
            uart_puts(" [ESC]");
            mon_newline();
            return;
        }
        
        /* Carácter normal */
        if (input_pos < MON_BUFFER_SIZE - 1 && c >= 0x20 && c < 0x7F) {
            input_buffer[input_pos++] = c;
            uart_putc(c); /* Echo */
        }
    }
}

/* ============================================
 * FUNCIONES PRINCIPALES
 * ============================================ */

void monitor_init(void) {
    input_pos = 0;
    last_addr = 0x0200;
}

void monitor_run(void) {
    uint8_t result;
    
    mon_newline();
    uart_puts("================================");
    mon_newline();
    uart_puts("  MONITOR 6502 v1.0");
    mon_newline();
    uart_puts("  Tang Nano 9K @ 3.375 MHz");
    mon_newline();
    uart_puts("================================");
    mon_newline();
    uart_puts("Escribe H para ayuda");
    
    while (1) {
        mon_prompt();
        mon_read_line();
        
        result = monitor_process_cmd(input_buffer);
        
        if (result == MON_EXIT) {
            break;
        }
    }
}
