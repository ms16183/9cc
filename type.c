#include "9cc.h"

// int型にする．
Type *int_type(){
  Type *new = (Type*)calloc(1, sizeof(Type));
  new->kind = TP_INT;
  return new;
}

// 引数へのポインタにする．
Type *pointer_to(Type *base){
  Type *new = (Type*)calloc(1, sizeof(Type));
  new->kind = TP_PTR;
  new->base = base;
  return new;
}

void visit(Node *node){

  if(!node){
    return;
  }

  // 子ノード全てに対して行う．
  visit(node->unary);
  visit(node->lhs);
  visit(node->rhs);
  visit(node->cond);
  visit(node->then);
  visit(node->if_else);
  visit(node->for_init);
  visit(node->for_update);
  for(Node *n = node->block; n; n = n->next){
    visit(n);
  }
  for(Node *n = node->args; n; n = n->next){
    visit(n);
  }

  switch(node->kind){
    case ND_ADD:
      if(node->rhs->type->kind == TP_PTR){
        Node *tmp = node->lhs;
        node->lhs = node->rhs;
        node->rhs = tmp;
      }
      if(node->rhs->type->kind == TP_PTR){
        error("無効なポインタ");
      }
      node->type = node->lhs->type;
      return;
    case ND_SUB:
      if(node->rhs->type->kind == TP_PTR){
        error("無効なポインタ");
      }
      node->type = node->lhs->type;
      return;
    case ND_ASSIGN:
      node->type = node->lhs->type;
      return;
    case ND_ADDR:
      // &<unary> より<unary>のをポインタの先とする．
      node->type = pointer_to(node->unary->type);
      return;
    case ND_DEREF:
      // *<unary> より<unary>がポインタならそのポインタのさす先の型と同じにする．
      if(node->unary->type->kind == TP_PTR){
        node->type = node->unary->type->base;
      }
      else{
        // でなければint型
        node->type = int_type();
      }
      return;
    default:
      // 通常，int型
      node->type = int_type();
      return;
  }
}

void add_type(Func *func){
  // 全ての関数のノードに型を付ける．
  for(Func *f = func; f; f = f->next){
    for(Node *n = f->node; n; n = n->next){
      visit(n);
    }
  }
}
