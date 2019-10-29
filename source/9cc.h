#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;
typedef struct Token Token;
typedef struct LVar LVar;

extern Token *token;
extern char *user_input;
extern Node *code[100];
extern LVar *locals;


typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
	ND_EQ,
	ND_NE,
	ND_LT,
	ND_LE,
	ND_ASSIGN,
	ND_LVAR,
} NodeKind;

struct LVar {
	LVar *next;
	char *name;
	int len;
	int offset;
};

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val; // kindがND_NUMの場合のみ使う
	int offset; // kindがND_LVARの場合のみ使う
};

typedef enum {
	TK_RESERVED,
	TK_IDENT,
	TK_NUM,
	TK_EOF,
} TokenKind;


struct Token {
	TokenKind kind;
	Token *next;
	int val;
	char *str;
	int len;
};

void error_at(char *loc, char *fmt, ...);

void error(char *fmt, ...);

bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *consume_ident(void);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();

Node *primary();
Node *mul();
Node *unary();
bool startswith(char *p, char *q);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();
void gen(Node *node);


