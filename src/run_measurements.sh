#! /bin/bash

set -o xtrace

MEASUREMENTS=10
ITERATIONS=10
THREAD_IT=5
INITIAL_THREAD_NUM=2
INITIAL_SIZE=16

SIZE=$INITIAL_SIZE
THREAD_NUM=$INITIAL_THREAD_NUM
NAMES_PAR=('mandelbrot_mpi')
NAME_SEQ=('mandelbrot_seq')

make
mkdir results


mkdir results/$NAME_SEQ

for ((i=1; i<=$ITERATIONS; i++)); do
        perf stat -r $MEASUREMENTS ./$NAME_SEQ -2.5 1.5 -2.0 2.0 $SIZE >> full.log 2>&1
        perf stat -r $MEASUREMENTS ./$NAME_SEQ -0.8 -0.7 0.05 0.15 $SIZE >> seahorse.log 2>&1
        perf stat -r $MEASUREMENTS ./$NAME_SEQ 0.175 0.375 -0.1 0.1 $SIZE >> elephant.log 2>&1
        perf stat -r $MEASUREMENTS ./$NAME_SEQ -0.188 -0.012 0.554 0.754 $SIZE >> triple_spiral.log 2>&1
        SIZE=$(($SIZE * 2))
done

SIZE=$INITIAL_SIZE

mv *.log results/$NAME_SEQ
rm output.ppm


for NAME in ${NAMES_PAR[@]}; do
    mkdir results/$NAME

    for ((i=1; i<=$ITERATIONS; i++)); do
        for ((j=1; j<=$THREAD_IT; j++)); do
            perf stat -r $MEASUREMENTS ./$NAME -2.5 1.5 -2.0 2.0 $SIZE $THREAD_NUM >> full.log 2>&1
            perf stat -r $MEASUREMENTS ./$NAME -0.8 -0.7 0.05 0.15 $SIZE $THREAD_NUM >> seahorse.log 2>&1
            perf stat -r $MEASUREMENTS ./$NAME 0.175 0.375 -0.1 0.1 $SIZE $THREAD_NUM >> elephant.log 2>&1
            perf stat -r $MEASUREMENTS ./$NAME -0.188 -0.012 0.554 0.754 $SIZE $THREAD_NUM >> triple_spiral.log 2>&1
            
            THREAD_NUM=$(($THREAD_NUM * 2))
        done
        SIZE=$(($SIZE * 2))
        THREAD_NUM=$INITIAL_THREAD_NUM
    done

    THREAD_NUM=$INITIAL_THREAD_NUM
    SIZE=$INITIAL_SIZE

    mv *.log results/$NAME
    rm output.ppm
done
