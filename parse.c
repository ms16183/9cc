#include "9cc.h"

/*
 * 構文解析パーサ
 */

// 次のノードを生成する．
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = kind;
  new->lhs = lhs;
  new->rhs = rhs;
  return new;
}

// 次の数値ノードを生成する．
Node *new_node_num(int val){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_NUM;
  new->val = val;
  return new;
}

// BNF記法による数式の構文解析
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *expr(){
  return equality();
}

Node *equality(){
  Node *node = relational();
  while(true){
    if(consume("==")){
      node = new_node(ND_EQ, node, relational());
    }
    else if(consume("!=")){
      node = new_node(ND_NE, node, relational());
    }
    else{
      return node;
    }
  }
}

Node *relational(){
  Node *node = add();
  while(true){
    if(consume("<=")){
      node = new_node(ND_LE, node, add());
    }
    else if(consume("<")){
      node = new_node(ND_LT, node, add());
    }
    else if(consume(">=")){
      // GEだが，左辺と右辺を入れ替えればLEと等価である．
      node = new_node(ND_LE, add(), node);
    }
    else if(consume(">")){
      // GTだが，左辺と右辺を入れ替えればLTと等価である．
      node = new_node(ND_LT, add(), node);
    }
    else{
      return node;
    }
  }
}

Node *add(){
  Node *node = mul();

  while(true){
    if(consume("+")){
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume("-")){
      node = new_node(ND_SUB, node, mul());
    }
    else{
      return node;
    }
  }
}

Node *mul(){
  Node *node = unary();

  while(true){
    if(consume("*")){
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume("/")){
      node = new_node(ND_DIV, node, unary());
    }
    else{
      return node;
    }
  }
}

Node *unary(){
  if(consume("+")){
    return unary();
  }
  if(consume("-")){
    // -a = 0 - a
    Node *zero = new_node_num(0);
    return new_node(ND_SUB, zero, unary());
  }
  return primary();
}

Node *primary(){

  if(consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }

  return new_node_num(expect_number());
}

