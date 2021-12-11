#!/bin/bash

FILE="$1"

echo "// See https://github.com/thoriumlang/thm/wiki/Instructions"
echo "#include \"ops.h\""
echo "op_ptr ops[256] = {"
OP_INDEX=0
for op in $(cat "$FILE"); do
  MNEMONIC=$(echo "$op" | cut -d',' -f2)
  BYTECODE=$(echo "$op" | cut -d',' -f1)
  FN_NAME=$(echo "$MNEMONIC" | sed -E -e 's/([A-Z][a-z]+)([A-Z])(.*)/\1_\2\3/' | tr '[:upper:]' '[:lower:]')

  while [[ $OP_INDEX -lt $BYTECODE ]]; do
    echo "        NULL, // 0x$(printf '%02x\n' $OP_INDEX),"
    OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
  done

  echo "        &op_$FN_NAME, // 0x$(printf '%02x\n' $BYTECODE),"
  OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
done
while [[ $OP_INDEX -lt 256 ]]; do
  echo "        NULL, // 0x$(printf '%02x\n' $OP_INDEX),"
  OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
done
echo "};"

echo "enum ops_list {"
OP_INDEX=0
for op in $(cat "$FILE"); do
  MNEMONIC=$(echo "$op" | cut -d',' -f2)
  BYTECODE=$(echo "$op" | cut -d',' -f1)
  MNEMONIC=$(echo "$MNEMONIC" | sed -E -e 's/([A-Z][a-z]+)([A-Z])(.*)/\1_\2\3/' | tr '[:lower:]' '[:upper:]')

  while [[ $OP_INDEX -lt $BYTECODE ]]; do
    echo "        _$OP_INDEX, // 0x$(printf '%02x\n' $OP_INDEX),"
    OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
  done

  echo "        $MNEMONIC, // 0x$(printf '%02x\n' $BYTECODE),"
  OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
done
while [[ $OP_INDEX -lt 256 ]]; do
  echo "        _$OP_INDEX, // 0x$(printf '%02x\n' $OP_INDEX),"
  OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
done
echo "};"

PAD=6
echo "char *ops_name[256] = {"
OP_INDEX=0
for op in $(cat "$FILE"); do
  MNEMONIC=$(echo "$op" | cut -d',' -f2)
  BYTECODE=$(echo "$op" | cut -d',' -f1)
  MNEMONIC=$(echo "$MNEMONIC" | sed -E -e 's/([A-Z][a-z]+)([A-Z])(.*)/\1_\2\3/' | tr '[:lower:]' '[:upper:]' | cut -d'_' -f1)
  MNEMONIC=$MNEMONIC$(printf '%*s' "$PAD" "")
  MNEMONIC=$(echo "${MNEMONIC}"|grep -Eo "^.{1,$PAD}")

  while [[ $OP_INDEX -lt $BYTECODE ]]; do
    echo "        \"_    \", // 0x$(printf '%02x\n' $OP_INDEX),"
    OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
  done

  echo "        \"$MNEMONIC\", // 0x$(printf '%02x\n' $BYTECODE),"
  OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
done
while [[ $OP_INDEX -lt 256 ]]; do
  echo "        \"_    \", // 0x$(printf '%02x\n' $OP_INDEX),"
  OP_INDEX=$(echo "$OP_INDEX + 1" | bc)
done
echo "};"
