#!/usr/bin/env python3
"""
generate_font_bitmaps.py - Generate extended character bitmaps for EPDDisplay fonts.

Usage (TTF mode - high quality):
    python3 generate_font_bitmaps.py --font /path/to/font.ttf --output ../src/fonts/font_ext.cpp

Usage (scale mode - generates from built-in hand-crafted Font12 data):
    python3 generate_font_bitmaps.py --scale --output ../src/fonts/font_ext.cpp

The scale mode is the default when no --font is specified.
"""

import argparse
import sys
import os

# ──────────────────────────────────────────────────────────────────────────────
# Font geometry (must match EPDDisplay font files exactly)
# ──────────────────────────────────────────────────────────────────────────────
FONTS = [
    {"name": "Font8",  "width": 5,  "height": 8},
    {"name": "Font12", "width": 7,  "height": 12},
    {"name": "Font16", "width": 11, "height": 16},
    {"name": "Font20", "width": 14, "height": 20},
    {"name": "Font24", "width": 17, "height": 24},
]

# ──────────────────────────────────────────────────────────────────────────────
# Extended character set (45 chars, MUST be sorted by codepoint)
# ──────────────────────────────────────────────────────────────────────────────
EXT_CHARS = [
    (0x00A1, '¡'),
    (0x00B0, '°'),
    (0x00B1, '±'),
    (0x00BF, '¿'),
    (0x00C0, 'À'),
    (0x00C1, 'Á'),
    (0x00C2, 'Â'),
    (0x00C4, 'Ä'),
    (0x00C7, 'Ç'),
    (0x00C8, 'È'),
    (0x00C9, 'É'),
    (0x00CA, 'Ê'),
    (0x00CB, 'Ë'),
    (0x00CD, 'Í'),
    (0x00CE, 'Î'),
    (0x00CF, 'Ï'),
    (0x00D1, 'Ñ'),
    (0x00D3, 'Ó'),
    (0x00D6, 'Ö'),
    (0x00D9, 'Ù'),
    (0x00DA, 'Ú'),
    (0x00DB, 'Û'),
    (0x00DC, 'Ü'),
    (0x00DF, 'ß'),
    (0x00E0, 'à'),
    (0x00E1, 'á'),
    (0x00E2, 'â'),
    (0x00E4, 'ä'),
    (0x00E7, 'ç'),
    (0x00E8, 'è'),
    (0x00E9, 'é'),
    (0x00EA, 'ê'),
    (0x00EB, 'ë'),
    (0x00ED, 'í'),
    (0x00EE, 'î'),
    (0x00EF, 'ï'),
    (0x00F1, 'ñ'),
    (0x00F3, 'ó'),
    (0x00F6, 'ö'),
    (0x00F9, 'ù'),
    (0x00FA, 'ú'),
    (0x00FB, 'û'),
    (0x00FC, 'ü'),
    (0x00FF, 'ÿ'),
    (0x20AC, '€'),
]

assert len(EXT_CHARS) == 45, f"Expected 45 chars, got {len(EXT_CHARS)}"
# Verify sorted
for i in range(len(EXT_CHARS) - 1):
    assert EXT_CHARS[i][0] < EXT_CHARS[i+1][0], f"Not sorted at index {i}"

