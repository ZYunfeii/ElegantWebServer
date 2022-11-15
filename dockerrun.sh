#!/bin/bash


redis-server ./code/redis/redis.conf
echo "redis start."
echo "server start."
./build/server

while [[ true ]]; do
    sleep 1
done