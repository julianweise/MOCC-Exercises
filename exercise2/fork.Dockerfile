FROM ubuntu

RUN mkdir -p /usr/local/mocc
WORKDIR /usr/local/mocc
RUN apt-get update
RUN apt-get -y install gcc
ADD measure-fork.sh measure-fork.sh
ADD forksum.c forksum.c
RUN echo "time,value" >> "docker-fork.csv"

ENTRYPOINT for i in `seq 1 48`; do echo $(date +\%s)","$(./measure-fork.sh) >> "docker-fork.csv"; sleep 3; done && cat docker-fork.csv