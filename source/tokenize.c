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



bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c) {
	return ('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		('0' <= c && c <= '9') ||
		(c == '_');
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

		if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
			cur = new_token(TK_RETURN, cur, "return", 6);
			p += 6;
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


