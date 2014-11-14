#!/bin/sh
export MINET_DISPLAY=xterm
case "foo$MINET_MONITORTYPE" in
   foo) run_module.sh monitor;;
   footext) run_module.sh monitor;;
   foojavagui) java -jar mmonitor.jar & echo $! >> pids;
esac
