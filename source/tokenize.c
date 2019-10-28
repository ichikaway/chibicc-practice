#include "9cc.h"

Token *token;
char *user_input;



void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

bool consume(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
	{
		return false;
	}
	token = token->next;
	return true;
}

Token *consume_ident(void) {
	if (token->kind != TK_IDENT) {
		return NULL;
	}
	Token *t = token;
	token = token->next;
	return t;
}

void expect(char *op) {
	if (token->kind != TK_RESERVED || 
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
	{
		error_at(token->str, "'%c'ではありません", op);
	}
	token = token->next;
}

int expect_number() {
	if (token->kind != TK_NUM) {
		error_at(token->str, "数ではありません");
	}
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

Token *tokenize() {
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while(*p) {
		if(isspace(*p)) {
			p++;
			continue;
		}

		if (isalpha(*p)) {
			char *q = p++;
			while (isalnum(*p)) {
				p++;
			}
			cur = new_token(TK_IDENT, cur, q, p-q);
			continue;
		}

		if (startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, "<=") || startswith(p, ">="))
		{
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if(*p == '+' || *p == '-' || *p == '*' || *p == '/'
			|| *p == '(' || *p == ')'	
			|| *p == '>' || *p == '<'
			|| *p == ';' || *p == '='

		) {
			//後置インクリメントのためわかりにくかったので、2つの処理に分割
			//cur = new_token(TK_RESERVED, cur, p++);
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}
		if(isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}
		error_at(p, "expected number");
	}
	new_token(TK_EOF, cur, p, 0);
	return head.next;
}


