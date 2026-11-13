"""Comprueba pixeles, padding, orientacion BMP, errores y resultados del benchmark."""
import csv
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parent.parent
PROGRAM = ROOT / "build" / ("transformaciones_bench.exe" if os.name == "nt" else "transformaciones_bench")


def escribir_bmp(path, filas, top_down=False, dib=40, final=b"TRAILER"):
    alto, ancho = len(filas), len(filas[0])
    padding = bytes((-ancho * 3) % 4)
    datos = bytearray()
    for fila in filas if top_down else filas[::-1]:
        for r, g, b in fila:
            datos.extend((b, g, r))
        datos.extend(padding)
    offset = 14 + dib
    cabecera = struct.pack("<2sIHHI", b"BM", offset + len(datos) + len(final), 0, 0, offset)
    cabecera += struct.pack("<IiiHHIIiiII", dib, ancho, -alto if top_down else alto,
                           1, 24, 0, len(datos), 2835, 2835, 0, 0)
    cabecera += bytes(dib - 40)
    path.write_bytes(cabecera + datos + final)
    return cabecera


def leer_bmp(path):
    datos = path.read_bytes()
    ancho, altura = struct.unpack_from("<ii", datos, 18)
    offset = struct.unpack_from("<I", datos, 10)[0]
    stride = (ancho * 3 + 3) & ~3
    filas = []
    for y in range(abs(altura)):
        fila = []
        for x in range(ancho):
            b, g, r = datos[offset + y * stride + x * 3:offset + y * stride + x * 3 + 3]
            fila.append((r, g, b))
        filas.append(fila)
    return filas if altura < 0 else filas[::-1]


def grises(filas):
    return [[(int(.21 * r + .72 * g + .07 * b),) * 3 for r, g, b in fila] for fila in filas]


class TransformacionesTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not PROGRAM.is_file():
            raise RuntimeError("Compila antes de ejecutar las pruebas.")
        cls.compilacion = tempfile.TemporaryDirectory()
        cls.probe = Path(cls.compilacion.name) / ("probe.exe" if os.name == "nt" else "probe")
        compiler = shlex.split(os.environ.get("CC", "cc"))
        subprocess.run(compiler + ["-std=c17", "-O1", "-Wall", "-Wextra", "-I", str(ROOT),
                                  str(ROOT / "tests/library_probe.c"), "-o", str(cls.probe)], check=True)

    @classmethod
    def tearDownClass(cls):
        cls.compilacion.cleanup()

    def setUp(self):
        self.temporal = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporal.cleanup)
        self.root = Path(self.temporal.name)
        self.entrada = self.root / "input"
        self.entrada.mkdir()
        self.salida = self.root / "output"
        self.pixeles = [[(23, 80, 130), (190, 31, 62), (91, 40, 219)],
                        [(17, 212, 35), (72, 98, 121), (142, 53, 86)]]

    def ejecutar(self, threads=1):
        env = os.environ.copy()
        env.update(OMP_NUM_THREADS=str(threads), OMP_DYNAMIC="FALSE")
        return subprocess.run([str(PROGRAM), str(self.entrada), str(self.salida), "1"],
                              env=env, capture_output=True, text=True, timeout=30)

    def test_four_operations_padding_orientation_and_extended_header(self):
        for top_down in (False, True):
            for dib in (40, 124):
                for threads in (1, 3):
                    with self.subTest(top_down=top_down, dib=dib, threads=threads):
                        cabecera = escribir_bmp(self.entrada / "img0.bmp", self.pixeles, top_down, dib)
                        result = self.ejecutar(threads)
                        self.assertEqual(result.returncode, 0, result.stderr)
                        esperados = {
                            "HorizontalGray": grises([fila[::-1] for fila in self.pixeles]),
                            "HorizontalRGB": [fila[::-1] for fila in self.pixeles],
                            "Rotate180Gray": grises([fila[::-1] for fila in self.pixeles[::-1]]),
                            "Rotate180RGB": [fila[::-1] for fila in self.pixeles[::-1]],
                        }
                        for nombre, esperado in esperados.items():
                            archivo = self.salida / f"Test0_{nombre}.bmp"
                            self.assertEqual(leer_bmp(archivo), esperado)
                            self.assertTrue(archivo.read_bytes().startswith(cabecera))
                            self.assertTrue(archivo.read_bytes().endswith(b"TRAILER"))

    def test_missing_invalid_and_truncated_inputs_fail(self):
        result = self.ejecutar()
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(self.salida.exists())
        archivo = self.entrada / "img0.bmp"
        escribir_bmp(archivo, self.pixeles)
        original = archivo.read_bytes()
        variantes = [original[:55], b"ZZ" + original[2:]]
        for offset, formato, valor in [(28, "H", 8), (30, "I", 1), (18, "i", -3), (22, "i", 0), (22, "i", -2147483648), (10, "I", 20)]:
            modificado = bytearray(original)
            struct.pack_into("<" + formato, modificado, offset, valor)
            variantes.append(modificado)
        for contenido in variantes:
            archivo.write_bytes(contenido)
            self.assertNotEqual(self.ejecutar().returncode, 0)
            self.assertFalse(self.salida.exists())

    def test_output_errors_fail(self):
        escribir_bmp(self.entrada / "img0.bmp", self.pixeles)
        self.salida.write_text("no es una carpeta")
        self.assertNotEqual(self.ejecutar().returncode, 0)
        self.salida.unlink()
        self.salida.mkdir()
        (self.salida / "Test0_HorizontalRGB.bmp").mkdir()
        self.assertNotEqual(self.ejecutar().returncode, 0)

    def test_gray_partial_and_histograms(self):
        imagen = self.entrada / "img0.bmp"
        escribir_bmp(imagen, self.pixeles, top_down=True)
        destino = self.root / "extra.bmp"
        for operacion in ("gray", "partial", "histogram", "emptyhist"):
            result = subprocess.run([str(self.probe), operacion, str(imagen), str(destino)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            if operacion == "gray":
                self.assertEqual(leer_bmp(destino), grises(self.pixeles))
            if operacion == "partial":
                esperado = [[tuple((b - a) % 256 for a, b in zip(fila[0], fila[1])), (0, 0, 0), fila[2]] for fila in self.pixeles]
                self.assertEqual(leer_bmp(destino), esperado)
            if operacion == "histogram":
                self.assertEqual(result.stdout.strip(), "6 6 6")

    def test_blur_preserves_borders_and_averages_center(self):
        filas = [[(0, 0, 0) for _ in range(11)] for _ in range(11)]
        filas[5][5] = (255, 255, 255)
        imagen = self.entrada / "img0.bmp"
        escribir_bmp(imagen, filas)
        esperado = [[(3, 3, 3) if 4 <= x <= 6 and 4 <= y <= 6 else (0, 0, 0) for x in range(11)] for y in range(11)]
        for operacion in ("blur", "blurgray"):
            destino = self.root / (operacion + ".bmp")
            subprocess.run([str(self.probe), operacion, str(imagen), str(destino)], check=True)
            self.assertEqual(leer_bmp(destino), esperado)

    def test_benchmark_records_actual_threads_and_reproducible_inputs(self):
        imagen = self.entrada / "img0.bmp"
        escribir_bmp(imagen, self.pixeles)
        resultados = self.root / "results"
        command = [sys.executable, str(ROOT / "benchmark_transformaciones.py"), "--threads", "1", "3",
                   "--repetitions", "2", "--warmups", "1", "--count", "1", "--input-dir", str(self.entrada),
                   "--output-dir", str(self.salida), "--results-dir", str(resultados)]
        result = subprocess.run(command, capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        with next(resultados.glob("runs_*.csv")).open() as f:
            filas = list(csv.DictReader(f))
        self.assertEqual(len(filas), 4)
        self.assertTrue(all(float(f["wall_seconds"]) >= float(f["c_seconds"]) > 0 for f in filas))
        with next(resultados.glob("summary_*.csv")).open() as f:
            resumenes = list(csv.DictReader(f))
        self.assertEqual(len(resumenes), 2)
        self.assertEqual(float(resumenes[0]["speedup_vs_1"]), 1.0)
        meta = json.loads(next(resultados.glob("metadata_*.json")).read_text())
        self.assertEqual(meta["inputs"][0]["sha256"], hashlib.sha256(imagen.read_bytes()).hexdigest())
        self.assertEqual(meta["OMP_DYNAMIC"], "FALSE")


if __name__ == "__main__":
    unittest.main()
