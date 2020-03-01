#include "9cc.h"

Token *token;     // トークン
char *user_input; // 入力プログラム(argv)

int main(int argc, char **argv){

  if(argc != 2){
    error("Usage: ./9cc code\n");
    exit(1);
  }

  user_input = argv[1];
  token = tokenize();
  codegen(program());

  return 0;
}

