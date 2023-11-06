#!/bin/bash

if [ $# -lt 3 ]; then
  echo "Usage: $0 <file_number> <event_number> <waveform_channel_0_to_3>"
  exit 2
fi

fileno=$1
eventno=$2
chan=$3

filecode=`printf '%05g' $fileno`
dumpout=`mktemp`

./dumpwf_rc $eventno $chan /disk02/comet/2023.2.Phase-Alpha/decdata/rawdata${filecode}.root > $dumpout

if [ $? -ne 0 ]; then
  echo "Error in dumpwf_rc"
  rm -f $dumpout
  exit 1
fi

set -- `stty size`
rows=$(($1-1))
cols=$(($2-1))
gplotcmds=`mktemp`
echo "set terminal dumb ${cols},${rows}" > $gplotcmds
echo "plot \"${dumpout}\" title \"file ${fileno} event ${eventno} channel ${chan}\" with lines" >> $gplotcmds

gnuplot $gplotcmds

rm -f $gplotcmds
rm -f $dumpout

