#!/bin/bash

divide_numbers() {
 	echo "$1 $2" | awk '{printf "%.3f", $1/$2}'
}

add_numbers() {
 echo "$1 $2" | awk '{printf "%.3f", $1+$2}'
}

min_runtime=$((SECONDS+10))
RESULTS=()

while [ $SECONDS -lt $min_runtime ]; do
 RESULT=$(fio --direct=1 --gtod_reduce=1 --filename=fio_test --name=fio_rand_write_test --iodepth=64 --size=16M --readwrite=randwrite --runtime=11 --time_based=9 --invalidate=1)
 # RESULT="read: IOPS=10.1 write: IOPS=100.2"
 rm fio_test

 # regex_write="write.*?(IOPS|iops)=([0-9]*.[0-9]+)"
regex_write="write:[[:blank:]]IOPS=([0-9]*.[0-9]+)"

 if [[ $RESULT =~ $regex_write ]]
 then
     VALUE_WRITE="${BASH_REMATCH[1]}"
 fi

 if [[ $VALUE_WRITE ]]
 then
  RESULTS+=($VALUE_WRITE)
 else 
  echo "Error getting results"
 fi
done

# Calculate average
total=0
for var in "${RESULTS[@]}"
do
    total=$(add_numbers $total $var)
done

average=$(divide_numbers $total ${#RESULTS[@]})

echo $average
