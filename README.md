# 9cc C compiler

[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)を参考にします．

サイト著者のリポジトリは[rui314/9cc](https://github.com/rui314/9cc)です．

# 環境
- WSL Ubuntu 18.04 LTS
- gcc (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0

# 学習事項

## トークナイザ
トークナイザは入力された文字列をトークンという，プログラムの動作として意味のある単位に分割する．

例えば

```
hoge = 10 + 2*4 - (9/3);
```

をトークナイズすると，

```
hoge, =, 10, +, 2, *, 4, -, (, 9, /, 3, ), ;
```

となる(空白は無視)．次に，`hoge`は変数，`10`や`2`は数値，`=`は代入を行う記号，`+`や`*`は二項演算子，`;`は終端記号と識別子を付与する．

## 構文解析器
構文解析器は，トークンを基に，どのように計算すればよいのかを解析する．先ほどの例を構文解析器で解析すると，

```
hoge = 10 + 2*4 - (9/3);
```

1. `9/3`を計算する．結果は`3`．
1. `2*4`を計算する．結果は`8`．
1. `8-3`を計算する．結果は`5`．
1. `10+5`を計算する．結果は`15`．
1. `=`により，`hoge`に代入を行う．
1. `;`によりこの式の計算を終了する．

のように計算順序を作成し，リストや配列に保存する．構文解析の方法としては文脈自由文法の1つである，Backs-Naur form(BNF)を用いている．

## BNFによる数式生成

- `<var>` 変数`var`
- `<A> ::= <B>` `A`を`B`と定義する．
- `|` 又は
- `()` 括弧
- `"str"` 文字列`str`
- `*` 直前の文字や変数の0回以上の繰り返し．
- `?` 直前の文字や変数が0回或いは1回出現する．

 ```
<function> ::= <type> <ident> "(" <params>? ")" "{" <stmt>* "}"
<params>   ::= <type> <ident> ("," <type> <ident>)*

<stmt> ::=   "{" <stmt>* "}"
           | "if" "(" <expr> ")" <stmt> ("else" <stmt>)?
           | "while" "(" <expr> ")" <stmt>
           | "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <stmt>
           | "return" <expr> ";"
           | <declaration> ";"
           | <expr> ";"

<declaration> ::= <type> <ident>("=" <expr>)? ("," <ident>("=" <expr>)?)*
<expr>        ::= <assign>
<assign>      ::= <equality> ("=" <assign>)?
<equality>    ::= <relational> ( "==" <relational> | "!=" <relational>)*
<relational>  ::= <add> ("<" <add> | ">" <add> | "<=" <add> | ">= <add>)*
<add>         ::= <mul> ("+" <mul> | "-" <mul>)*
<mul>         ::= <unary> ("+" <unary> | "-" <unary>)*
<unary>       ::= <primary> | ("+" | "-" | "&" | "*")? <unary>
<primary>     ::= "(" <expr> ")" | <ident> <func-args>? | <num>
<func-args>   ::= "(" (<assign> ("," <assign>)*)? ")"
<num>         ::= <num>? <digit>

<type>     ::= ("int" | "float") "*"?
<ident>    ::= (<alphabet> | "_") (<alphabet> | <digit> | "_")*
<digit>    ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
<alphabet> ::= "a" | "b" | ... | "z" | "A" | "B" | ... | "Z"
```

```
数値が浮動小数点の場合，浮動小数点は
<浮動小数点定義> ::= [<符号>]<小数点定数>[<指数部>]
                    |[<符号>]<数字列><指数部>

<小数点定数> ::= [<数字列>]<.><数字列>|<数字列><.>
<指数部>     ::=<E>[<符号>]<数字列>
<数字列>     ::=<数字>|<数字列><数字>
<数字>       ::= 0|1|2|3|4|5|6|7|8|9
<符号>       ::= +|-

 ```


## コードジェネレータ
コードジェネレータは，構文解析器から得た計算順序を基にアセンブリあるいは機械語をジェネレートする．
今回は，.sという拡張子で，アセンブリを出力する．

最初に，`.intel_syntax noprefix`を宣言する．Intel記法でアセンブリを記述するという意味である．
次に，構文解析器の情報を基に，各関数を生成する．関数の生成には，`.global <関数名>`という形で
宣言すればよい．その次に`<関数名>:`で下に命令を記述する．

関数ではローカル変数や引数を保持したり，関数終了後に戻るアドレスを保持する必要がある．
そのため関数の情報はスタックというデータ構造を用いて管理する．
スタックは関数毎に必要になるため，スタックの底であるrbpをpushし，前の関数のアドレスを保持する．
次に，rspの値をrbpにコピーし，スタックのトップがスタックの底になるように移動する．
最後にrspから変数を保持するために必要な分の領域を減ずる．これはスタックトップが低位の方向に伸びるからである．

反対に，関数が終了する際はrbpの値をrspにコピーし，rbpをポップして元のアドレスに戻る．なお，関数の戻り値は
raxレジスタに保存されている．

次に，関数で実行される命令を作成する．初めに，2項間で行われる計算を考える．まず，値(整数リテラルや変数の保持する値)はすべてpush命令によってスタックに保持されている．そのためスタックからその値をpopする．pop先は，rdi，raxレジスタである．次にノードの種類を見る．四則演算は簡単である．

- 加算: `add rax, rdi`により`rax+=rdi`が行われる．
- 減算: `sub rax, rdi`により`rax-=rdi`が行われる．
- 乗算: `imul rax, rdi`により`rax*=rdi`が行われる．
- 除算: `idiv rdi`により`rax/=rdi`が行われる．ただしこの命令の前にcqo命令でraxを128bitに伸ばしている．

アセンブリの命令があるため，そのままの実装になる．

続いて，比較演算を考える．手順としては，cmp命令でraxとrdiの値からフラグレジスタを操作する．次にそのフラグレジスタの状態から比較が正しいかをalレジスタにコピーする．

- `cmp rax, rdi`を行い，フラグレジスタを操作する．
- `==`: `sete al`でフラグを確認し，cmp命令でrax, rdiの値が等しいかの真偽値をalにコピーする．
- `!=`: `setne al`で同様のことを行う．
- `<=`: `setle al`で同様のことを行う．
- `<`: `setl al`で同様のことを行う．
- `>=`, `>`: 左辺と右辺を交換すればそれぞれ`<=`，`<`と同じである．

最後にraxをpushする．

if, while, forの様な構文も，比較命令とjmp命令で記述できる．

if文では，if文の条件式ノードをアセンブリで記述する．その結果は，前述の四則演算や比較演算の結果である．raxがpushされているので，pop命令でraxをraxに取り出す．次に，cmp命令でそのraxの値と0を比較し条件式が真か否かを確かめる．

```
// condが真ならばそのまま実行する．
// condが偽ならLabelに飛ぶ．
if(cond){
  ...
}
Label:
```

```
// condが真ならばそのまま実行し，自動でelse文を飛ばすLabel1に飛ぶ．
// condが偽ならば自動でelse文の先頭Label2に飛び，そのまま実行する．
if(cond){
  ...
  Label1へ移動する．
}
else{
  Label2:
  ...
}
Label1:
```

while文もif文と同様に条件式ノードをアセンブリで記述する．

```
Label1:
// condが偽ならばループを抜けるためLabel2に飛ぶ．
while(cond){
  ...

// 再度condの結果を知るためLabel1に飛ぶ．
}
Label2:
```

for文も同様である．

```
for( init; cond; update){
  ...
}

↓
init
while(cond){
  ...
  update;
}

```

と等価であるため同様に記述できる．ただし，for文の初期化，条件式，更新は省略できるので
トークナイズ時に各式が省略されているか確認し，場合に応じて式をアセンブリにする．

変数を考える．変数はスタックに確保されているので，トークナイズ時のオフセット計算の値を
用いて，rbpから引き算を行う．その後にlea命令でraxにその変数のアドレスを代入し，
pushする．次に，ロードを行う．raxにpopして，そのアドレスの値をraxに代入する．
それをpushする．つまりスタックにあるアドレスをアドレスのさす値に変換している．

例えば

```
int main(){int a=3; x2(&a); return a;} int x2(int *n){*n = 2*(*n);return 0;}
```

をアセンブリにする．

```
.intel_syntax noprefix

.global main

main:
  push rbp
  mov rbp, rsp
  sub rsp, 8
  lea rax, [rbp-8]
  push rax
  push 3
  pop rdi
  pop rax
  mov [rax],  rdi
  push rdi
  add rsp, 8
  lea rax, [rbp-8]
  push rax
  pop rdi
  call x2
  push rax
  add rsp, 8
  lea rax, [rbp-8]
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rax
  jmp .Lreturn.main
.Lreturn.main:
  mov rsp, rbp
  pop rbp
  ret

.global x2

x2:
  push rbp
  mov rbp, rsp
  sub rsp, 8
 mov [rbp-8], rdi
  lea rax, [rbp-8]
  push rax
  pop rax
  mov rax, [rax]
  push rax
  push 2
  lea rax, [rbp-8]
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rax
  mov rax, [rax]
  push rax
  pop rdi
  pop rax
  imul rax, rdi
  push rax
  pop rdi
  pop rax
  mov [rax],  rdi
  push rdi
  add rsp, 8
  push 0
  pop rax
  jmp .Lreturn.x2
.Lreturn.x2:
  mov rsp, rbp
  pop rbp
  ret
```

最初の行は，`.intel_syntax`ディレクティブによって，Intel形式で記述することを表している．
これは必須である．

次に，`.global <関数名>`で関数を宣言している．

次に，main関数では

```
push rbp
mov rbp, rsp
sub rsp 8
```

のように，関数の最初でスタックの確保を行っている．変数は1つしか登場していないので8byte確保されている．
次に，宣言された変数を代入する．

```
mov [rbp-8], rdi
```

として，特定のレジスタに確保した変数を保存する．




## 翻訳限界
[C言語の翻訳限界](https://qiita.com/yuki12/items/26994416162b54c811a1)が存在する．
そのため無限のネスト，無限の引数を意識する必要はない．
