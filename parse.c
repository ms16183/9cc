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
      locals->name = tok->str;
      locals->len = tok->len;
      locals->offset = 8;
      new->offset = locals->offset;
    }
    else{
      lvar = (LVar*)calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals->offset + 8;
      new->offset = lvar->offset;
      locals = lvar;
    }
  }
  return new;
}

// 関数ノードを生成する．
Node *new_node_func(Token *tok){
  Node *node = new_node(ND_FUNCALL, NULL, NULL);
  char *buf = (char*)malloc(tok->len + 1);
  strncpy(buf, tok->str, tok->len);
  buf[tok->len] = '\0';
  node->funcname = buf;
  return node;
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
    node = new_node(ND_BLOCK, NULL, NULL);
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
    return new_node(ND_ADD, new_node_num(0), unary());
  }
  if(consume("-")){
    // -a = 0 - a
    return new_node(ND_SUB, new_node_num(0), unary());
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
    // 関数
    if(consume("(")){
      // TODO: 引数なし
      expect(")");
      return new_node_func(tok);
    }
    return new_node_lvar(tok);
  }
  else{
    return new_node_num(expect_number());
  }
}

