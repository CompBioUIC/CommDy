#!/bin/bash

awk 'BEGIN{print "time,group,individual"} {for (i=3; i<=NF; i++) {print $2","$1","$i}}' $*
