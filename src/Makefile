OUTPUT=mandelbrot

IMAGE=.ppm

CC=mpicc
CC_OPT=-std=c11

CC_PTH=-pthread

.PHONY: all
all: $(OUTPUT)_mpi

$(OUTPUT)_omp: $(OUTPUT)_mpi.c
	$(CC) $(CC_OPT) -o $(OUTPUT)_mpi $(OUTPUT)_mpi.c

.PHONY: clean
clean:
	rm $(OUTPUT)_mpi *$(IMAGE)
