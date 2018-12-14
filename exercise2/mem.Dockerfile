FROM ubuntu

RUN mkdir -p /usr/local/mocc
WORKDIR /usr/local/mocc
RUN apt-get update
RUN apt-get -y install gcc
ADD measure-mem.sh measure-mem.sh
ADD memsweap.c memsweap.c
RUN echo "time,value" >> "docker-mem.csv"

ENTRYPOINT for i in `seq 1 48`; do echo $(date +\%s)","$(./measure-mem.sh) >> "docker-mem.csv"; sleep 3; done && cat docker-mem.csv