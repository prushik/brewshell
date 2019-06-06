struct token
{
	int type;			//type of token
	int level;			//
//	int group_id;		//id of current group
//	int group_parent_id;//id of parent group
//	int group_depth;	//depth of group nesting
	long sym;			//symbolic representation of token (machine readable)
	int text_len;		//length of token text
	const char *text;	//pointer to text of token
//	int argc;			//number of operands
//	int argv[6];		//token numbers of operands to this token
	int line;			//line number from original source file
	long custom_flags;	//reserved for application use
};

int id_token_str(const char *token);
int id_operator(const char *op_str, int len);
int count_tokens(const char *expr, unsigned int len);
int tokenize(const char *expr, unsigned int len, struct token *token_array);
int expr_error(const char *msg, int tok_n, struct token *token_array);
int find_l_arg(int op_n, struct token *token_array);
int find_r_arg(int op_n, int len, struct token *token_array);
