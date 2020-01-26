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

msg(){
  input="$1"

  # underscore;Cyan
  echo -e "\n\033[4;36m$input\033[0;37m"
}

msg "加算，減算のテスト"
try 100 "20-10+120-5-5-30+10"
try 24 "   5 - 2+4  +3 -1+4 + 12-     1"
try 1 " 2+   3 - 3-1  "

msg "エラー出力のテスト"
./9cc "1+ hoge- 3"

msg "四則演算のテスト"
try 6 "2*3"
try 2 "1+5/3"
try 3 " ( 10* 2 +   1 -3) / 6"
try 2 "20 *2/ (4*(2+3))"

msg "単項演算子のテスト"
try 1 "-2* 2 + +5"
try 10 "+10/-2+-7-(-2)++20"
try 3 "2 --1"

echo -e "\nOK"
