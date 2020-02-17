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

// 変数ノードを生成する．
Node *new_node_lvar(Token *tok){

  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if(lvar){
    new->offset = lvar->offset;
  }
  // 新しく出現した変数名の場合，リストに追加する．
  else{
    // ローカル変数自体が登場していない場合，
    if(!locals){
      locals = (LVar*)calloc(1, sizeof(LVar));
    }

    lvar = (LVar*)calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->offset = locals->offset + 8;
    new->offset = lvar->offset;
    locals = lvar;
  }
  return new;
}

// 片方のみのノードを生成する．
Node *new_node_unary(NodeKind kind, Node *expr){
  Node *node = new_node(kind, expr, NULL);
  return node;
}

// 次の数値ノードを生成する．
Node *new_node_num(int val){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_NUM;
  new->val = val;
  return new;
}

// if else文のノードを生成する．
Node *new_node_if(Node *if_cond, Node *if_true, Node *if_false){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_IF;
  new->if_cond = if_cond;
  new->if_true = if_true;
  new->if_false = if_false;
  return new;
}

// while文のノードを生成する．
Node *new_node_while(Node *while_cond, Node *while_true){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_WHILE;
  new->while_cond = while_cond;
  new->while_true = while_true;
  return new;
}

// for文のノードを生成する．
Node *new_node_for(Node *for_init, Node *for_cond, Node *for_update, Node *for_true){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_FOR;
  new->for_init = for_init;
  new->for_cond = for_cond;
  new->for_update = for_update;
  new->for_true = for_true;
  return new;
}

// BNFによる数式の構文解析
Node *program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *program(){
  Node head;
  head.next = NULL;
  Node *cur = &head;

  while(!at_eof()){
    cur->next = stmt();
    cur = cur->next;
  }
  return head.next;
}

Node *stmt(){
  Node *node;

  if(consume("if")){
    Node *if_cond;
    Node *if_true;
    Node *if_false = NULL;

    expect("(");
    if_cond = expr(); // 条件式
    expect(")");
    if_true = stmt(); // 条件式が真のときの処理

    if(consume("else")){
      if_false = stmt(); // 条件式が偽のときの処理
    }

    node = new_node_if(if_cond, if_true, if_false);
    return node;
  }

  if(consume("while")){
    Node *while_cond;
    Node *while_true;

    expect("("); // 条件式
    while_cond = expr();
    expect(")"); // 条件式が真のときの処理
    while_true = stmt();

    node = new_node_while(while_cond, while_true);
    return node;
  }

  if(consume("for")){
    Node * for_init = NULL;
    Node * for_cond = NULL;
    Node * for_update = NULL;
    Node * for_true;

    expect("(");

    // for(;;)となっていた場合，";"を先読みする．
    // ";"で無ければ式が存在する．
    if(memcmp(token->str, ";", 1)){
      for_init = expr();
    }
    expect(";");
    if(memcmp(token->str, ";", 1)){
      for_cond = expr();
    }
    expect(";");
    if(memcmp(token->str, ")", 1)){
      for_update = expr();
    }
    expect(")");
    for_true = stmt();

    node = new_node_for(for_init, for_cond, for_update, for_true);
    return node;
  }

  if(consume("return")){
    node = new_node_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }

  node = new_node_unary(ND_EXPR_STMT, expr());
  expect(";");
  return node;
}

Node *expr(){
  return assign();
}

Node *assign(){
  Node *node = equality();
  if(consume("=")){
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
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
    // +a = 0 + a
    Node *zero = new_node_num(0);
    return new_node(ND_ADD, zero, unary());
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

  // 変数or数字
  Token *tok=consume_ident();
  if(tok){
    return new_node_lvar(tok);
  }
  else{
    return new_node_num(expect_number());
  }
}

