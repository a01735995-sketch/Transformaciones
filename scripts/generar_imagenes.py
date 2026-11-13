"""Genera BMP sinteticos de 24 bits sin dependencias externas."""
import argparse
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parent.parent


def crear_bmp(destino, ancho, alto, indice=0, top_down=False):
    relleno = b"\x00" * ((-ancho * 3) % 4)
    bytes_imagen = (ancho * 3 + len(relleno)) * alto
    cabecera = struct.pack("<2sIHHI", b"BM", 54 + bytes_imagen, 0, 0, 54)
    cabecera += struct.pack("<IiiHHIIiiII", 40, ancho, -alto if top_down else alto,
                           1, 24, 0, bytes_imagen, 2835, 2835, 0, 0)
    with destino.open("wb") as stream:
        stream.write(cabecera)
        filas = range(alto) if top_down else range(alto - 1, -1, -1)
        for y in filas:
            fila = bytearray()
            for x in range(ancho):
                r = (x * 3 + indice * 17) % 256
                g = (y * 5 + indice * 31) % 256
                b = (x + y + indice * 47) % 256
                fila.extend((b, g, r))
            stream.write(fila)
            stream.write(relleno)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--count", type=int, default=10)
    parser.add_argument("--width", type=int, default=640)
    parser.add_argument("--height", type=int, default=480)
    parser.add_argument("--output-dir", type=Path, default=ROOT / "images/input")
    parser.add_argument("--top-down", action="store_true", help="almacenar filas desde arriba (altura BMP negativa)")
    parser.add_argument("--force", action="store_true", help="sobrescribir imagenes con los mismos nombres")
    args = parser.parse_args()
    if not 1 <= args.count <= 10000 or args.width < 1 or args.height < 1 or args.width * args.height > 2_147_483_647:
        parser.error("Se requiere 1 <= count <= 10000, dimensiones positivas y width * height <= INT_MAX.")
    args.output_dir.mkdir(parents=True, exist_ok=True)
    archivos = [args.output_dir / f"img{i}.bmp" for i in range(args.count)]
    if not args.force and any(p.exists() for p in archivos):
        parser.error("Ya hay imagenes con esos nombres. Elige otra carpeta o usa --force.")
    for i, archivo in enumerate(archivos):
        crear_bmp(archivo, args.width, args.height, i, args.top_down)
    print(f"Generadas {args.count} imagenes de {args.width} x {args.height} en {args.output_dir.resolve()}")


if __name__ == "__main__":
    main()
