#!/bin/sh
#
# externremux.sh - sample remux script using mencoder for remuxing.
#
# Install this script as VDRCONFDIR/plugins/streamdev/externremux.sh
#
# The parameter STREAMQUALITY selects the default remux parameters. Adjust
# to your needs and point your web browser to http://servername:3000/extern/
# To select different remux parameters on the fly, insert a semicolon and
# the name of the requested quality: http://servername:3000/extern;WLAN11/

# CONFIG START
  STREAMQUALITY="DSL6000" # DSL{1,2,3,6}000, LAN10, WLAN{11,54}, IPAQ
  TMP=/tmp/externremux-${RANDOM:-$$}
  MENCODER=mencoder
# CONFIG END

mkdir -p $TMP
mkfifo $TMP/out.avi
(trap "rm -rf $TMP" EXIT HUP INT TERM ABRT; cat $TMP/out.avi) &

case ${1:-$STREAMQUALITY} in
     DSL1000) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=100 \
		-oac mp3lame -lameopts preset=15:mode=3 -vf scale=160:104 \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
     DSL2000) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=128 \
		-oac mp3lame -lameopts preset=15:mode=3 -vf scale=160:104 \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
     DSL3000) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=250 \
		-oac mp3lame -lameopts preset=15:mode=3 -vf scale=320:208 \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
     DSL6000) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=350 \
		-oac mp3lame -lameopts preset=15:mode=3 -vf scale=320:208 \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
       LAN10) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=4096 \
		-oac mp3lame -lameopts preset=standard \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
      WLAN11) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=768 \
		-oac mp3lame -lameopts preset=standard -vf scale=640:408 \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
      WLAN54) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=2048 \
		-oac mp3lame -lameopts preset=standard \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
	IPAQ) exec $MENCODER -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=350 \
		-oac mp3lame -lameopts preset=15:mode=3 -vf scale=320:208 \
		-o $TMP/out.avi -- - &>$TMP/out.log ;;
	   *) touch $TMP/out.avi ;;
esac
