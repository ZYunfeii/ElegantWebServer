FROM ubuntu:18.04

ADD ./code/ /server/code
ADD ./CMakeLists.txt /server
ADD ./resources /server/resources
ADD redis-7.0.4.tar.gz /server
ADD hiredis-1.0.2.tar.gz /server
ADD dockerrun.sh /server
WORKDIR /server
RUN chmod 777 dockerrun.sh

RUN apt-get update
RUN apt-get install -y cmake
RUN apt-get install -y build-essential
RUN apt-get install -y openssl
RUN apt-get install -y libssl-dev
RUN apt-get clean

WORKDIR /server/redis-7.0.4/src
RUN make && make install
WORKDIR /server/hiredis-1.0.2
RUN make && make install
RUN ldconfig

WORKDIR /server
RUN mkdir build
RUN mkdir bin
RUN mkdir log
RUN mkdir redisfiles
RUN mkdir user-msgs

WORKDIR /server/build
RUN cmake ..
RUN make


WORKDIR /server
ENTRYPOINT  ["./dockerrun.sh"]