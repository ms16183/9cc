#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

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

Token *token;
char *user_input; // 入力プログラム

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

    if(*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

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

int main(int argc, char **argv){

  if(argc != 2){
    fprintf(stderr, "引数の個数が正しくありません．\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("\n");
  printf("main:\n");

  // 最初は数値である．
  printf("  mov rax, %d\n", expect_number());

  // '+ 数値'或いは'- 数値'というトークンを読み取る．
  while(!at_eof()){
    // +かな?
    if(consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    // +じゃなければ当然-だよね?
    expect('-');
    printf("  sub rax, %d\n", expect_number());

    /*
    // このような書き方もできる．
    if(consume('-')){
      printf("  sub rax, %d\n", expect_number());
      continue;
    }
    error(token->str, "数ではありません．");
    */
  }

  printf("  ret\n");
  return 0;
}

