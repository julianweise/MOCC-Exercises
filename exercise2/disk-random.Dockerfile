FROM ubuntu

RUN mkdir -p /usr/local/mocc
WORKDIR /usr/local/mocc
RUN apt-get update
RUN apt-get -y install fio
ADD measure-disk-random.sh measure-disk-ranom.sh
RUN echo "time,value" >> "docker-disk-random.csv"

ENTRYPOINT for i in `seq 1 48`; do echo $(date +\%s)","$(./measure-disk-random.sh) >> "docker-disk-random.csv"; sleep 15; done && cat docker-disk-random.csv