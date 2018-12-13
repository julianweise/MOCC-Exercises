FROM ubuntu

RUN mkdir -p /usr/local/mocc
WORKDIR /usr/local/mocc
RUN apt-get update
RUN apt-get -y install gcc
ADD scripts/measure-fork.sh measure-fork.sh
ADD scripts/forksum.c forksum.c

ENTRYPOINT for i in `seq 1 48`; do echo $(date +\%s)","$(./measure-fork.sh) >> "docker-fork.csv"; sync; echo 1 > /proc/sys/vm/drop_caches; sleep 15; done && cat docker-fork.csv