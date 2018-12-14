#!/bin/bash
EXECUTABLE=$(dirname $0)
EXECUTABLE="${EXECUTABLE}/memsweep"
if [ ! -e ${EXECUTABLE} ] ; then
	cc -O -o ${EXECUTABLE} ${EXECUTABLE}.c -lm
fi

divide_numbers() {
 	echo "$1 $2" | awk '{printf "%.3f", $1/$2}'
}

add_numbers() {
 echo "$1 $2" | awk '{printf "%.3f", $1+$2}'
}

BENCHMARK_END=$((SECONDS+10))
RESULT=()

while [ $SECONDS -lt $BENCHMARK_END ]; do
	RESULT=$(${EXECUTABLE})
	RESULTS+=($RESULT)
done

TOTAL=0
for VAR in "${RESULTS[@]}"
do
    TOTAL=$(add_numbers $TOTAL $VAR)
done

AVERAGE=$(divide_numbers $TOTAL ${#RESULTS[@]})

echo $AVERAGE

rm ${EXECUTABLE}
