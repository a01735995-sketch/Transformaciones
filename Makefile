ifeq ($(origin CC), default)
CC = cc
endif
CFLAGS ?= -std=c17 -O2 -Wall -Wextra
PYTHON ?= python3

ifeq ($(shell uname -s),Darwin)
LIBOMP_PREFIX ?= $(shell brew --prefix libomp 2>/dev/null)
OPENMP_CFLAGS ?= -Xpreprocessor -fopenmp -isystem $(LIBOMP_PREFIX)/include
OPENMP_LDFLAGS ?= -L$(LIBOMP_PREFIX)/lib -Wl,-rpath,$(LIBOMP_PREFIX)/lib -lomp
else
OPENMP_CFLAGS ?= -fopenmp
OPENMP_LDFLAGS ?= -fopenmp
endif

ifeq ($(OS),Windows_NT)
EXE = .exe
endif
TARGET = build/transformaciones_bench$(EXE)

.PHONY: all run benchmark sample test clean
all: $(TARGET)

$(TARGET): bench_mark_transformaciones.c Transformaciones/Transformaciones.h
	mkdir -p build
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) bench_mark_transformaciones.c $(LDFLAGS) $(OPENMP_LDFLAGS) -o $@

run: all
	./$(TARGET)

benchmark: all
	$(PYTHON) benchmark_transformaciones.py

sample:
	$(PYTHON) scripts/generar_imagenes.py

test: all
	$(PYTHON) -m unittest discover -s tests -v

clean:
	rm -f $(TARGET)
