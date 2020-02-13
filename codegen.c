#include "9cc.h"

void generate_lval(Node *node){
  if(node->kind != ND_LVAR){
    error("左辺値が変数ではありません．\n");
  }

  printf("  lea rax, [rbp-%d]\n", node->offset);
  printf("  push rax\n");
  return;
}

void load(){
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
}

void store(){
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax],  rdi\n");
    printf("  push rdi\n");
    return;
}

void generate(Node *node){

  if(node->kind == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }
  if(node->kind == ND_LVAR){
    generate_lval(node);
    load();
    return;
  }
  if(node->kind == ND_EXPR_STMT){
    generate(node->lhs);
    printf("  add rsp, 8\n");
    return;
  }
  if(node->kind == ND_ASSIGN){
    generate_lval(node->lhs);
    generate(node->rhs);
    store();
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

void codegen(Node *node){
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("\n");
  printf("main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // セミコロンで区切って計算する．
  for(Node *n = node; n; n = n->next){
    generate(n);
  }
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret \n");
  return;
}

