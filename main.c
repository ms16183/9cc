#include "9cc.h"

Token *token;     // トークン
char *user_input; // 入力プログラム(argv)
//VarList *locals;  // ローカル変数

int main(int argc, char **argv){

  if(argc != 2){
    error("Usage: ./9cc code\n");
    return 1;
  }
  Func *func;       // 関数

  // トークナイズ & 構文解析．
  user_input = argv[1];
  token = tokenize();
  func = program();

  int offset;
  for(Func *f=func; f; f=f->next){
    offset = 0;
    for(VarList *vl = f->locals; vl; vl=vl->next){
      offset += 8;
      vl->var->offset = offset;
    }
    f->stack_size = offset;
  }

  // アセンブリ生成(標準出力)
  codegen(func);

  return 0;
}

