struct token
{
	int type;			//type of token
	int level;			//
	long sym;			//symbolic representation of token (machine readable)
	double fsym;		//floating point symbolic representation
	int text_len;		//length of token text
	const char *text;	//pointer to text of token
	int line;			//line number from original source file
};

int id_token_str(const char *token);
int id_operator(const char *op_str, int len);
int count_tokens(const char *expr, unsigned int len);
int tokenize(const char *expr, unsigned int len, struct token *token_array);
int expr_error(const char *msg, int tok_n, struct token *token_array);
int find_l_arg(int op_n, struct token *token_array);
int find_r_arg(int op_n, int len, struct token *token_array);
