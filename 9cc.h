#ifndef _9CC_H_
#define _9CC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

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

typedef enum{
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_LVAR,      // ローカル変数
  ND_NUM,       // 数値
  ND_ASSIGN,    // 代入
  ND_EXPR_STMT, // 式
  ND_IF,        // if
  ND_WHILE,     // while
  ND_RETURN,    // return
} NodeKind;

typedef struct Node Node;
struct Node{
  NodeKind kind; // ノードの種類
  Node *next;
  Node *lhs;        // 左辺
  Node *rhs;        // 右辺
  Node *if_cond;    // kind=ND_IFにおいて，if文の条件式
  Node *if_true;    // kind=ND_IFにおいて，if文の条件式が真のとき
  Node *if_false;   // kind=ND_IFにおいて，if文の条件式が偽のとき
  Node *while_cond; // kind=ND_WHILEにおいて，while文の条件式
  Node *while_true; // kind=ND_WHILEにおいて，while文の条件式が真のとき
  int val;          // kind=ND_NUMの時の数値
  char name;        // kind=ND_LVARの時の変数名
  int offset;       // kind=ND_LVARの時のベースポインタからのオフセット
};

typedef struct LVar LVar;
struct LVar{
  LVar *next; // リスト
  char *name; // 変数名
  int len;    // 変数名の長さ
  int offset; // rbpからのオフセット
};

extern Token *token;     // 現在のトークン
extern char *user_input; // 入力プログラム(argv)
extern Node *node;       // 計算ノード
extern LVar *locals;     // ローカル変数

/*
 * デバッグ
 */

// ポインタ指定エラー用関数(使い方はprintfと同じ)
void error_at(char *loc, char *fmt, ...);

// エラー用関数(使い方はprintfと同じ)
void error(char *fmt, ...);

/*
 * トークナイザ
 */

// 次のトークンが期待される記号であれば真を返す．
bool consume(char *op);

// 次のトークンが変数であれば真を返す．
Token *consume_ident();

// 次のトークンが期待される記号であれば進める．
// consumeとの違いは，エラーを出すかどうかである．
void expect(char *op);

// 次のトークンが数字であればその数値を返す．
int expect_number();

// トークンがこれ以上続かないなら真を返す．
bool at_eof();

// 次のトークンを生成する．
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

// 入力されたプログラムの記号が正しいか確認する．
bool check_symbol(char *p, char *q);

// ローカル変数でその名前が以前使われたか判別する．
// 見つかった場合，そのローカル変数のリストのポインタが返却される．
LVar *find_lvar(Token *token);

// トークナイザ
Token *tokenize();

/*
 * 構文解析パーサ
 */

// 次のノードを生成する．
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

// 次の数値ノードを生成する．
Node *new_node_num(int val);

// BNF記法による数式の構文解析
Node *program();

/*
 * アセンブリ出力
 */

// アセンブリ生成
void codegen(Node *node);

#endif
