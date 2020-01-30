#include "9cc.h"

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

