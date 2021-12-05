#!/bin/bash

FILE="$1"

for reg in `cat $FILE`; do
  REG_NAME=`echo "$reg" | cut -d',' -f1`
  REG_INDEX=`echo "$reg" | cut -d',' -f2`
  echo "#define $REG_NAME $REG_INDEX"
done