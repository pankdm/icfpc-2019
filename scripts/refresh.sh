#!/usr/bin/env bash

while [ 1 ]; do
    git pull
    cd ../src
    make
    cd -
    sleep 60
done
