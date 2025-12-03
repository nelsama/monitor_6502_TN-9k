#!/usr/bin/env python3
"""
Generador de ROM para FPGA con bus de direcciones y datos
Basado en bin2rom.py
"""

import argparse
from pathlib import Path
from datetime import datetime

def format_vhdl_data(data, data_width, offset):
    """Formatea los datos para inicialización VHDL con un offset inicial"""
    elements = []
    bytes_per_word = data_width // 8


    # Formatear los datos reales
    for i in range(0, len(data), bytes_per_word):
        word = data[i:i+bytes_per_word]
        if len(word) < bytes_per_word:
            word += b'\x00' * (bytes_per_word - len(word))  # Relleno con ceros
        hex_addr_str = f"{i:04X}"
        hex_data_str = ''.join(f"{b:02X}" for b in word)  # Big-endian
        tmp_str = f"when x\"{hex_addr_str}\" => data_out<= x\"{hex_data_str}\""
        elements.append(tmp_str)

    return ";\n        ".join(elements) + ";"

def generate_vhdl_rom(data, full_data_len,rom_name="rom", data_width=8, addr_width=None, offset=0):
    """
    Genera un módulo VHDL completo con:
    - Bus de direcciones
    - Bus de datos de salida
    - Reloj para acceso síncrono
    """
    if addr_width is None:
        addr_width = (full_data_len + offset - 1).bit_length()
    addr_dif = 16 - addr_width;
    str_complete = "0" * addr_dif;
    vhdl_template = f"""-- ======================================================
-- ROM generada automáticamente con Python
-- Fecha: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
-- Tamaño: {full_data_len} bytes
-- Ancho de datos: {data_width} bits
-- Ancho de dirección: {addr_width} bits
-- Offset inicial: {offset}
-- ======================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

ENTITY {rom_name} IS
    port (
    clk      : in  std_logic;
    address  : in  std_logic_vector({addr_width-1} downto 0);
    data_out : out std_logic_vector({data_width-1} downto 0)
    );
END entity;

architecture rtl of {rom_name} is
BEGIN

	PROCESS(clk)

    variable addr : std_logic_vector(15 downto 0);

	BEGIN
    if rising_edge(clk) then
        addr:="{str_complete}"&address;
        case addr is

            {format_vhdl_data(data, data_width, offset)}
            when others => data_out<= x"FF";

        end case;
    end if;
	END PROCESS;
end architecture;
"""
    return vhdl_template

def generate_intel_hex(data, offset):
    """Genera un archivo Intel HEX con un offset inicial"""
    hex_lines = []
    address = offset

    for i in range(0, len(data), 16):  # Procesar en bloques de 16 bytes
        chunk = data[i:i+16]
        byte_count = len(chunk)
        address_high = (address >> 8) & 0xFF
        address_low = address & 0xFF
        record_type = 0x00  # Data record
        checksum = byte_count + address_high + address_low + record_type + sum(chunk)
        checksum = (-checksum) & 0xFF

        # Formatear línea Intel HEX
        hex_line = f":{byte_count:02X}{address_high:02X}{address_low:02X}{record_type:02X}" + \
                   ''.join(f"{b:02X}" for b in chunk) + \
                   f"{checksum:02X}"
        hex_lines.append(hex_line)
        address += 16

    # Agregar línea de fin de archivo
    hex_lines.append(":00000001FF")
    return "\n".join(hex_lines)

def bin_to_rom(input_file, output_dir, rom_size, rom_name="rom", data_width=8, addr_width=None, fill_byte=0xFF, offset=0):
    """Convierte binario a múltiples formatos"""
    # Leer y ajustar datos
    
    input_data = Path(input_file).read_bytes()
    data_len = len(input_data)

    if data_len > rom_size:
        raise ValueError(f"Archivo muy grande ({data_len} > {rom_size} bytes)")

    # Crear ROM con padding
    padded_data = bytearray(input_data + bytes([fill_byte] * (rom_size - data_len)))
    
    # Si el binario tiene vectores 6502 al final (últimos 6 bytes), copiarlos al final de la ROM
    if data_len >= 6:
        # Los últimos 6 bytes del binario original son los vectores
        vectors = input_data[-6:]
        # Copiarlos a las últimas 6 posiciones de la ROM completa
        padded_data[-6:] = vectors
        print(f"  Vectores 6502 copiados a posiciones {rom_size-6}-{rom_size-1}")

    # Crear directorio
    output_dir = Path(output_dir)
    output_dir.mkdir(exist_ok=True)

    # Generar archivo VHDL
    vhdl_path = output_dir / f"{rom_name}.vhd"
    vhdl_content = generate_vhdl_rom(padded_data, len(padded_data),rom_name=rom_name, data_width=data_width, addr_width=addr_width, offset=0)
    vhdl_path.write_text(vhdl_content)
    print(f"Generado: {vhdl_path}")

    # Generar archivo binario
    bin_path = output_dir / f"{rom_name}.bin"
    bin_path.write_bytes(padded_data)
    print(f"Generado: {bin_path}")

    # Generar archivo Intel HEX
    hex_path = output_dir / f"{rom_name}.hex"
    hex_content = generate_intel_hex(padded_data, offset)
    hex_path.write_text(hex_content)
    print(f"Generado: {hex_path}")

def parse_int(value):
    """Convierte un valor de cadena a entero, aceptando tanto decimal como hexadecimal."""
    try:
        return int(value, 0)  # Base 0 permite interpretar valores como 0xNNNN (hex) o NNNN (dec)
    except ValueError:
        raise argparse.ArgumentTypeError(f"Valor inválido: '{value}'. Debe ser un entero decimal o hexadecimal.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Generador de ROM para FPGA con bus VHDL e Intel HEX',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('input', help='Archivo binario de entrada')
    parser.add_argument('-o', '--output-dir', default='output', help='Directorio de salida')
    parser.add_argument('-s', '--size', type=parse_int, required=True, help='Tamaño total de la ROM en bytes')
    parser.add_argument('--name', default='rom', help='Nombre de la entidad VHDL')
    parser.add_argument('--data-width', type=int, choices=[8, 16, 32], default=8,
                        help='Ancho del bus de datos en bits')
    parser.add_argument('--addr-width', type=int,
                        help='Ancho del bus de direcciones en bits (auto-calculado si no se especifica)')
    parser.add_argument('--fill', type=parse_int, default=0xFF,
                        help='Byte de relleno (ej: 0x00, 255)')
    parser.add_argument('--offset', type=parse_int, default=0,
                        help='Offset inicial en la salida Intel HEX (en bytes)')

    args = parser.parse_args()

    try:
        bin_to_rom(
            args.input,
            args.output_dir,
            args.size,
            rom_name=args.name,
            data_width=args.data_width,
            addr_width=args.addr_width,
            fill_byte=args.fill,
            offset=args.offset
        )
    except Exception as e:
        print(f"❌ Error: {e}")
        exit(1)