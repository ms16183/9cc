#! /bin/bash

try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 100 "20-10+120-5-5-30+10"
try 24 "   5 - 2+4  +3 -1+4 + 12-     1"
try 1 " 2+   3 - 3-1  "

# Error
./9cc "1 +   2 --1 "
./9cc "1+ hoge- 3"

echo OK
