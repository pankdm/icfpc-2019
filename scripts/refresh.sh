#!/usr/bin/env bash

while [ 1 ]; do
    git pull
    git add ../lambda-client/data/*
    git add ../lambda-client/state.json
    git add ../lambda-client/balance.txt
    git commit -m "new data"
    git push
    cd ../src
    make
    cd -

    ps_out=$(ps fx | grep submitter | grep -v grep)
    if [[ "${ps_out}" == "" ]]; then
        cd ../lambda-client
        ./submitter.py &
        cd -
    fi

    sleep 60
done