# ──────────────────────────────────────────────────────────────────────────────
# Hand-crafted Font12 bitmaps (7×12, verified against existing font tables)
# Each entry is exactly 12 bytes.
# Encoding: bit7=col0, bit6=col1, ..., bit1=col6, bit0=unused
# ──────────────────────────────────────────────────────────────────────────────
FONT12_BITMAPS = {
    0x00A1: [0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00],  # ¡
    0x00B0: [0x18, 0x24, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],  # °
    0x00B1: [0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00],  # ±
    0x00BF: [0x00, 0x10, 0x00, 0x10, 0x10, 0x20, 0x40, 0x44, 0x44, 0x38, 0x00, 0x00],  # ¿
    # Uppercase accented
    0x00C0: [0x10, 0x08, 0x30, 0x10, 0x28, 0x28, 0x28, 0x7C, 0x44, 0xEE, 0x00, 0x00],  # À
    0x00C1: [0x08, 0x10, 0x30, 0x10, 0x28, 0x28, 0x28, 0x7C, 0x44, 0xEE, 0x00, 0x00],  # Á
    0x00C2: [0x10, 0x28, 0x30, 0x10, 0x28, 0x28, 0x28, 0x7C, 0x44, 0xEE, 0x00, 0x00],  # Â
    0x00C4: [0x28, 0x00, 0x30, 0x10, 0x28, 0x28, 0x28, 0x7C, 0x44, 0xEE, 0x00, 0x00],  # Ä
    0x00C7: [0x00, 0x3C, 0x44, 0x40, 0x40, 0x40, 0x40, 0x44, 0x38, 0x10, 0x20, 0x00],  # Ç
    0x00C8: [0x10, 0x08, 0xFC, 0x44, 0x50, 0x70, 0x50, 0x40, 0x44, 0xFC, 0x00, 0x00],  # È
    0x00C9: [0x08, 0x10, 0xFC, 0x44, 0x50, 0x70, 0x50, 0x40, 0x44, 0xFC, 0x00, 0x00],  # É
    0x00CA: [0x10, 0x28, 0xFC, 0x44, 0x50, 0x70, 0x50, 0x40, 0x44, 0xFC, 0x00, 0x00],  # Ê
    0x00CB: [0x28, 0x00, 0xFC, 0x44, 0x50, 0x70, 0x50, 0x40, 0x44, 0xFC, 0x00, 0x00],  # Ë
    0x00CD: [0x08, 0x10, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00],  # Í
    0x00CE: [0x10, 0x28, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00],  # Î
    0x00CF: [0x28, 0x00, 0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00],  # Ï
    0x00D1: [0x24, 0x18, 0xEE, 0x64, 0x64, 0x54, 0x4C, 0x4C, 0x44, 0xEE, 0x00, 0x00],  # Ñ
    0x00D3: [0x08, 0x10, 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00],  # Ó
    0x00D6: [0x28, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00],  # Ö
    0x00D9: [0x10, 0x08, 0xEE, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00],  # Ù
    0x00DA: [0x08, 0x10, 0xEE, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00],  # Ú
    0x00DB: [0x10, 0x28, 0xEE, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00],  # Û
    0x00DC: [0x28, 0x00, 0xEE, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00],  # Ü
    0x00DF: [0x00, 0x38, 0x4C, 0x44, 0x78, 0x44, 0x44, 0x4C, 0x40, 0x00, 0x00, 0x00],  # ß
    # Lowercase accented
    0x00E0: [0x10, 0x08, 0x00, 0x38, 0x44, 0x3C, 0x44, 0x44, 0x3E, 0x00, 0x00, 0x00],  # à
    0x00E1: [0x08, 0x10, 0x00, 0x38, 0x44, 0x3C, 0x44, 0x44, 0x3E, 0x00, 0x00, 0x00],  # á
    0x00E2: [0x10, 0x28, 0x00, 0x38, 0x44, 0x3C, 0x44, 0x44, 0x3E, 0x00, 0x00, 0x00],  # â
    0x00E4: [0x28, 0x00, 0x00, 0x38, 0x44, 0x3C, 0x44, 0x44, 0x3E, 0x00, 0x00, 0x00],  # ä
    0x00E7: [0x00, 0x00, 0x00, 0x3C, 0x44, 0x40, 0x40, 0x44, 0x38, 0x10, 0x20, 0x00],  # ç
    0x00E8: [0x10, 0x08, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x40, 0x3C, 0x00, 0x00, 0x00],  # è
    0x00E9: [0x08, 0x10, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x40, 0x3C, 0x00, 0x00, 0x00],  # é
    0x00EA: [0x10, 0x28, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x40, 0x3C, 0x00, 0x00, 0x00],  # ê
    0x00EB: [0x28, 0x00, 0x00, 0x38, 0x44, 0x7C, 0x40, 0x40, 0x3C, 0x00, 0x00, 0x00],  # ë
    0x00ED: [0x08, 0x10, 0x00, 0x70, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00],  # í
    0x00EE: [0x10, 0x28, 0x00, 0x70, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00],  # î
    0x00EF: [0x28, 0x00, 0x00, 0x70, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00, 0x00],  # ï
    0x00F1: [0x24, 0x18, 0x00, 0xD8, 0x64, 0x44, 0x44, 0x44, 0xEE, 0x00, 0x00, 0x00],  # ñ
    0x00F3: [0x08, 0x10, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00],  # ó
    0x00F6: [0x28, 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00],  # ö
    0x00F9: [0x10, 0x08, 0x00, 0xCC, 0x44, 0x44, 0x44, 0x4C, 0x36, 0x00, 0x00, 0x00],  # ù
    0x00FA: [0x08, 0x10, 0x00, 0xCC, 0x44, 0x44, 0x44, 0x4C, 0x36, 0x00, 0x00, 0x00],  # ú
    0x00FB: [0x10, 0x28, 0x00, 0xCC, 0x44, 0x44, 0x44, 0x4C, 0x36, 0x00, 0x00, 0x00],  # û
    0x00FC: [0x28, 0x00, 0x00, 0xCC, 0x44, 0x44, 0x44, 0x4C, 0x36, 0x00, 0x00, 0x00],  # ü
    0x00FF: [0x28, 0x00, 0x00, 0xEE, 0x44, 0x24, 0x28, 0x18, 0x10, 0x10, 0x78, 0x00],  # ÿ
    0x20AC: [0x00, 0x38, 0x44, 0x40, 0x7C, 0x40, 0x7C, 0x44, 0x38, 0x00, 0x00, 0x00],  # €
}

