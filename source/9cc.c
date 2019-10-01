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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *expr() {
	return equality();
}

Node *equality() {
	Node *node = relational();
	
	for(;;) {
		if (consume("==")) {
			node = new_node(ND_EQ, node, relational());
		} else if (consume("!=")) {
			node = new_node(ND_NE, node, relational());
		} else {
			return node;
		}
	}
}

Node *relational() {
	Node *node = add();

	for(;;) {
		if (consume("<")) {
			node = new_node(ND_LT, node, add());
		} else if (consume("<=")) {
			node = new_node(ND_LE, node, add());
		} else if (consume(">")) {
			node = new_node(ND_LT, add(), node);
		} else if (consume(">=")) {
			node = new_node(ND_LE, add(), node);
		} else {
			return node;
		}
	}
}

Node *add() {
	Node *node = mul();

	for(;;){
		if (consume("+")) {
			node = new_node(ND_ADD, node, mul());
		} else if (consume("-")) {
			node = new_node(ND_SUB, node, mul());
		} else {
			return node;
		}
	}
}


Node *primary() {
	if(consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}
	return new_node_num(expect_number());
}

Node *mul() {
	Node *node = unary();

	for(;;) {
		if (consume("*")) {
			node = new_node(ND_MUL, node, unary());
		} else if (consume("/")) {
			node = new_node(ND_DIV, node, unary());
		} else {
			return node;
		}
	}
}

Node *unary() {
	if (consume("+")) {
		return primary();
	}
	if (consume("-")) {
		return new_node(ND_SUB, new_node_num(0), primary());
	}
	return primary();
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

int main(int argc, char **argv)
{
	if(argc != 2){
		fprintf(stderr, "error argc\n");
		return 1;
	}

	user_input = argv[1];
	token = tokenize();
	Node *node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	gen(node);

	printf("  pop rax\n");


	printf("  ret\n");
	return 0;

}
