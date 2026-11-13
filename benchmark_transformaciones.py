"""Mide ejecuciones completas del programa C y guarda tiempos y contexto."""
import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import statistics
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parent


def positivo(valor):
    numero = int(valor)
    if numero < 1:
        raise argparse.ArgumentTypeError("debe ser un entero positivo")
    return numero


def no_negativo(valor):
    numero = int(valor)
    if numero < 0:
        raise argparse.ArgumentTypeError("debe ser un entero mayor o igual a cero")
    return numero


def huella(archivo):
    with archivo.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def ejecutar(programa, entrada, salida, cantidad, hilos, timeout):
    entorno = os.environ.copy()
    entorno["OMP_NUM_THREADS"] = str(hilos)
    entorno["OMP_DYNAMIC"] = "FALSE"
    inicio = time.perf_counter()
    resultado = subprocess.run(
        [str(programa), str(entrada), str(salida), str(cantidad)],
        cwd=ROOT, env=entorno, capture_output=True, text=True,
        encoding="utf-8", errors="replace", timeout=timeout,
    )
    total = time.perf_counter() - inicio
    if resultado.returncode:
        raise RuntimeError(
            f"El programa C termino con codigo {resultado.returncode}:\n"
            f"{resultado.stderr.strip() or resultado.stdout.strip()}"
        )
    interno = re.search(r"^Total time: ([0-9.]+) seconds$", resultado.stdout, re.M)
    reales = re.search(r"^OpenMP threads: ([0-9]+)$", resultado.stdout, re.M)
    if not interno or not reales:
        raise RuntimeError("El ejecutable no devolvio los tiempos e hilos esperados. Recompila el programa.")
    return total, float(interno.group(1)), int(reales.group(1))


