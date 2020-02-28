#! /bin/bash

msg(){
  # underscore;Cyan
  echo -e "\n\033[4;36m$1\033[0;37m"
}

try() {

  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s

  if [ -e tmp2.o ]; then
    gcc -static -o tmp tmp.s tmp2.o
  else
    gcc -static -o tmp tmp.s
  fi

  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input -> $actual"
  else
    echo "$input -> $expected expected, but got $actual"
    exit 1
  fi
}

out(){
  compatible="$1"
  input="$2"
  ./9cc "$input" > tmp.s
  gcc -static -o tmp tmp.s
  ./tmp
  ret="$?"
  echo "$input -> $ret (This evalution is not checked.)"
}

testcase(){

  case $1 in
  1 )
    msg "加算，減算のテスト"
    try 2 "main(){return 1+1;}"
    try 100 "main(){return 20-10+120-5-5-30+10;}"
    try 24 "main(){return   5 - 2+4  +3 -1+4 + 12-     1;}"
    try 1 "main(){return 2+   3 - 3-1  ;}"
    ;;

  2 )
    msg "四則演算のテスト"
    try 90 "main() { return 50+25+5*(10/2)-10; }"
    try  6 "main() { return 2*3; }"
    try  2 "main() { return 1+5/3; }"
    try  3 "main() { return  ( 10* 2 +   1 -3) / 6; }"
    try  2 "main() { return 20 *2/ (4*(2+3)); }"
    ;;

  3 )
    msg "単項演算子のテスト"
    try 1 "main(){return +1;}"
    try 1 "main(){return -1*-1;}"
    try 1 "main(){return ++++1;}"
    try 1 "main(){return ----1;}"
    try 1 "main(){return -2* 2 + +5;}"
    try 10 "main(){return +10/-2+-7-(-2)++20;}"
    try 3 "main(){return 2 --1;}"
    ;;

  4 )
    msg "比較演算子のテスト"
    try 1 "main(){return 100==100;}"
    try 0 "main(){return 100==0;}"
    try 1 "main(){return 100!=0;}"
    try 0 "main(){return 100!=100;}"
    try 1 "main(){return 100>0;}"
    try 0 "main(){return 100<0;}"
    try 0 "main(){return 100>100;}"
    try 0 "main(){return 100<100;}"

    try 1 "main(){return 100>=0;}"
    try 0 "main(){return 100<=0;}"
    try 1 "main(){return 100>=100;}"
    try 1 "main(){return 100<=100;}"

    try 1 "main(){return 1+1>0;}"
    try 2 "main(){return 1+(1>0);}"
    try 0 "main(){return (1+1)<0;}"

    try 1 "main(){return 1<2 == 3<4;}"
    try 1 "main(){return 1>2 != 3<4;}"

    try 1 "main(){return (2+10)*2==24;}"
    ;;

  5 )
    msg "変数代入のテスト"
    try   0 "main(){a=0; return a;}"
    try 100 "main(){_b=100; return _b;}"
    try 100 "main(){c1=100; return c1;}"
    try 100 "main(){_a1b2_ = 100; return _a1b2_;}"
    try  10 "main(){z=2; return z*(z*z)+z;}"
    try   1 "main(){hoge = 2; foo = 1 + hoge; return foo - hoge;}"
    try   6 "main(){a=1; b=2; c=3; return c + a + b;}"
    try   6 "main(){I = 1; love = 2; you = 3; return I*love*you;}"
    try   1 "main(){greater = 100; less = 20; return greater > less;}"
    ;;

  6 )
    msg "return文のテスト"
    try 6 "main(){return 1+2+3;}"
    try 2 "main(){hoge=2; return hoge;}"
    try 1 "main(){return 1; 2; 3;}"
    try 5 "main(){4; return 5; 6;}"
    try 9 "main(){7; 8; return 9;}"
    try 0 "main(){return 0; return 1;}"
    ;;

  7 )
    msg "if文のテスト"
    try  3 "main(){if(1+2 == 3) return 3; return 2;}"
    try  2 "main(){if(1*2 == 3) return 3; return 2;}"
    try  1 "main(){if ( 0 ) return 100; return 2-1;}"
    try  3 "main(){foo=3; if(5-foo == 5-(2+1))   return foo; return 0;}"
    try  0 "main(){foo=3; if(5-foo != 5-(2+1))   return foo; return 0;}"
    try  2 "main(){abc = 5; hoge = 2; if ( 0 ) hoge = abc * 3; return hoge;}"
    try 15 "main(){abc = 5; hoge = 2; if ( 1 ) hoge = abc * 3; return hoge;}"
    try  4 "main(){num=1; if(num == 1) if(num == 1) num = num*2; num = num*2; return num;}"
    try  2 "main(){num=1; if(num == 1) if(num != 1) num = num*2; num = num*2; return num;}"
    try  2 "main(){num=1; if(num != 1) if(num == 1) num = num*2; num = num*2; return num;}"
    try  2 "main(){num=1; if(num != 1) if(num != 1) num = num*2; num = num*2; return num;}"
    ;;

  8 )
    msg "if else文のテスト"
    try 5 "main(){hoge = 2; if(hoge == 2) hoge = 5; else hoge = 0; return hoge;}"
    try 0 "main(){hoge = 2; if(hoge != 2) hoge = 5; else hoge = 0; return hoge;}"
    try 1 "main(){a = 1; b = a; out = 0; if(a == 1) if(b == 1) out = 1; else out = 2; else out = 3; return out;}"
    try 3 "main(){a = 1; b = a; out = 0; if(a != 1) if(b == 1) out = 1; else out = 2; else out = 3; return out;}"
    try 2 "main(){a = 1; b = a; out = 0; if(a == 1) if(b != 1) out = 1; else out = 2; else out = 3; return out;}"
    try 3 "main(){a = 1; b = a; out = 0; if(a != 1) if(b != 1) out = 1; else out = 2; else out = 3; return out;}"
