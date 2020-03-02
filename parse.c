#include "9cc.h"

/*
 * 構文解析パーサ
 */

VarList *locals;

// ローカル変数でその名前が以前使われたか判別する．
// 見つかった場合，そのローカル変数のリストのポインタが返却される．
Var *find_var(Token *tok){
  for(VarList *vl = locals; vl; vl=vl->next){
    Var *var = vl->var;
    if(strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len)){
      return var;
    }
  }
  return NULL;
}

// 次のノードを生成する．
Node *new_node(NodeKind kind){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = kind;
  return new;
}

// 二項間演算子のノードを生成する．
Node *new_node_bin(NodeKind kind, Node *lhs, Node *rhs){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = kind;
  new->lhs = lhs;
  new->rhs = rhs;
  return new;
}

// 変数ノードを生成する．
Node *new_node_var(Var *var){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_VAR;
  new->var = var;
  return new;
}

// 終端にヌル文字を付与してコピーする．
char *cpy_nullc(char *name, int len){
  char *buf = (char*)calloc(1, len+1);
  strncpy(buf, name, len);
  buf[len] = '\0';
  return buf;
}

Var *push_var(char *name){

  Var *new = (Var*)calloc(1, sizeof(Var));
  new->name = name;
  VarList *vl = (VarList*)calloc(1, sizeof(VarList));
  vl->var = new;
  vl->next = locals;
  locals = vl;
  return new;
}

// 関数ノードを生成する．
Node *new_node_func(Token *tok, Node *args){
  Node *node = new_node(ND_FUNCALL);
  node->funcname = cpy_nullc(tok->str, tok->len);
  node->args = args;
  return node;
}

// 片方のみのノードを生成する．
Node *new_node_unary(NodeKind kind, Node *unary){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = kind;
  new->unary = unary;
  return new;
}

// 次の数値ノードを生成する．
Node *new_node_num(int val){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_NUM;
  new->val = val;
  return new;
}

// if else文のノードを生成する．
Node *new_node_if(Node *cond, Node *then, Node *if_else){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_IF;
  new->cond = cond;
  new->then = then;
  new->if_else = if_else;
  return new;
}

// while文のノードを生成する．
Node *new_node_while(Node *cond, Node *then){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_WHILE;
  new->cond = cond;
  new->then = then;
  return new;
}

// for文のノードを生成する．
Node *new_node_for(Node *for_init, Node *cond, Node *for_update, Node *then){
  Node *new = (Node*)calloc(1, sizeof(Node));
  new->kind = ND_FOR;
  new->for_init = for_init;
  new->cond = cond;
  new->for_update = for_update;
  new->then = then;
  return new;
}

// BNFによる数式の構文解析
Func *program();
Func *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *fargs();
Node *primary();

Func *program(){
  Func head;
  head.next = NULL;
  Func *cur = &head;

  while(!at_eof()){
    cur->next = function();
    cur = cur->next;
  }
  return head.next;
}

VarList *read_func_params(){
  if(consume(")")){
    return NULL;
  }

  VarList *head = (VarList*)calloc(1, sizeof(VarList));
  head->var = push_var(expect_ident());
  VarList *cur = head;

  while(!consume(")")){
    expect(",");
    cur->next = (VarList*)calloc(1, sizeof(VarList));
    cur->next->var = push_var(expect_ident());
    cur = cur->next;
  }
  return head;
}

Func *function(){
  locals = NULL;
  Func *f = (Func*)calloc(1, sizeof(Func));
  f->name = expect_ident();

  expect("(");

  f->params = read_func_params();
  expect("{");

  Node head;
  head.next = NULL;
  Node *cur = &head;
  while(!consume("}")){
    cur->next = stmt();
    cur = cur->next;
  }

  f->node = head.next;
  f->locals = locals;
  return f;
}

Node *stmt(){
  Node *node;

  if(consume("{")){
    // ブロックが閉じるまでの式をリスト化する．
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while(!consume("}")){
      cur->next = stmt();
      cur = cur->next;
    }
    // ブロックであるとわかればよいので左辺値や右辺値は無くてよい．
    node = new_node(ND_BLOCK);
    node->block = head.next;
    return node;
  }

  if(consume("if")){
    Node *cond;
    Node *then;
    Node *if_else = NULL;

    expect("(");
    cond = expr(); // 条件式
    expect(")");
    then = stmt(); // 条件式が真のときの処理

    if(consume("else")){
      if_else = stmt(); // 条件式が偽のときの処理
    }

    return new_node_if(cond, then, if_else);
  }

  if(consume("while")){
    Node *cond;
    Node *then;

    expect("(");
    cond = expr(); // 条件式
    expect(")");
    then = stmt(); // 条件式が真のときの処理

    return new_node_while(cond, then);
  }

  if(consume("for")){
    Node *for_init = NULL;
    Node *cond = NULL;
    Node *for_update = NULL;
    Node *for_then;

    expect("(");

    // for(;;)となっていた場合，";"を先読みする．
    // ";"で無ければ式が存在する．
    if(!check_symbol(token->str, ";")){
      for_init = expr();
    }
    expect(";");
    if(!check_symbol(token->str, ";")){
      cond = expr();
    }
    expect(";");
    if(!check_symbol(token->str, ")")){
      for_update = expr();
    }
    expect(")");
    for_then = stmt();

    return new_node_for(for_init, cond, for_update, for_then);
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
    node = new_node_bin(ND_ASSIGN, node, assign());
  }
  return node;
}

Node *equality(){
  Node *node = relational();
  while(true){
    if(consume("==")){
      node = new_node_bin(ND_EQ, node, relational());
    }
    else if(consume("!=")){
      node = new_node_bin(ND_NE, node, relational());
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
      node = new_node_bin(ND_LE, node, add());
    }
    else if(consume("<")){
      node = new_node_bin(ND_LT, node, add());
    }
    else if(consume(">=")){
      // GEだが，左辺と右辺を入れ替えればLEと等価である．
      node = new_node_bin(ND_LE, add(), node);
    }
    else if(consume(">")){
      // GTだが，左辺と右辺を入れ替えればLTと等価である．
      node = new_node_bin(ND_LT, add(), node);
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
      node = new_node_bin(ND_ADD, node, mul());
    }
    else if(consume("-")){
      node = new_node_bin(ND_SUB, node, mul());
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
      node = new_node_bin(ND_MUL, node, unary());
    }
    else if(consume("/")){
      node = new_node_bin(ND_DIV, node, unary());
    }
    else{
      return node;
    }
  }
}

Node *unary(){
  if(consume("+")){
    // +a = 0 + a
    return new_node_bin(ND_ADD, new_node_num(0), unary());
  }
  if(consume("-")){
    // -a = 0 - a
    return new_node_bin(ND_SUB, new_node_num(0), unary());
  }
  if(consume("&")){
    return new_node_unary(ND_ADDR, unary());
  }
  if(consume("*")){
    return new_node_unary(ND_DEREF, unary());
  }
  return primary();
}

Node *fargs(){
  if(consume(")")){
    return NULL;
  }
  Node *head = assign();
  Node *cur = head;

  while(consume(",")){
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return head;
}

Node *primary(){

  if(consume("(")){
    Node *node = expr();
    expect(")");
    return node;
  }

  // 変数or関数
  Token *tok = consume_ident();
  if(tok){
    // 関数
    if(consume("(")){
      return new_node_func(tok, fargs());
    }
    // 変数
    Var *var = find_var(tok);
    if(!var){
      var = push_var(cpy_nullc(tok->str, tok->len));
    }
    return new_node_var(var);
  }
  // 数値
  return new_node_num(expect_number());
}

