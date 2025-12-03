# Makefile para 6502 I2C EPROM Test
# Compila main.c con todas las librerías

# ============================================
# DIRECTORIOS
# ============================================
SRC_DIR = src
LIB_DIR = libs
BUILD_DIR = build
OUTPUT_DIR = output
CONFIG_DIR = config
SCRIPTS_DIR = scripts

# ============================================
# HERRAMIENTAS
# ============================================
CC65 = cc65
CA65 = ca65
LD65 = ld65
CL65 = cl65
PYTHON = py

# ============================================
# CONFIGURACIÓN
# ============================================
CONFIG = $(CONFIG_DIR)/fpga.cfg
PLATAFORMA = D:\cc65\lib\none.lib
CFLAGS = -t none -O --cpu 6502

# ============================================
# LIBRERÍAS
# ============================================

UART_DIR = $(LIB_DIR)/uart
MONITOR_DIR = $(LIB_DIR)/monitor

INCLUDES = -I$(UART_DIR) -I$(MONITOR_DIR)

# ============================================
# ARCHIVOS OBJETO
# ============================================
MAIN_OBJ = $(BUILD_DIR)/main.o
UART_OBJ = $(BUILD_DIR)/uart.o
MONITOR_OBJ = $(BUILD_DIR)/monitor.o
VECTORS_OBJ = $(BUILD_DIR)/simple_vectors.o

OBJS = $(MAIN_OBJ) $(UART_OBJ) $(MONITOR_OBJ) $(VECTORS_OBJ)

# ============================================
# TARGET PRINCIPAL
# ============================================
TARGET = $(BUILD_DIR)/main.bin

# Regla por defecto
all: dirs $(TARGET) rom
	@echo ========================================
	@echo COMPILADO EXITOSAMENTE
	@echo ========================================
	@echo VHDL: $(OUTPUT_DIR)/rom.vhd
	@echo ========================================

# Crear directorios
dirs:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(OUTPUT_DIR)" mkdir "$(OUTPUT_DIR)"

# ============================================
# COMPILACIÓN DE OBJETOS
# ============================================

# Main
$(MAIN_OBJ): $(SRC_DIR)/main.c
	$(CL65) -t none $(INCLUDES) -c -o $@ $<

# UART
$(UART_OBJ): $(UART_DIR)/uart.c
	$(CC65) $(CFLAGS) -o $(BUILD_DIR)/uart.s $<
	$(CA65) -t none -o $@ $(BUILD_DIR)/uart.s

# Monitor
$(MONITOR_OBJ): $(MONITOR_DIR)/monitor.c
	$(CC65) $(CFLAGS) -I$(UART_DIR) -o $(BUILD_DIR)/monitor.s $<
	$(CA65) -t none -o $@ $(BUILD_DIR)/monitor.s

# Vectores
$(VECTORS_OBJ): $(SRC_DIR)/simple_vectors.s
	$(CA65) -t none -o $@ $<

# ============================================
# ENLAZADO
# ============================================
$(TARGET): $(OBJS)
	$(LD65) -C $(CONFIG) --start-addr 0x8000 -o $@ $(OBJS) $(PLATAFORMA)

# ============================================
# GENERACIÓN DE ROM
# ============================================
rom: $(TARGET)
	$(PYTHON) $(SCRIPTS_DIR)/bin2rom3.py $(TARGET) -s 8192 --name rom --data-width 8 -o $(OUTPUT_DIR)

# ============================================
# LIMPIEZA
# ============================================
clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@if exist "$(OUTPUT_DIR)\*.vhd" del /q "$(OUTPUT_DIR)\*.vhd"
	@if exist "$(OUTPUT_DIR)\*.bin" del /q "$(OUTPUT_DIR)\*.bin"
	@if exist "$(OUTPUT_DIR)\*.hex" del /q "$(OUTPUT_DIR)\*.hex"

# ============================================
# AYUDA
# ============================================
help:
	@echo ========================================
	@echo Comandos
	@echo ========================================
	@echo   make        - Compilar y generar ROM
	@echo   make clean  - Limpiar archivos
	@echo   make help   - Mostrar esta ayuda
	@echo ========================================

.PHONY: all dirs rom clean help