<< COMMENT
    if (a == 1)
      if (b == 1)
        out = 1
      else
        out = 2
    else
      out = 3
    return out
COMMENT
    ;;

  9 )
    msg "while文のテスト"
    try   3 "main(){i = 1; while(i < 3) i = i + 1; return i;}"
    try 200 "main(){i = 1; while(i < 100) if (i == 10) i = 200; else i = i + 1; return i;}"
    try   1 "main(){i = 1; while (0) i = i + 1; return i;}"
    try 100 "main(){i = 1; while ( 1 ) if (i == 100) return i; else i = i + 1;}"
    ;;

  10 )
    msg "for文のテスト"
    try 6 "main (){sum = 0; for(i = 1; i <= 3; i = i + 1) sum = sum + i; return sum;}"
    try 0 "main (){sum = 5050; for(i = 100; i > 0; i = i - 1) sum = sum - i; return sum;}"
    try 3 "main (){i = 0; for(     ;i < 3;) i = i + 1; return i;}"
    try 3 "main (){       for(i = 0;i < 3;) i = i + 1; return i;}"
    try 4 "main (){i = 0; for(     ;i < 3; i = i + 1) i = i + 1; return i;}"
    try 9 "main (){n = 0; for( ; ; ) if (n == 9) return n; else n = n + 1;}"
    ;;

  11 )
    msg "ブロックのテスト"
    try   6 "main(){i = 0; sum = 0; while(i <= 3){sum = sum + i; i = i + 1;} return sum;}"
    try 120 "main(){pro = 1; for(i = 1; i <= 5; i = i + 1){pro = pro * i; } return pro;}"
    try   9 "main(){n = 0; for( ; ; ) {if (n == 9) return n; n = n + 1;}}"
    try   1 "main(){a = 0; if(a == 0){} else return 0; return 1;}"
    ;;

  12 )
    msg "外部関数引コールのテスト"
    echo "int ret3(){return 3;} int ret120(){return 120;}" | gcc -xc -c -o tmp2.o -
    try   3 "main () { return ret3();}"
    try 120 "main () { return ret120();}"
    rm -f tmp2.o
    echo "int x2(int a){return 2*a;} int add(int a, int b, int c){return a+b+c;}" | gcc -xc -c -o tmp2.o -
    try 10 "main() { n = 5; ret = x2(n); return ret;}"
    try 14 "main() { n = 7; ret = x2(n); return ret;}"
    try  6 "main() { return add(2, 3, 1);}"
    try  5 "main() { return add(3, 3, -1);}"
    rm -f tmp2.o
    ;;

  13 )
    msg "関数のテスト"
    try  1 "main(){return ret1();} ret1(){return 1;}"
    try 30 "main(){return ret30();} ret30(){return x2(15);} x2(a){return 2*a;}"
    try 10 "x2(n){return n + n;} main(){return x2(5);}"
    try 24 "main(){n=4; return fac(n);} fac(n){ if(n==1){return 1;} return n * fac(n-1); }"
    try 55 "main(){return fib(10);} fib(n){ if(n<=1){return n;}return fib(n-2)+fib(n-1); }"
    ;;

  * )
    exit 0
    ;;

  esac

  echo -e "\nOK"
}

for i in {1..100} ; do
  testcase $i
done

