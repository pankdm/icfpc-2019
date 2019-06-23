#!/usr/bin/env bash

while [ 1 ]; do
    git pull
    git add ../lambda-client/data/*
    git commit -m "new data"
    git push
    cd ../src
    make
    cd -
    sleep 60
done
