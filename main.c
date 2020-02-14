#include "9cc.h"

Token *token;     // トークン
char *user_input; // 入力プログラム(argv)
Node *node;       // 計算ノード
LVar *locals;     // ローカル変数

int main(int argc, char **argv){

  if(argc != 2){
    error("Usage: ./9cc code\n");
    return 1;
  }

  // トークナイズ & 構文解析．
  user_input = argv[1];
  token = tokenize();
  node = program();

  // アセンブリ生成(標準出力)
  codegen(node);

  return 0;
}

