# Imágenes de entrada

Usa BMP de 24 bits sin compresión, llamados `img0.bmp`, `img1.bmp`, etc. El valor `--count` del benchmark indica cuántos archivos consecutivos se procesan; por defecto son diez.

Desde la raíz del proyecto puedes generar un conjunto sintético:

```bash
python3 scripts/generar_imagenes.py
```

En Windows usa `python`. El generador no sobrescribe entradas existentes sin `--force`.

También puedes copiar aquí los archivos de `BMP_image-processing-library-main/images/input/` del ZIP original. Los BMP locales no se incluyen en Git.
