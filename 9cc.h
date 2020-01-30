#ifndef _9CC_H_
#define _9CC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

typedef enum{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 数値
} NodeKind;

typedef struct Node Node;
struct Node{
  NodeKind kind; // ノードの種類
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kind=ND_NUMの時の数値
};

typedef enum{
  TK_RESERVED, // 記号
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

Token *token;     // 現在のトークン
char *user_input; // 入力プログラム(argv)
Node *node;       // 計算ノード

// エラー用関数(使い方はprintfと同じ)
void error(char *loc, char *fmt, ...);

/*
 * トークナイザ
 */

// 次のトークンが期待される記号であれば真を返す．
bool consume(char *op);

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
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/*
 * アセンブリ出力
 */
void begin();
void generate(Node *node);
void end();
// アセンブリ生成
void generator(Node *node);

#endif
