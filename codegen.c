#include "9cc.h"

int label_num = 0;

char *funcname;
const char *reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9",
                     "r10", "r11", "r12", "r13", "r14", "r15"};
const int reglen = (int)sizeof(reg)/sizeof(char*);

void generate_val(Node *node){
  if(node->kind != ND_VAR){
    error("変数ではありません．\n");
  }

  printf("  lea rax, [rbp-%d]\n", node->var->offset);
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

  int label_num_tmp;
  int nargs = 0;

  switch(node->kind){
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_VAR:
      generate_val(node);
      load();
      return;
    case ND_EXPR_STMT:
      generate(node->lhs);
      printf("  add rsp, 8\n");
      return;
    case ND_FUNCALL:
      nargs = 0;
      for(Node *a = node->args; a; a = a->next){
        generate(a);
        nargs++;
      }

      for(int i = nargs-1; i >= 0 && i < reglen; i--){
        printf("  pop %s\n", reg[i]);
      }
      printf("  call _%s\n", node->funcname);
      printf("  push rax\n");
      return;
    case ND_BLOCK:
      for(Node *n = node->block; n; n = n->next){
        generate(n);
      }
      return;
    case ND_ASSIGN:
      generate_val(node->lhs);
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
      printf("  pop rax\n");
      printf("  jmp .Lreturn._%s\n", funcname);
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
      printf("  movzx rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzx rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzx rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzx rax, al\n");
      break;
    default:
      break;
  }

  printf("  push rax\n");
  return;
}

void codegen(Func *func){

  printf(".intel_syntax noprefix\n");

  // 関数
  for(Func *f=func; f; f=f->next){

    // スタック確保
    int offset = 0;
    for(VarList *vl = f->locals; vl; vl = vl->next){
      offset += 8;
      vl->var->offset = offset;
    }
    f->stack_size = offset;

    // 宣言
    funcname = f->name;
    printf(".global _%s\n", funcname);
    printf("_%s:\n", funcname);

    // スタック移動
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", f->stack_size);

    // 変数設定
    int i = 0;
    for(VarList *vl = f->params; vl && i < reglen; vl = vl->next, i++){
      Var *var = vl->var;
      printf(" mov [rbp-%d], %s\n", var->offset, reg[i]);
    }

    // ターミネータで区切られたノードを計算する．
    for(Node *n = f->node; n; n = n->next){
      generate(n);
    }

    printf(".Lreturn._%s:\n", funcname);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}

