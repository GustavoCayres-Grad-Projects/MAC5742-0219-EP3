#! /bin/bash

set -o xtrace

MEASUREMENTS=10
ITERATIONS=1
THREAD_IT=2
INITIAL_THREAD_NUM=1 
INITIAL_SIZE=8192

SIZE=$INITIAL_SIZE
THREAD_NUM=$INITIAL_THREAD_NUM
NAME_SEQ=('mandelbrot_mpi')

make
mkdir results

mkdir results/$THREAD_IT

for ((i=1; i<=$ITERATIONS; i++)); do
        perf stat -r $MEASUREMENTS mpirun -np 8 -hostfile hostfile ./$NAME_SEQ --allow-run-as-root -2.5 1.5 -2.0 2.0 $SIZE >> full.log 2>&1
        perf stat -r $MEASUREMENTS mpirun -np 8 -hostfile hostfile ./$NAME_SEQ --allow-run-as-root -0.8 -0.7 0.05 0.15 $SIZE >> seahorse.log 2>&1
        perf stat -r $MEASUREMENTS mpirun -np 8 -hostfile hostfile ./$NAME_SEQ --allow-run-as-root 0.175 0.375 -0.1 0.1 $SIZE >> elephant.log 2>&1
        perf stat -r $MEASUREMENTS mpirun -np 8 -hostfile hostfile ./$NAME_SEQ --allow-run-as-root -0.188 -0.012 0.554 0.754 $SIZE >> triple_spiral.log 2>&1
        SIZE=$(($SIZE * 2))
done

SIZE=$INITIAL_SIZE

mv *.log results/$THREAD_IT
rm output.ppm
