#!/bin/bash

FILE="$1"

echo "/// See https://github.com/thoriumlang/thm/wiki/Instructions"
echo "#[derive(Debug, PartialEq, Clone, Copy)]"
echo "pub enum Op {"
for op in `cat $FILE`; do
  BYTECODE=`echo "$op" | cut -d',' -f1`
  MNEMONIC=`echo "$op" | cut -d',' -f2`
  echo "    $MNEMONIC = $BYTECODE, // 0x`printf '%02x\n' $BYTECODE`"
done
echo "}"
echo ""
echo "impl Op {"
echo "    pub fn length(&self) -> u8 {"
echo "        match self {"
for op in `cat $FILE`; do
  MNEMONIC=`echo "$op" | cut -d',' -f2`
  LENGTH=`echo "$op" | cut -d',' -f3`
  echo "            Op::$MNEMONIC => $LENGTH,"
done
echo "        }"
echo "    }"
echo ""
echo "    pub fn bytecode(&self) -> u8 {"
echo "        *self as u8"
echo "    }"
echo "}"
echo ""
echo "impl From<u8> for Op {"
echo "    fn from(v: u8) -> Self {"
echo "        match v {"
for op in `cat $FILE`; do
  BYTECODE=`echo "$op" | cut -d',' -f1`
  MNEMONIC=`echo "$op" | cut -d',' -f2`
echo "            $BYTECODE => Self::$MNEMONIC,"
done
echo "            _ => Self::Panic,"
echo "        }"
echo "    }"
echo "}"
echo ""
echo "#[cfg(test)]"
echo "mod tests {"
echo "    use super::*;"
for op in `cat $FILE`; do
  MNEMONIC=`echo "$op" | cut -d',' -f2`
  MNEMONIC_LOWER=`echo "$MNEMONIC" | tr '[:upper:]' '[:lower:]'`
  echo ""
  echo "    #[test]"
  echo "    fn test_$MNEMONIC_LOWER() {"
  echo "        assert_eq!(Op::$MNEMONIC, Op::from(Op::$MNEMONIC.bytecode()));"
  echo "    }"
done
echo "}"