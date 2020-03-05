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
    gcc -o tmp tmp.s tmp2.o
  else
    gcc -o tmp tmp.s
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
    try   2 "int main(){return 1+1;}"
    try 100 "int main(){return 20-10+120-5-5-30+10;}"
    try  24 "int main(){return   5 - 2+4  +3 -1+4 + 12-     1;}"
    try   1 "int main(){return 2+   3 - 3-1  ;}"
    ;;

  2 )
    msg "四則演算のテスト"
    try 90 "int main() { return 50+25+5*(10/2)-10; }"
    try  6 "int main() { return 2*3; }"
    try  2 "int main() { return 1+5/3; }"
    try  3 "int main() { return  ( 10* 2 +   1 -3) / 6; }"
    try  2 "int main() { return 20 *2/ (4*(2+3)); }"
    ;;

  3 )
    msg "単項演算子のテスト"
    try  1 "int main(){return +1;}"
    try  1 "int main(){return -1*-1;}"
    try  1 "int main(){return ++++1;}"
    try  1 "int main(){return ----1;}"
    try  1 "int main(){return -2* 2 + +5;}"
    try 10 "int main(){return +10/-2+-7-(-2)++20;}"
    try  3 "int main(){return 2 --1;}"
    ;;

  4 )
    msg "比較演算子のテスト"
    try 1 "int main(){return 100==100;}"
    try 0 "int main(){return 100==0;}"
    try 1 "int main(){return 100!=0;}"
    try 0 "int main(){return 100!=100;}"
    try 1 "int main(){return 100>0;}"
    try 0 "int main(){return 100<0;}"
    try 0 "int main(){return 100>100;}"
    try 0 "int main(){return 100<100;}"

    try 1 "int main(){return 100>=0;}"
    try 0 "int main(){return 100<=0;}"
    try 1 "int main(){return 100>=100;}"
    try 1 "int main(){return 100<=100;}"

    try 1 "int main(){return 1+1>0;}"
    try 2 "int main(){return 1+(1>0);}"
    try 0 "int main(){return (1+1)<0;}"

    try 1 "int main(){return 1<2 == 3<4;}"
    try 1 "int main(){return 1>2 != 3<4;}"

    try 1 "int main(){return (2+10)*2==24;}"
    ;;

  5 )
    msg "変数代入のテスト"
    try   0 "int main(){int a=0; return a;}"
    try 100 "int main(){int _b=100; return _b;}"
    try 100 "int main(){int c1=100; return c1;}"
    try 100 "int main(){int _a1b2_ = 100; return _a1b2_;}"
    try  10 "int main(){int z=2; return z*(z*z)+z;}"
    try   1 "int main(){int hoge = 2; int foo = 1 + hoge; return foo - hoge;}"
    try   6 "int main(){int a=1; int b=2; int c=3; return c + a + b;}"
    try   6 "int main(){int I = 1; int love = 2; int you = 3; return I*love*you;}"
    try   1 "int main(){int greater = 100; int less = 20; return greater > less;}"
    ;;

  6 )
    msg "return文のテスト"
    try 6 "int main(){return 1+2+3;}"
    try 2 "int main(){int hoge=2; return hoge;}"
    try 1 "int main(){return 1; 2; 3;}"
    try 5 "int main(){4; return 5; 6;}"
    try 9 "int main(){7; 8; return 9;}"
    try 0 "int main(){return 0; return 1;}"
    ;;

  7 )
    msg "if文のテスト"
    try  3 "int main(){if(1+2 == 3) return 3; return 2;}"
    try  2 "int main(){if(1*2 == 3) return 3; return 2;}"
    try  1 "int main(){if ( 0 ) return 100; return 2-1;}"
    try  3 "int main(){int foo=3; if(5-foo == 5-(2+1))   return foo; return 0;}"
    try  0 "int main(){int foo=3; if(5-foo != 5-(2+1))   return foo; return 0;}"
    try  2 "int main(){int abc = 5; int hoge = 2; if ( 0 ) hoge = abc * 3; return hoge;}"
    try 15 "int main(){int abc = 5; int hoge = 2; if ( 1 ) hoge = abc * 3; return hoge;}"
    try  4 "int main(){int num=1; if(num == 1) if(num == 1) num = num*2; num = num*2; return num;}"
    try  2 "int main(){int num=1; if(num == 1) if(num != 1) num = num*2; num = num*2; return num;}"
    try  2 "int main(){int num=1; if(num != 1) if(num == 1) num = num*2; num = num*2; return num;}"
    try  2 "int main(){int num=1; if(num != 1) if(num != 1) num = num*2; num = num*2; return num;}"
    ;;

  8 )
    msg "if else文のテスト"
    try 5 "int main(){int hoge = 2; if(hoge == 2) hoge = 5; else hoge = 0; return hoge;}"
    try 0 "int main(){int hoge = 2; if(hoge != 2) hoge = 5; else hoge = 0; return hoge;}"
    try 1 "int main(){int a = 1; int b = a; int out = 0; if(a == 1) if(b == 1) out = 1; else out = 2; else out = 3; return out;}"
    try 3 "int main(){int a = 1; int b = a; int out = 0; if(a != 1) if(b == 1) out = 1; else out = 2; else out = 3; return out;}"
    try 2 "int main(){int a = 1; int b = a; int out = 0; if(a == 1) if(b != 1) out = 1; else out = 2; else out = 3; return out;}"
    try 3 "int main(){int a = 1; int b = a; int out = 0; if(a != 1) if(b != 1) out = 1; else out = 2; else out = 3; return out;}"
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
    try   3 "int main(){int i = 1; while(i < 3) i = i + 1; return i;}"
    try 200 "int main(){int i = 1; while(i < 100) if (i == 10) i = 200; else i = i + 1; return i;}"
    try   1 "int main(){int i = 1; while (0) i = i + 1; return i;}"
    try 100 "int main(){int i = 1; while ( 1 ) if (i == 100) return i; else i = i + 1;}"
    ;;

  10 )
    msg "for文のテスト"
    try 6 "int main (){int sum = 0; int i; for(i = 1; i <= 3; i = i + 1) sum = sum + i; return sum;}"
    try 0 "int main (){int sum = 5050; int i; for(i = 100; i > 0; i = i - 1) sum = sum - i; return sum;}"
    try 3 "int main (){int i = 0; for(     ; i < 3;) i = i + 1; return i;}"
    try 3 "int main (){int i = 1; for(i = 0;i < 3;) i = i + 1; return i;}"
    try 4 "int main (){int i = 0; for(     ;i < 3; i = i + 1) i = i + 1; return i;}"
    try 9 "int main (){int n = 0; for( ; ; ) if (n == 9) return n; else n = n + 1;}"
    ;;

  11 )
    msg "ブロックのテスト"
    try   6 "int main(){int i = 0; int sum = 0; while(i <= 3){sum = sum + i; i = i + 1;} return sum;}"
    try 120 "int main(){int pro = 1; int i = 1; for(i = 1; i <= 5; i = i + 1){pro = pro * i; } return pro;}"
    try   9 "int main(){int n = 0; for( ; ; ) {if (n == 9) return n; n = n + 1;}}"
    try   1 "int main(){int a = 0; if(a == 0){} else return 0; return 1;}"
    ;;

  12 )
    msg "外部関数引コールのテスト"
    echo "int ret3(){return 3;} int ret120(){return 120;}" | gcc -xc -c -o tmp2.o -
    try   3 "int main () { return ret3();}"
    try 120 "int main () { return ret120();}"
    rm -f tmp2.o
    echo "int x2(int a){return 2*a;} int add(int a, int b, int c){return a+b+c;}" | gcc -xc -c -o tmp2.o -
    try 10 "int main() { int n = 5; int ret = x2(n); return ret;}"
    try 14 "int main() { int n = 7; int ret = x2(n); return ret;}"
    try  6 "int main() { return add(2, 3, 1);}"
    try  5 "int main() { return add(3, 3, -1);}"
    rm -f tmp2.o
    ;;

  13 )
    msg "関数のテスト"
    try  1 "int main(){return ret1();} int ret1(){return 1;}"
    try  1 "int main(){int i=1;int j=f();return i;} int f(){int i=2;return i;}"
    try 30 "int main(){return ret30();} int ret30(){return x2(15);} int x2(int a){return 2*a;}"
    try 10 "int x2(int n){return n + n;} int main(){return x2(5);}"
    try 24 "int add(int a, int b, int c){return a+b+c;} int main(){return add(5+5, 10-3, 3+3+1);}"
    try 24 "int main(){int n=4; return fac(n);} int fac(int n){ if(n==1){return 1;} return n * fac(n-1); }"
    try 55 "int main(){return fib(10);} int fib(int n){ if(n<=1){return n;}return fib(n-2)+fib(n-1); }"
    ;;

  14)
    msg "ポインタのテスト"
    try 1 "int main(){int x = 1; int *addr = &x; return *addr;}"
    try 2 "int main(){int s1 = 1; int s2 = 2; int s3 = 3; int *addr = &s2 + 0; return *addr;}"
    try 3 "int main(){int s1 = 1; int s2 = 2; int s3 = 3; int *addr = &s2 + 1; return *addr;}"
    try 1 "int main(){int s1 = 1; int s2 = 2; int s3 = 3; int *addr = &s2 - 1; return *addr;}"
    try 1 "int main(){int x=1;int *y=&x;int *z=&y; return **z; }"
    try 2 "int main(){int x=1;int y=2; return *(&x+1);}"
    try 1 "int main(){int x=1;int y=2; return *(&y-1);}"
    try 2 "int main(){int x=1;int *y=&x; *y=2; return x;}"
    try 4 "int main(){int x=1;int y=2; *(&x+1)=4; return y;}"
    try 3 "int main(){int x=1;int y=2; *(&y-1)=3; return x;}"
    try 6 "int main(){int a=3; x2(&a); return a;} int x2(int *n){*n = 2*(*n);return 0;}"
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

