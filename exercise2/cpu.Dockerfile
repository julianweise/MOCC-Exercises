FROM ubuntu

RUN mkdir -p /usr/local/mocc
WORKDIR /usr/local/mocc
RUN apt-get update
RUN apt-get -y install gcc
ADD measure-cpu.sh measure-cpu.sh
ADD linpack.c linpack.c
RUN echo "time,value" >> "docker-cpu.csv"

ENTRYPOINT for i in `seq 1 48`; do echo $(date +\%s)","$(./measure-cpu.sh) >> "docker-cpu.csv"; sleep 3; done && cat docker-cpu.csv