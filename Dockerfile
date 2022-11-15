FROM ubuntu:18.04
LABEL maintainer="yunfei_z@buaa.edu.cn"
ENV PROJECT_DIR=/server 
# copy the code and something others
ADD ./code/ $PROJECT_DIR/code
ADD ./CMakeLists.txt $PROJECT_DIR
ADD ./resources $PROJECT_DIR/resources
ADD redis-7.0.4.tar.gz $PROJECT_DIR
ADD hiredis-1.0.2.tar.gz $PROJECT_DIR
# install some package necessary
RUN apt-get update && apt-get install -y cmake && apt-get install -y build-essential \
    && apt-get install -y openssl && apt-get install -y libssl-dev && apt-get clean && \
    rm -rf /var/lib/apt/lists/*
# install redis and hiredis
WORKDIR $PROJECT_DIR/redis-7.0.4/src
RUN make && make install
WORKDIR $PROJECT_DIR/hiredis-1.0.2
RUN make && make install
RUN ldconfig
# create some folders to store the log and something others
WORKDIR $PROJECT_DIR
RUN mkdir build && mkdir bin && mkdir log && mkdir redisfiles && mkdir user-msgs
# compile the web server
WORKDIR $PROJECT_DIR/build
RUN cmake .. && make

WORKDIR $PROJECT_DIR