#!/bin/sh

if [ ! -f pids ]; then
    echo "No PID file found. Is Minet Running?"
    exit
fi


kill `cat pids`
#killall reader
#killall writer

rm -f pids
