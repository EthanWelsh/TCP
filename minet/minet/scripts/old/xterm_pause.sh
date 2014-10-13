#!/bin/sh
$*
echo $! >> pids
echo "Hit ctrl-d to close this window"
cat - > /dev/null
