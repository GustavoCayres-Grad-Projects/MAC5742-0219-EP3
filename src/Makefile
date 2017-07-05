OUTPUT=mandelbrot

IMAGE=.ppm

CC=gcc
MPICC=mpicc
CC_OPT=-std=c11

.PHONY: all
all: $(OUTPUT)_mpi $(OUTPUT)_seq

$(OUTPUT)_mpi: $(OUTPUT)_omp.c
	$(MPICC) -o $(OUTPUT)_mpi $(CC_OPT) $(OUTPUT)_mpi.c

$(OUTPUT)_seq: $(OUTPUT)_seq.c
	$(CC) -o $(OUTPUT)_seq $(CC_OPT) $(OUTPUT)_seq.c

.PHONY: clean
clean:
	rm $(OUTPUT)_mpi $(OUTPUT)_seq *$(IMAGE)
