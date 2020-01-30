#include "9cc.h"

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

