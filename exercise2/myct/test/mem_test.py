#!/usr/bin/python3
import numpy
result = [numpy.random.bytes(1024*1024) for x in range(2048)]
print(len(result))