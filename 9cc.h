#ifndef _9CC_H_
#define _9CC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

/*
 * トークナイズ用トークン
 */
typedef enum{
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // EOFトークン
} TokenKind;

typedef struct Token Token;
struct Token{
  TokenKind kind; // トークンの種類
  Token *next;    // 次のトークン
  int val;        // kind=TK_NUMの時の数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

/*
 * 型
 */
typedef enum{
  TP_INT, // int
  TP_PTR, // ポインタ
} TypeKind;

typedef struct Type Type;
struct Type{
  TypeKind kind;
  Type *base;
};

/*
 * 変数
 */
typedef struct Var Var;
struct Var{
  char *name; // 変数名
  int len;    // 変数名の長さ
  int offset; // オフセット
};

// 変数リスト
typedef struct VarList VarList;
struct VarList{
  VarList *next;
  Var *var;
};

/*
 * ノード
 */
typedef enum{
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_VAR,       // 変数
  ND_NUM,       // 数値
  ND_ASSIGN,    // 代入
  ND_EXPR_STMT, // 式
  ND_FUNCALL,   // 関数
  ND_BLOCK,     // ブロック
  ND_IF,        // if
  ND_WHILE,     // while
  ND_FOR,       // for
  ND_RETURN,    // return
  ND_ADDR,      // アドレス
  ND_DEREF,     // アドレス参照
} NodeKind;

typedef struct Node Node;
struct Node{
  NodeKind kind;    // ノードの種類
  Node *next;
  Node *unary;      // 単項
  Node *lhs;        // 左辺
  Node *rhs;        // 右辺
  Node *cond;       // if, while, for文の条件式
  Node *then;       // if, while, for文の条件式が真のとき
  Node *if_else;    // if文の条件式が偽のとき
  Node *for_init;   // for文の初期化
  Node *for_update; // for文の更新
  Node *block;      // {}の中の複数の式のリスト
  Node *args;       // 関数の引数
  Var *var;         // kind=ND_VARの時の変数
  Type *type;       // 型
  int val;          // kind=ND_NUMの時の数値
  int offset;       // kind=ND_VARの時のベースポインタからのオフセット
  char *funcname;   // 関数名
};

/*
 * 関数
 */
typedef struct Func Func;
struct Func{
  Func *next;
  char *name;
  VarList *params;
  Node *node;
  VarList *locals;
  int stack_size;
};



extern Token *token;     // 現在のトークン
extern char *user_input; // 入力プログラム(argv)

/*
 * デバッグ
 */

// エラー用関数(使い方はprintfと同じ)
// 第一引数に文字列のポインタを指定すると，その箇所を指摘する矢印を出力する．
void error_at(char *loc, char *fmt, ...);

// エラー用関数(使い方はprintfと同じ)
void error(char *fmt, ...);

/*
 * トークナイザ
 */

// 次のトークンで引数の単語が存在するか否かを返す．
// もし存在すれば，トークンを1つ進める．
bool consume(char *op);

// 次のトークンの単語が変数であれば真を返す．
Token *consume_ident();

// 次のトークンが引数の単語でなければならない場合に用いる．
// もし存在すれば，トークンを1つ進める．
// 存在しないなら，エラーを出してプログラムを終了する．
void expect(char *op);

// 次のトークンの単語が数字であればその数値を返す．
int expect_number();

// 次のトークンが識別子であればその名前を返す．
char *expect_ident();

// トークン列の最後か否かを返す．
bool at_eof();

// 今の文字が第2引数と等しいかを確認する．
bool check_symbol(char *p, char *q);

// トークナイズを行う．
Token *tokenize();

/*
 * 型指定
 */

// 型を指定する．
void add_type(Func *func);

/*
 * 構文解析パーサ
 */

// BNF記法による数式の構文解析を行う．
// 構文解析結果を木構造として持ち，そのノードを返す．
Func *program();

/*
 * アセンブリ出力
 */

// ノードを基に，アセンブリを生成する
void codegen(Func *func);

#endif
