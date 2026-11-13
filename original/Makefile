CC = clang
LIBOMP_PREFIX = /opt/homebrew/opt/libomp
CFLAGS = -std=c17 -O2 -Wall -Wextra -isystem $(LIBOMP_PREFIX)/include -Xpreprocessor -fopenmp
LDFLAGS = -L$(LIBOMP_PREFIX)/lib -Wl,-rpath,$(LIBOMP_PREFIX)/lib -lomp

all:
	mkdir -p build
	$(CC) $(CFLAGS) Transformaciones/main.c $(LDFLAGS) -o build/Transformaciones

run: all
	cd Transformaciones && ../build/Transformaciones
