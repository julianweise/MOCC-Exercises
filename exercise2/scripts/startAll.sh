#!/bin/bash

clear_caches() {
	sync; echo 1 > /proc/sys/vm/drop_caches
}

ENVIRONMENT="native"
TIME_TO_WAIT_BETWEEN=15s

for i in `seq 1 48`;
do
	echo $(date +\%s)","$(./measure-cpu.sh) >> "../results/${ENVIRONMENT}-cpu.csv"
	clear_caches
	sleep TIME_TO_WAIT_BETWEEN
	echo $(date +\%s)","$(./measure-mem.sh) >> "../results/${ENVIRONMENT}-mem.csv"
	clear_caches
	sleep TIME_TO_WAIT_BETWEEN
	echo $(date +\%s)","$(./measure-disk-random.sh) >> "../results/${ENVIRONMENT}-disk-random.csv"
	clear_caches
	sleep TIME_TO_WAIT_BETWEEN
	echo $(date +\%s)","$(./measure-fork.sh) >> "../results/${ENVIRONMENT}-fork.csv"
	clear_caches
	sleep TIME_TO_WAIT_BETWEEN
done
