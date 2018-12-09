#!/bin/bash
EXECUTABLE=$(dirname $0)
EXECUTABLE="${EXECUTABLE}/forksum"
if [ ! -e ${EXECUTABLE} ] ; then
	cc -O -o ${EXECUTABLE} ${EXECUTABLE}.c -lm
fi

RESULT=$(${EXECUTABLE}