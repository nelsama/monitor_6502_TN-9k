# 🔧 Scripts de Desarrollo

Scripts utilitarios para el proyecto Micro6502.

## 📄 bin2rom3.py

### Generador de ROM para FPGA con múltiples formatos de salida

### 🚀 Uso Rápido

```bash
# Generar ROM de 8KB a partir de main.bin
python bin2rom3.py build/main.bin -s 8192

# ROM personalizada con offset
python bin2rom3.py build/main.bin -s 0x2000 --offset 0x8000 --name program_rom
```

### 📋 Características

- ✅ **Múltiples formatos**: VHDL, Binario, Intel HEX
- ✅ **Bus configurable**: 8/16/32 bits de datos
- ✅ **Offset flexible**: Inicio en cualquier dirección
- ✅ **Relleno automático**: Completa ROM con byte configurable
- ✅ **FPGA ready**: Código VHDL optimizado con reloj

### 🎯 Parámetros

| Parámetro | Descripción | Ejemplo |
|-----------|-------------|---------|
| `input` | Archivo binario entrada | `build/main.bin` |
| `-s, --size` | Tamaño total ROM (bytes) | `8192` o `0x2000` |
| `-o, --output-dir` | Directorio salida | `output/` |
| `--name` | Nombre entidad VHDL | `program_rom` |
| `--data-width` | Ancho bus datos (8/16/32) | `8` |
| `--addr-width` | Ancho bus direcciones | Auto-calculado |
| `--fill` | Byte de relleno | `0xFF` |
| `--offset` | Offset inicial Intel HEX | `0x8000` |

### 📤 Archivos Generados

```text
output/
├── rom.vhd         # Entidad VHDL con bus síncrono
├── rom.bin         # Archivo binario rellenado
└── rom.hex         # Intel HEX con offset
```

### 💻 Ejemplos de Uso

#### ROM Básica 8KB

```bash
python bin2rom3.py build/main.bin -s 8192
```

#### ROM para Boot Loader (32KB, offset 0x8000)

```bash
python bin2rom3.py build/bootloader.bin -s 32768 --offset 0x8000 --name boot_rom
```

#### ROM de Datos 16-bit

```bash
python bin2rom3.py data/lookup.bin -s 4096 --data-width 16 --name data_rom
```

### 🔧 Código VHDL Generado

```vhdl
ENTITY rom IS
    port (
    clk      : in  std_logic;
    address  : in  std_logic_vector(12 downto 0);  -- Auto-calculado
    data_out : out std_logic_vector(7 downto 0)
    );
END entity;
```

### ⚡ Optimizaciones

- **Acceso síncrono**: Usa `rising_edge(clk)` para FPGA
- **Bus expandible**: Hasta 32 bits de datos
- **Relleno inteligente**: Completa automáticamente con 0xFF
- **Direcciones hexadecimales**: Soporte para 0x notation

---

Parte del proyecto **Micro6502** - Sistema 6502 en FPGA