def guardar_csv(destino, filas):
    with destino.open("w", newline="", encoding="utf-8") as stream:
        escritor = csv.DictWriter(stream, fieldnames=list(filas[0]))
        escritor.writeheader()
        escritor.writerows(filas)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--threads", nargs="+", type=positivo, default=[1, 5, 10], help="hilos por grupo (predeterminado: 1 5 10)")
    parser.add_argument("--repetitions", type=positivo, default=10, help="mediciones por grupo")
    parser.add_argument("--warmups", type=no_negativo, default=1, help="ejecuciones previas por grupo, fuera de las estadisticas")
    parser.add_argument("--count", type=positivo, default=10, help="cantidad de imagenes img0.bmp, img1.bmp, ...")
    parser.add_argument("--input-dir", type=Path, default=ROOT / "images/input")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "output")
    parser.add_argument("--results-dir", type=Path, default=ROOT / "results")
    parser.add_argument("--program", type=Path, default=ROOT / "build" / ("transformaciones_bench.exe" if os.name == "nt" else "transformaciones_bench"))
    parser.add_argument("--timeout", type=positivo, default=600, help="limite de segundos por ejecucion")
    args = parser.parse_args(argv)
    if len(set(args.threads)) != len(args.threads):
        parser.error("--threads no puede contener valores repetidos")
    if args.count > 10000:
        parser.error("--count no puede superar 10000")
    programa = args.program.resolve()
    entrada, salida, resultados = (p.resolve() for p in (args.input_dir, args.output_dir, args.results_dir))
    if not programa.is_file():
        parser.error(f"No existe el ejecutable {programa}. Compila primero con make o GCC.")
    entradas = [entrada / f"img{i}.bmp" for i in range(args.count)]
    for archivo in entradas:
        if not archivo.is_file():
            parser.error(f"No existe {archivo}. Copia tus BMP o usa scripts/generar_imagenes.py.")
    salida.mkdir(parents=True, exist_ok=True)
    resultados.mkdir(parents=True, exist_ok=True)
    metadata = {
        "started_at_utc": datetime.now(timezone.utc).isoformat(),
        "platform": platform.platform(), "machine": platform.machine(),
        "processor": platform.processor(), "logical_cpus": os.cpu_count(),
        "python": platform.python_version(),
        "program": str(programa), "program_sha256": huella(programa),
        "header_sha256": huella(ROOT / "Transformaciones/Transformaciones.h"),
        "input_dir": str(entrada), "output_dir": str(salida),
        "count": args.count, "threads_requested": args.threads,
        "repetitions": args.repetitions, "warmups_per_group": args.warmups,
        "timeout_seconds": args.timeout, "OMP_DYNAMIC": "FALSE",
        "other_omp_environment": {k: v for k, v in os.environ.items() if k.startswith("OMP_") and k not in {"OMP_NUM_THREADS", "OMP_DYNAMIC"}},
        "inputs": [{"name": p.name, "bytes": p.stat().st_size, "sha256": huella(p)} for p in entradas],
    }
    print("BENCHMARK TRANSFORMACIONES BMP / OpenMP")
    print(f"Sistema: {metadata['platform']}; CPU logicas: {metadata['logical_cpus']}")
    print(f"Imagenes: {args.count}; salidas por ejecucion: {4 * args.count}")
    print(f"Repeticiones: {args.repetitions}; calentamientos por grupo: {args.warmups}")
    filas, resumenes = [], []
    for hilos in args.threads:
        print(f"\nHilos solicitados: {hilos}", flush=True)
        for _ in range(args.warmups):
            ejecutar(programa, entrada, salida, args.count, hilos, args.timeout)
        grupo = []
        for repeticion in range(1, args.repetitions + 1):
            pared, interno, reales = ejecutar(programa, entrada, salida, args.count, hilos, args.timeout)
            fila = {"threads_requested": hilos, "threads_actual": reales, "run": repeticion,
                    "wall_seconds": pared, "c_seconds": interno}
            grupo.append(fila)
            filas.append(fila)
            print(f"  {repeticion:02d}: total={pared:.6f} s; C={interno:.6f} s; hilos reales={reales}", flush=True)
        usados = {f["threads_actual"] for f in grupo}
        if len(usados) != 1:
            raise RuntimeError("Cambio el numero real de hilos dentro del grupo. Revisa las variables OMP_.")
        reales = usados.pop()
        if reales != hilos:
            print(f"  Aviso: OpenMP creo {reales} hilos de los {hilos} solicitados.")
        tiempos = [f["wall_seconds"] for f in grupo]
        resumenes.append({
            "threads_requested": hilos, "threads_actual": reales,
            "repetitions": args.repetitions, "wall_mean_seconds": statistics.mean(tiempos),
            "wall_min_seconds": min(tiempos), "wall_max_seconds": max(tiempos),
            "wall_stdev_seconds": statistics.stdev(tiempos) if len(tiempos) > 1 else 0.0,
            "c_mean_seconds": statistics.mean(f["c_seconds"] for f in grupo),
            "speedup_vs_1": "", "efficiency": "",
        })
    base = next((r["wall_mean_seconds"] for r in resumenes if r["threads_requested"] == 1 and r["threads_actual"] == 1), None)
    for resumen in resumenes:
        if base is not None:
            resumen["speedup_vs_1"] = base / resumen["wall_mean_seconds"]
            resumen["efficiency"] = resumen["speedup_vs_1"] / resumen["threads_actual"]
        print(f"Media con {resumen['threads_actual']} hilos: {resumen['wall_mean_seconds']:.6f} s")
    marca = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S_%fZ")
    guardar_csv(resultados / f"runs_{marca}.csv", filas)
    guardar_csv(resultados / f"summary_{marca}.csv", resumenes)
    (resultados / f"metadata_{marca}.json").write_text(json.dumps(metadata, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Resultados guardados en: {resultados}")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as error:
        print(f"Error: {error}", file=sys.stderr)
        sys.exit(1)
    except KeyboardInterrupt:
        sys.exit(130)
