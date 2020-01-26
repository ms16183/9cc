#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

typedef enum{
  ND_ADD, // 四則演算
  ND_SUB,
  ND_MUL,
  ND_DIV,
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
};

Token *token;     // 現在のトークン
char *user_input; // 入力プログラム
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

// 次のトークンが期待される記号であればtrue
bool consume(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op){
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待される記号であれば進める．
// consumeとの違いは，エラーを出すかどうかである．
void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op){
    error(token->str, "'%c'ではありません．\n", op);
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

// トークンがこれ以上続かないならtrue
bool at_eof(){
  return token->kind == TK_EOF;
}

// 次のトークンを生成する．
Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *new = (Token*)calloc(1, sizeof(Token));
  new->kind = kind;
  new->str = str;
  cur->next = new;
  return new;
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

    // 記号
    if(*p == '+' || *p == '-' ||
       *p == '*' || *p == '/' ||
       *p == '(' || *p == ')'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    // 数値
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error(p, "トークナイズできない文字です．");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

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
Node *mul();
Node *primary();

Node *expr(){
  Node *node = mul();

  while(true){
    if(consume('+')){
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume('-')){
      node = new_node(ND_SUB, node, mul());
    }
    else{
      return node;
    }
  }
}

Node *mul(){
  Node *node = primary();

  while(true){
    if(consume('*')){
      node = new_node(ND_MUL, node, primary());
    }
    else if(consume('/')){
      node = new_node(ND_DIV, node, primary());
    }
    else{
      return node;
    }
  }
}

Node *primary(){

  if(consume('(')){
    Node *node = expr();
    expect(')');
    return node;
  }

  return new_node_num(expect_number());
}

void gen(Node *node){

  if(node->kind == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  if(node->kind == ND_ADD){
    printf("  add rax, rdi\n");
  }
  else if(node->kind == ND_SUB){
    printf("  sub rax, rdi\n");
  }
  else if(node->kind == ND_MUL){
    printf("  imul rax, rdi\n");
  }
  else if(node->kind == ND_DIV){
    printf("  cqo\n");      // raxの64bitを128bitに伸ばしている．
    printf("  idiv rdi\n");
  }
  else{
  }

  printf("  push rax\n");
}

int main(int argc, char **argv){

  if(argc != 2){
    fprintf(stderr, "引数の個数が正しくありません．\n");
    return 1;
  }

  // トークナイズする．
  user_input = argv[1];
  token = tokenize();

  // 四則演算(+-*/())を解析して計算する．
  node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("\n");
  printf("main:\n");

  // 構文解析
  gen(node);

  // スタックトップにある，数式の計算結果をraxにpopして出力とする．
  printf("  pop rax\n");
  printf("  ret \n");
  return 0;
}

