#!/bin/bash

FILE="$1"

echo "// See https://github.com/thoriumlang/thm/wiki/Instructions"
echo "#include \"ops.h\""
echo "op_ptr ops[OPS_COUNT] = {"
OP_INDEX=0
for op in `cat $FILE`; do
  MNEMONIC=`echo "$op" | cut -d',' -f2`
  BYTECODE=`echo "$op" | cut -d',' -f1`
  FN_NAME=`echo "$MNEMONIC" | sed -E -e 's/([A-Z][a-z]+)([A-Z])(.*)/\1_\2\3/' | tr '[:upper:]' '[:lower:]'`

  while [[ $OP_INDEX -lt $BYTECODE ]]; do
    echo "        NULL, // 0x`printf '%02x\n' $OP_INDEX`,"
    OP_INDEX=`echo "$OP_INDEX + 1" | bc`
  done

  echo "#ifdef OP_$(echo $FN_NAME |  tr '[:lower:]' '[:upper:]')"
  echo "        &op_$FN_NAME, // 0x`printf '%02x\n' $BYTECODE`,"
  echo "#else"
  echo "        NULL,"
  echo "#endif"
  OP_INDEX=`echo "$OP_INDEX + 1" | bc`
done
echo "};"