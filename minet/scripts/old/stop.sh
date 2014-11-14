#!/bin/sh

kill `cat pids`
killall reader
killall writer

