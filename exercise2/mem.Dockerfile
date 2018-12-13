FROM ubuntu

RUN mkdir -p /usr/local/mocc
WORKDIR /usr/local/mocc
RUN apt-get update
RUN apt-get -y install gcc
ADD scripts/measure-mem.sh measure-mem.sh
ADD scripts/memsweap.c memsweap.c

ENTRYPOINT for i in `seq 1 48`; do echo $(date +\%s)","$(./measure-mem.sh) >> "docker-mem.csv"; sync; echo 1 > /proc/sys/vm/drop_caches; sleep 15; done && cat docker-mem.csv