# Verify Font12 bitmaps
for cp, char in EXT_CHARS:
    bmp = FONT12_BITMAPS[cp]
    assert len(bmp) == 12, f"U+{cp:04X} ({char}) Font12 has {len(bmp)} bytes (expected 12)"

def get_bit_font12(bitmap, col, row):
    """Get a single pixel from a Font12 7×12 bitmap.
    Encoding: bit7=col0, ..., bit1=col6 of each row byte."""
    if row < 0 or row >= 12 or col < 0 or col >= 7:
        return 0
    byte = bitmap[row]
    return 1 if (byte & (0x80 >> col)) else 0

def scale_bitmap(src_bitmap, src_w, src_h, dst_w, dst_h):
    """Scale a bitmap using nearest-neighbor sampling.
    Returns a 2D list [row][col] of 0/1 values."""
    result = []
    for dst_row in range(dst_h):
        row_data = []
        src_row = int(dst_row * src_h / dst_h + 0.5)
        src_row = min(src_row, src_h - 1)
        for dst_col in range(dst_w):
            src_col = int(dst_col * src_w / dst_w + 0.5)
            src_col = min(src_col, src_w - 1)
            # Read from source bitmap (Font12 format)
            byte = src_bitmap[src_row]
            bit = 1 if (byte & (0x80 >> src_col)) else 0
            row_data.append(bit)
        result.append(row_data)
    return result

def bitmap_to_bytes(pixels, width, height):
    """Convert 2D pixel array to byte array using MSB-first packing.
    Returns list of bytes. For each row: ceil(width/8) bytes."""
    bytes_per_row = (width + 7) // 8
    result = []
    for row in range(height):
        for byte_idx in range(bytes_per_row):
            byte_val = 0
            for bit_idx in range(8):
                col = byte_idx * 8 + bit_idx
                if col < width and row < len(pixels) and col < len(pixels[row]):
                    if pixels[row][col]:
                        byte_val |= (0x80 >> bit_idx)
            result.append(byte_val)
    return result

def font12_to_pixels(bitmap):
    """Convert Font12 byte array to 2D pixel array."""
    pixels = []
    for row in range(12):
        row_data = []
        for col in range(7):
            bit = 1 if (bitmap[row] & (0x80 >> col)) else 0
            row_data.append(bit)
        pixels.append(row_data)
    return pixels

