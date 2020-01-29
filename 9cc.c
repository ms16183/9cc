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
void error(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%d\n", pos);
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  // pos個の空白を出力する．
  fprintf(stderr, "^ ");            // 指摘箇所
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/*
 * トークナイザ
 */

// 次のトークンが期待される記号であれば真を返す．
bool consume(char *op){
  if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待される記号であれば進める．
// consumeとの違いは，エラーを出すかどうかである．
void expect(char *op){
  if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    error(token->str, "'%s'ではありません．\n", op);
  }
  token = token->next;
}

// 次のトークンが数字であればその数値を返す．
int expect_number(){
  if(token->kind != TK_NUM){
    error(token->str, "数ではありません．");
  }
  int val = token->val;
  token = token->next;
  return val;
}

// トークンがこれ以上続かないなら真を返す．
bool at_eof(){
  return token->kind == TK_EOF;
}

// 次のトークンを生成する．
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  Token *new = (Token*)calloc(1, sizeof(Token));
  new->kind = kind;
  new->str = str;
  new->len = len;
  cur->next = new;
  return new;
}

// 入力されたプログラムの記号が正しいか確認する．
bool check_symbol(char *p, char *q){
  return !memcmp(p, q, strlen(q));
}

Token *tokenize(){

  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){

    // 空白を無視する．
    if(isspace(*p)){
      p++;
      continue;
    }

    // 四則演算
    if(check_symbol(p, "+") || check_symbol(p, "-") ||
       check_symbol(p, "*") || check_symbol(p, "/") ||
       check_symbol(p, "(") || check_symbol(p, ")") ){
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    // 比較演算子
    if(check_symbol(p, "==") || check_symbol(p, "!=") ||
       check_symbol(p, ">=") || check_symbol(p, "<=")){
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
    }
    // >=や<=と1文字目が被るので>=，<=より後に記述すること．
    if(check_symbol(p, ">") || check_symbol(p, "<")){
        cur = new_token(TK_RESERVED, cur, p, 1);
        p++;
        continue;
    }

    // 数値
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    // それ以外
    error(p, "トークナイズできない文字です．");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

/*
 * 構文解析パーサ
 */

// 次のノードを生成する．
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = kind;
  new->lhs = lhs;
  new->rhs = rhs;
  return new;
}

// 次の数値ノードを生成する．
Node *new_node_num(int val){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_NUM;
  new->val = val;
  return new;
}

// BNF記法による数式の構文解析
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *expr(){
  return equality();
}

Node *equality(){
  Node *node = relational();
  while(true){
    if(consume("==")){
      node = new_node(ND_EQ, node, relational());
    }
    else if(consume("!=")){
      node = new_node(ND_NE, node, relational());
    }
    else{
      return node;
    }
  }
}

Node *relational(){
  Node *node = add();
  while(true){
    if(consume("<=")){
      node = new_node(ND_LE, node, add());
    }
    else if(consume("<")){
      node = new_node(ND_LT, node, add());
    }
    else if(consume(">=")){
      // GEだが，左辺と右辺を入れ替えればLEと等価である．
      node = new_node(ND_LE, add(), node);
    }
    else if(consume(">")){
      // GTだが，左辺と右辺を入れ替えればLTと等価である．
      node = new_node(ND_LT, add(), node);
    }
    else{
      return node;
    }
  }
}

Node *add(){
  Node *node = mul();

  while(true){
    if(consume("+")){
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume("-")){
      node = new_node(ND_SUB, node, mul());
    }
    else{
      return node;
    }
  }
}

Node *mul(){
  Node *node = unary();

  while(true){
    if(consume("*")){
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume("/")){
      node = new_node(ND_DIV, node, unary());
    }
    else{
      return node;
    }
  }
}

Node *unary(){
  if(consume("+")){
    return unary();
  }
  if(consume("-")){
    // -a = 0 - a
    Node *zero = new_node_num(0);
    return new_node(ND_SUB, zero, unary());
  }
  return primary();
}

Node *primary(){

  if(consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}

void begin(){
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("\n");
  printf("main:\n");
  return;
}

void generate(Node *node){

  if(node->kind == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  generate(node->lhs);
  generate(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  if(node->kind == ND_ADD){
    printf("  add rax, rdi\n");
  }
  if(node->kind == ND_SUB){
    printf("  sub rax, rdi\n");
  }
  if(node->kind == ND_MUL){
    printf("  imul rax, rdi\n");
  }
  if(node->kind == ND_DIV){
    printf("  cqo\n");      // raxの64bitを128bitに伸ばしている．
    printf("  idiv rdi\n");
  }
  if(node->kind == ND_EQ){
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
  }
  if(node->kind == ND_NE){
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
  }
  if(node->kind == ND_LT){
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
  }
  if(node->kind == ND_LE){
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
  }

  // スタックトップに計算結果を置く．
  printf("  push rax\n");
  return;
}

void end(){
  // スタックトップにある，数式の計算結果をraxにpopして出力とする．
  printf("  pop rax\n");
  printf("  ret \n");
  return;
}

void generator(Node *node){
  begin();
  generate(node);
  end();
  return;
}

int main(int argc, char **argv){

  if(argc != 2){
    fprintf(stderr, "Usage: ./9cc code\n");
    return 1;
  }

  // トークナイズする．
  user_input = argv[1];
  token = tokenize();

  // 四則演算(+-*/())を解析して計算する．
  node = expr();

  // アセンブリ生成(標準出力)
  generator(node);

  return 0;
}

