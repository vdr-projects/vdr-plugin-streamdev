#!/bin/bash
#
# externremux.sh - sample remux script using mencoder for remuxing.
#
# Install this script as VDRCONFDIR/plugins/streamdev-server/externremux.sh
#
# The parameter QUALITY selects the default remux parameters. Adjust
# to your needs and point your web browser to http://servername:3000/ext/
# To select different remux parameters on the fly, insert a semicolon
# followed by the name and value of the requested parameter, e.g:
#   e.g. http://servername:3000/ext;QUALITY=WLAN11;VBR=512/
# The following parameters are recognized:
#
# PROG   actual remux program
# VC     video codec
# VBR    video bitrate (kbit)
# VOPTS  custom video options
# WIDTH  scale video to width
# AC     audio codec
# ABR    audio bitrate (kbit)
# AOPTS  custom audio options
#

##########################################################################

### GENERAL CONFIG START
###
# Pick one of DSL1000/DSL2000/DSL3000/DSL6000/DSL16000/LAN10/WLAN11/WLAN54
QUALITY='DSL1000'
# Program used for logging (logging disabled if empty)
LOGGER=logger
# Path and name of FIFO
FIFO=/tmp/externremux-${RANDOM:-$$}
# Default remux program (cat/mencoder/ogg)
PROG=mencoder
# Use mono if $ABR is lower than this value
ABR_MONO=64
###
### GENERAL CONFIG END

### MENCODER CONFIG START
###
# mencoder binary
MENCODER=mencoder
# Default video codec (e.g. lavc/x264/copy)
MENCODER_VC=lavc
# Default audio codec (e.g. lavc/mp3lame/faac/copy)
MENCODER_AC=mp3lame
# Default video codec if lavc is used (-ovc lavc -lavcopts vcodec=)
MENCODER_LAVC_VC=mpeg4
# Default audio codec if lavc is used (-oac lavc -lavcopts acodec=)
MENCODER_LAVC_AC=mp2
###
### MENCODER CONFIG END

### OGG CONFIG START
###
# ffmpeg2theora binary 
OGG=ffmpeg2theora
# speedlevel - lower value gives better quality but is slower (0..2)
OGG_SPEED=1
# videoquality - higher value gives better quality but is slower (0..10)
OGG_VQUALITY=0
# audioquality - higher value gives better quality but is slower (0..10)
OGG_AQUALITY=0
###
### OGG CONFIG END

##########################################################################

function hasOpt { echo "$1" | grep -q "\b${2}\b"; }

function isNumeric() { echo "$@" | grep -q '^[0-9]\{1,\}$'; }

function remux_cat
{
	startReply
	exec 3<&0
	cat 0<&3 >"$FIFO" &
}

function remux_mencoder
{
	# lavc may be used for video and audio
	LAVCOPTS=()

	# Assemble video options
	VC=${REMUX_PARAM_VC:-$MENCODER_VC}
	VOPTS=${REMUX_PARAM_VOPTS}
	WIDTH=${REMUX_PARAM_WIDTH:-$WIDTH}
	case "$VC" in
		lavc)
			LAVCOPTS=(
				${VOPTS}
				$(hasOpt "$VOPTS" vcodec || echo "vcodec=$MENCODER_LAVC_VC")
				${VBR:+vbitrate=$VBR}
			)
			[ ${#LAVCOPTS[*]} -gt 0 ] && VOPTS=$(IFS=:; echo -lavcopts "${LAVCOPTS[*]}")
			;; 
		x264)
			X264OPTS=(
				${VOPTS}
				$(hasOpt "$VOPTS" threads || echo "threads=auto")
				${VBR:+bitrate=$ABR}
			)
			[ ${#X264OPTS[*]} -gt 0 ] && VOPTS=$(IFS=:; echo -x264encopts "${X264OPTS[*]}")
			;;
		copy)
			VOPTS=
			;;
		*)
			error "Unknown video codec '$VC'"
			;;
	esac

	# Assemble audio options 
	AC=${REMUX_PARAM_AC:-$MENCODER_AC}
	AOPTS=${REMUX_PARAM_AOPTS}
	case "$AC" in
		lavc)
			LAVCOPTS=(
				${LAVCOPTS[*]}
				${AOPTS}
				$(hasOpt "$AOPTS" acodec || echo "acodec=$MENCODER_LAVC_AC")
				${ABR:+abitrate=$ABR}
			)
	
			[ ${#LAVCOPTS[*]} -gt 0 ] && AOPTS=$(IFS=:; echo -lavcopts "${LAVCOPTS[*]}")
			# lavc used for video and audio decoding - wipe out VOPTS as video options became part of AOPTS
			[ "$VC" = lavc ] && VOPTS=
			;; 
		mp3lame)
			LAMEOPTS=(
				${AOPTS}
				$(isNumeric "${ABR}" && [ "${ABR}" -lt "$ABR_MONO" ] && ! hasOpt "${AOPTS}" mode ] && echo 'mode=3')
				${ABR:+preset=$ABR}
			)
			[ ${#LAMEOPTS[*]} -gt 0 ] && AOPTS=$(IFS=:; echo -lameopts "${LAMEOPTS[*]}")
			;;
		faac)
			FAACOPTS=(
				${AOPTS}
				${ABR:+br=$ABR}
			)
			[ ${#FAACOPTS[*]} -gt 0 ] && AOPTS=$(IFS=:; echo -faacopts "${FAACOPTS[*]}")
			;;
		copy)
			AOPTS=
			;;
		*)
			error "Unknown audio codec '$AC'"
			;;
	esac


	startReply
	exec 3<&0
	echo "$MENCODER" \
		-ovc $VC $VOPTS \
		-oac $AC $AOPTS \
		${WIDTH:+-vf scale -zoom -xy $WIDTH} \
		-o "$FIFO" -- - >&2
	"$MENCODER" \
		-ovc $VC $VOPTS \
		-oac $AC $AOPTS \
		${WIDTH:+-vf scale -zoom -xy $WIDTH} \
		-o "$FIFO" -- - 0<&3 >/dev/null &
}

