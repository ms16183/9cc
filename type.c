#include "9cc.h"

Type *int_type(){
  Type *new = (Type*)calloc(1, sizeof(Type));
  new->kind = TP_INT;
  return new;
}

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
      node->type = pointer_to(node->unary->type);
      return;
    case ND_DEREF:
      if(node->unary->type->kind == TP_PTR){
        node->type = node->unary->type->base;
      }
      else{
        node->type = int_type();
      }
      node->type = int_type();
      return;
    default:
      node->type = int_type();
      return;
  }
}

void add_type(Func *func){
  for(Func *f = func; f; f = f->next){
    for(Node *n = f->node; n; n = n->next){
      visit(n);
    }
  }
}
