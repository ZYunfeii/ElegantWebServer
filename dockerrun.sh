#!/bin/bash

{
    echo "server start."
    ./build/server
} & 
{
    echo "redis start."
    redis-server ./code/redis/redis.conf
}&

while [[ true ]]; do
    sleep 1
done