function remux_ogg
{
	VOPTS=${REMUX_PARAM_VOPTS//[:=]/ }
	AOPTS=${REMUX_PARAM_AOPTS//[:=]/ }
	WIDTH=${REMUX_PARAM_WIDTH:-$WIDTH}

	OGGOPTS=(
		${VOPTS}
		${VBR:+--videobitrate $VBR}
		$(hasOpt "${VOPTS}" videoquality || echo "--videoquality $OGG_VQUALITY")
		$(hasOpt "${VOPTS}" speedlevel || echo "--speedlevel $OGG_SPEED")
		${AOPTS}
		${ABR:+--audiobitrate $ABR}
		$(isNumeric "${ABR}" && [ "${ABR}" -lt "$ABR_MONO" ] && ! hasOpt "${AOPTS}" channels ] && echo '--channels 1')
		$(hasOpt "${AOPTS}" audioquality || echo "--audioquality $OGG_AQUALITY")
		$(hasOpt "${AOPTS}" audiostream || echo '--audiostream 1')
	)

	startReply
	exec 3<&0
	echo "$OGG" --format ts \
		${OGGOPTS[*]} \
		${WIDTH:+--width $WIDTH --height $(($WIDTH * 3 / 4 / 8 * 8))} \
                --title "VDR Streamdev: ${REMUX_CHANNEL_NAME}" \
                --output "$FIFO" -- - 0<&3 >&2
	"$OGG" --format ts \
		${OGGOPTS[*]} \
		${WIDTH:+--width $WIDTH --height $(($WIDTH * 3 / 4 / 8 * 8))} \
                --title "VDR Streamdev: ${REMUX_CHANNEL_NAME}" \
                --output "$FIFO" -- - 0<&3 >/dev/null &
}

function error
{
	if [ "$SERVER_PROTOCOL" = HTTP ]; then
		echo -ne "Content-type: text/plain\r\n"
		echo -ne '\r\n'
		echo "$*"
	fi

	echo "$*" >&2
	exit 1
}

function startReply
{
	if [ "$SERVER_PROTOCOL" = HTTP ]; then
		# send content-type and custom headers
		echo -ne "Content-type: ${CONTENTTYPE}\r\n"
		for header in "${HEADER[@]}"; do echo -ne "$header\r\n"; done
		echo -ne '\r\n'

		# abort after headers
		[ "$REQUEST_METHOD" = HEAD ] && exit 0
	fi

	# create FIFO and read from it in the background
	mkfifo "$FIFO"
	trap "kill 0; sleep 1; rm '$FIFO'; trap - EXIT HUP INT TERM ABRT PIPE CHLD" EXIT HUP INT TERM ABRT PIPE CHLD
	cat "$FIFO" <&- &
}

HEADER=()

set > /tmp/env
[ "$LOGGER" ] && exec 2> >($LOGGER -t "vdr: [$$] streamdev EXT" 2>&-)

# set default content-types
case "$REMUX_VPID" in
	''|0|1) CONTENTTYPE='audio/mpeg';;
	*)      CONTENTTYPE='video/mpeg';;
esac

QUALITY=${REMUX_PARAM_QUALITY:-$QUALITY}
case "$QUALITY" in
	DSL1000)  VBR=96;   ABR=16;  WIDTH=160;;
	DSL2000)  VBR=128;  ABR=16;  WIDTH=160;;
	DSL3000)  VBR=256;  ABR=16;  WIDTH=320;;
	DSL6000)  VBR=378;  ABR=32;  WIDTH=320;;
	DSL16000) VBR=512;  ABR=32;  WIDTH=480;;
	WLAN11)   VBR=768;  ABR=64;  WIDTH=640;;
	WLAN45)   VBR=2048; ABR=128; WIDTH=;;
	LAN10)    VBR=4096; ABR=;    WIDTH=;;
	*)        error "Unknown quality '$QUALITY'";;
esac
ABR=${REMUX_PARAM_ABR:-$ABR}
VBR=${REMUX_PARAM_VBR:-$VBR}
WIDTH=${REMUX_PARAM_WIDTH:-$WIDTH}
PROG=${REMUX_PARAM_PROG:-$PROG}

case "$PROG" in
	cat)      remux_cat;;
	mencoder) remux_mencoder;;
	ogg)      remux_ogg;;
	*)        error "Unknown remuxer '$PROG'";;
esac

set -o monitor
wait
