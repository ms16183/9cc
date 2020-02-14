#include "9cc.h"

/*
 * トークナイザ
 */

// 次のトークンが期待される記号であれば真を返す．
bool consume(char *op){
  if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが変数であればそのノードを返す．
Token *consume_ident(){
  if(token->kind != TK_IDENT){
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

// 次のトークンが期待される記号であれば進める．
// consumeとの違いは，エラーを出すかどうかである．
void expect(char *op){
  if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    error_at(token->str, "'%s'ではありません．\n", op);
  }
  token = token->next;
}

// 次のトークンが数字であればその数値を返す．
int expect_number(){
  if(token->kind != TK_NUM){
    error_at(token->str, "数ではありません．");
  }
  int val = token->val;
  token = token->next;
  return val;
}

// トークンがこれ以上続かないなら真を返す．
bool at_eof(){
  return token->kind == TK_EOF;
}

// 次のトークンを生成する．
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  Token *new = (Token*)calloc(1, sizeof(Token));
  new->kind = kind;
  new->str = str;
  new->len = len;
  cur->next = new;
  return new;
}

// 入力されたプログラムの記号が正しいか確認する．
bool check_symbol(char *p, char *q){
  return !memcmp(p, q, strlen(q));
}

// ローカル変数でその名前が以前使われたか判別する．
// 見つかった場合，そのローカル変数のリストのポインタが返却される．
LVar *find_lvar(Token *token){
  for(LVar *var = locals; var; var=var->next){
    if(var->len == token->len && !memcmp(token->str, var->name, var->len)){
      return var;
    }
  }
  return NULL;
}

Token *tokenize(){

  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){

    /*
     * ここのif文では，文字長が長いトークンから判別する．
     * なぜなら，>=よりも先に>をトークン化しようとすると
     * >=の>が優先されてトークン化され，=は無視されるからである．
     * ただし空白や数字は他の演算子や識別子と被ることはないので
     * どこに書いても問題ない．
     */

    // 空白を無視する．
    if(isspace(*p)){
      p++;
      continue;
    }

    // 比較演算子
    if(check_symbol(p, "==") || check_symbol(p, "!=") ||
       check_symbol(p, ">=") || check_symbol(p, "<=")){
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
    }
    if(check_symbol(p, ">") || check_symbol(p, "<")){
        cur = new_token(TK_RESERVED, cur, p, 1);
        p++;
        continue;
    }

    // 代入
    if(check_symbol(p, "=")){
        cur = new_token(TK_RESERVED, cur, p, 1);
        p++;
        continue;
    }

    // 区切り
    if(check_symbol(p, ";")){
        cur = new_token(TK_RESERVED, cur, p, 1);
        p++;
        continue;
    }

    // 四則演算
    if(check_symbol(p, "+") || check_symbol(p, "-") ||
       check_symbol(p, "*") || check_symbol(p, "/") ||
       check_symbol(p, "(") || check_symbol(p, ")") ){
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    // 変数
    if(isalpha(*p)){
      int l = 1;
      while(isalpha(*(p+l))){
        l++;
      }
      cur = new_token(TK_IDENT, cur, p, l);
      p += l;
      continue;
    }

    // 数値
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    // それ以外
    error_at(p, "トークナイズできない文字です．");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