def generate_scale_mode():
    """Generate all font bitmaps by scaling from Font12 reference data."""
    all_tables = {}  # font_name -> list of bytes for all 45 chars

    for font in FONTS:
        fname = font["name"]
        fw = font["width"]
        fh = font["height"]
        bytes_per_char = ((fw + 7) // 8) * fh
        table_bytes = []

        for cp, char in EXT_CHARS:
            src_bitmap = FONT12_BITMAPS[cp]
            if fw == 7 and fh == 12:
                # Use directly
                char_bytes = list(src_bitmap)
            else:
                # Scale from Font12
                src_pixels = font12_to_pixels(src_bitmap)
                dst_pixels = scale_bitmap(src_bitmap, 7, 12, fw, fh)
                char_bytes = bitmap_to_bytes(dst_pixels, fw, fh)

            assert len(char_bytes) == bytes_per_char, \
                f"{fname} U+{cp:04X}: expected {bytes_per_char} bytes, got {len(char_bytes)}"
            table_bytes.extend(char_bytes)

        expected_total = bytes_per_char * 45
        assert len(table_bytes) == expected_total, \
            f"{fname} total: expected {expected_total}, got {len(table_bytes)}"
        all_tables[fname] = (table_bytes, bytes_per_char)

    return all_tables

def generate_ttf_mode(font_path):
    """Generate font bitmaps from a TTF font file using Pillow."""
    try:
        from PIL import Image, ImageDraw, ImageFont
    except ImportError:
        print("Error: Pillow not installed. Run: pip install Pillow", file=sys.stderr)
        sys.exit(1)

    all_tables = {}

    for font in FONTS:
        fname = font["name"]
        fw = font["width"]
        fh = font["height"]
        bytes_per_char = ((fw + 7) // 8) * fh
        table_bytes = []

        # Load font at appropriate size
        try:
            pil_font = ImageFont.truetype(font_path, fh - 2)
        except Exception as e:
            print(f"Warning: could not load {font_path} at size {fh}: {e}", file=sys.stderr)
            pil_font = ImageFont.load_default()

        for cp, char in EXT_CHARS:
            # Render character
            img = Image.new('L', (fw * 3, fh * 2), 0)
            draw = ImageDraw.Draw(img)
            draw.text((2, 1), char, font=pil_font, fill=255)

            # Find bounding box and crop to fw×fh
            pixels_2d = []
            for row in range(fh):
                row_data = []
                for col in range(fw):
                    px = img.getpixel((col + 2, row + 1))
                    row_data.append(1 if px > 128 else 0)
                pixels_2d.append(row_data)

            char_bytes = bitmap_to_bytes(pixels_2d, fw, fh)
            assert len(char_bytes) == bytes_per_char
            table_bytes.extend(char_bytes)

        all_tables[fname] = (table_bytes, bytes_per_char)

    return all_tables

def format_c_array(name, data, bytes_per_char, chars):
    """Format data as a C array with one char per section."""
    lines = [f"const uint8_t {name}[] = {{"]
    offset = 0
    for i, (cp, char) in enumerate(chars):
        char_bytes = data[offset:offset + bytes_per_char]
        hex_vals = ", ".join(f"0x{b:02X}" for b in char_bytes)
        lines.append(f"    // [{i:2d}] U+{cp:04X} '{char}'")
        lines.append(f"    {hex_vals},")
        offset += bytes_per_char
    lines.append("};")
    lines.append("")
    return "\n".join(lines)

def generate_cpp(all_tables, output_path):
    """Write the complete font_ext.cpp file."""
    lines = []
    lines.append("// font_ext.cpp - Extended character bitmaps for EPDDisplay")
    lines.append("// AUTO-GENERATED by tools/generate_font_bitmaps.py")
    lines.append("// DO NOT EDIT manually - regenerate with the Python tool for changes.")
    lines.append("")
    lines.append('#include "../EPDDisplay.h"')
    lines.append('#include "font_ext.h"')
    lines.append("")
    lines.append("// ──────────────────────────────────────────────────────────────────────────")
    lines.append(f"// Sorted Unicode codepoints ({len(EXT_CHARS)} extended characters)")
    lines.append("// ──────────────────────────────────────────────────────────────────────────")
    lines.append(f"const uint32_t fontExtCodepoints[{len(EXT_CHARS)}] = {{")
    cp_vals = ", ".join(f"0x{cp:04X}U" for cp, _ in EXT_CHARS)
    lines.append(f"    {cp_vals}")
    lines.append("};")
    lines.append("")

    for font in FONTS:
        fname = font["name"]
        fw = font["width"]
        fh = font["height"]
        bpr = (fw + 7) // 8
        bpc = bpr * fh
        total = bpc * 45
        table_data, _ = all_tables[fname]
        varname = f"fontExt{fname[4:]}_Table"  # e.g. fontExt8_Table

        lines.append(f"// ──────────────────────────────────────────────────────────────────────────")
        lines.append(f"// {fname}: {fw}×{fh} pixels, {bpr} byte(s)/row, {bpc} bytes/char, {total} bytes total")
        lines.append(f"// ──────────────────────────────────────────────────────────────────────────")
        lines.append(format_c_array(varname, table_data, bpc, EXT_CHARS))

    lines.append("// ──────────────────────────────────────────────────────────────────────────")
    lines.append("// Binary search lookup: returns index [0..44] or -1 if not found")
    lines.append("// ──────────────────────────────────────────────────────────────────────────")
    lines.append("int16_t fontExtLookup(uint32_t codepoint) {")
    lines.append(f"    int16_t lo = 0, hi = {len(EXT_CHARS) - 1};")
    lines.append("    while (lo <= hi) {")
    lines.append("        int16_t mid = (lo + hi) >> 1;")
    lines.append("        if (fontExtCodepoints[mid] == codepoint) return mid;")
    lines.append("        if (fontExtCodepoints[mid] < codepoint) lo = mid + 1;")
    lines.append("        else hi = mid - 1;")
    lines.append("    }")
    lines.append("    return -1;")
    lines.append("}")
    lines.append("")

    content = "\n".join(lines)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"Written: {output_path}")

    # Print statistics
    print(f"\nMemory usage summary:")
    total_bytes = 0
    for font in FONTS:
        fname = font["name"]
        fw, fh = font["width"], font["height"]
        bpc = ((fw + 7) // 8) * fh
        sz = bpc * 45
        total_bytes += sz
        print(f"  fontExt{fname[4:]}_Table: {sz:5d} bytes  ({fw}×{fh}, {bpc} bytes/char)")
    print(f"  Codepoint table:        {45*4:5d} bytes")
    total_bytes += 45 * 4
    print(f"  ─────────────────────────────────")
    print(f"  TOTAL Flash usage:      {total_bytes:5d} bytes  (~{total_bytes//1024}.{(total_bytes%1024)*10//1024} KB)")


def main():
    parser = argparse.ArgumentParser(description='Generate EPDDisplay extended font bitmaps')
    parser.add_argument('--font', metavar='TTF_PATH',
                        help='Path to TTF font file (high quality mode)')
    parser.add_argument('--scale', action='store_true',
                        help='Use built-in Font12 data and scale (default if no --font)')
    parser.add_argument('--output', metavar='PATH',
                        default=os.path.join(os.path.dirname(__file__),
                                             '..', 'src', 'fonts', 'font_ext.cpp'),
                        help='Output .cpp file path')
    args = parser.parse_args()

    output_path = os.path.abspath(args.output)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    if args.font:
        print(f"TTF mode: {args.font}")
        all_tables = generate_ttf_mode(args.font)
    else:
        print("Scale mode: scaling from built-in Font12 reference data")
        all_tables = generate_scale_mode()

    generate_cpp(all_tables, output_path)


if __name__ == '__main__':
    main()
