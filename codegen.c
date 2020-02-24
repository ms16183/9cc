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

  static int label_num = 0;
  int label_num_tmp;

  switch(node->kind){
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      generate_lval(node);
      load();
      return;
    case ND_EXPR_STMT:
      generate(node->lhs);
      printf("  add rsp, 8\n");
      return;
    case ND_FUNCALL:
      printf("  call %s\n", node->funcname);
      printf("  push rax\n");
      return;
    case ND_BLOCK:
      for(Node *n = node->block; n; n = n->next){
        generate(n);
      }
      return;
    case ND_ASSIGN:
      generate_lval(node->lhs);
      generate(node->rhs);
      store();
      return;
    case ND_IF:
      label_num_tmp = label_num;
      label_num++;

      generate(node->cond); // 条件式
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");

      // elseがない場合
      if(!node->if_else){
        printf("  je .Lend%03d\n", label_num_tmp);
        generate(node->then); // 条件式が真の場合
        printf(".Lend%03d:\n", label_num_tmp);
      }
      else{
        printf("  je .Lelse%03d\n", label_num_tmp);
        generate(node->then); // 条件式が真の場合
        printf("  jmp .Lend%03d\n", label_num_tmp);
        printf(".Lelse%03d:\n", label_num_tmp);
        generate(node->if_else); // 条件式が偽の場合
        printf(".Lend%03d:\n", label_num_tmp);
      }
      return;
    case ND_WHILE:
      label_num_tmp = label_num;
      label_num++;

      printf(".Lbegin%03d:\n", label_num_tmp);
      generate(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%03d\n", label_num_tmp);
      generate(node->then);
      printf("  jmp .Lbegin%03d\n", label_num_tmp);
      printf(".Lend%03d:\n", label_num_tmp);
      return;
    case ND_FOR:
      label_num_tmp = label_num;
      label_num++;

      if(node->for_init){
        generate(node->for_init);
      }

      printf(".Lbegin%03d:\n", label_num_tmp);
      if(node->cond){
        generate(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", label_num_tmp);
      }
      generate(node->then);
      if(node->for_update){
        generate(node->for_update);
      }
      printf("  jmp .Lbegin%03d\n", label_num_tmp);
      printf(".Lend%03d:\n", label_num_tmp);
      return;
    case ND_RETURN:
      generate(node->lhs);
      printf("  jmp .Lreturn\n");
      return;
    default:
      break;
  }

  // 二項演算子

  generate(node->lhs);
  generate(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->kind){
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");      // raxの64bitを128bitに伸ばしている．
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    default:
      break;
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

  // ローカル変数の数をカウントし，rbpからrspの間の確保を行う．
  int lvar_num = 0;
  for(LVar *var = locals; var; var = var->next){
    lvar_num++;
  }

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", lvar_num * 8);

  // ターミネータで区切って計算する．
  for(Node *n = node; n; n = n->next){
    generate(n);
  }
  printf(".Lreturn:\n");
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return;
}

