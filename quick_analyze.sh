#!/bin/bash

if [ $# -lt 1 ]; then
  echo "Usage: $0 <file_number> [start_event_number]"
  exit 2
fi

fileno=$1

evstart=0
if [ $# -gt 1 ]; then
  evstart=$2
fi

filecode=`printf '%05g' $fileno`

./rc_textanalyser $evstart /disk02/comet/2023.2.Phase-Alpha/decdata/rawdata${filecode}.root
