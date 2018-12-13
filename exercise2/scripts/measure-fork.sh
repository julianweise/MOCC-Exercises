#!/bin/bash
EXECUTABLE=$(dirname $0)
EXECUTABLE="${EXECUTABLE}/forksum"
if [ ! -e ${EXECUTABLE} ] ; then
	cc -O -o ${EXECUTABLE} ${EXECUTABLE}.c -lm -w
fi

divide_numbers() {
 	echo "$1 $2" | awk '{printf "%.3f", $1/$2}'
}

add_numbers() {
 echo "$1 $2" | awk '{printf "%.3f", $1+$2}'
}

sub_numbers() {
 echo "$1 $2" | awk '{printf "%.3f", $1-$2}'
}

BENCHMARK_END=$((SECONDS+15))
INTERVAL_STEP=50
INTERVAL_LOWER=0
INTERVAL_UPPER=0

while [ $SECONDS -lt $BENCHMARK_END ]; do
	INTERVAL_UPPER=$(add_numbers $INTERVAL_UPPER $INTERVAL_STEP)

	RESULT=$(${EXECUTABLE} $INTERVAL_LOWER $INTERVAL_UPPER)
	RESULTS+=($RESULT)
done

RESULT=$(divide_numbers $INTERVAL_UPPER $SECONDS)

echo $RESULT

rm ${EXECUTABLE}