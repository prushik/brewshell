#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "host_config.h"
#include "lang_config.h"
#include "tokenizer.h"
#include "parser.h"
#include "symbols.h"

// TODO: remove this and use of sprintf
#include <stdio.h>


// This function actually returns the index of the keyword, or -1 if 
// no keyword is matched
static inline int is_keyword(struct token *tok)
{
	int i,j;
	for (i = 0; i < RESERVED_KEYWORDS; i++)
	{
		int len = strlen(reserved[i]);
		if (tok->text_len != len)
			continue;
		
		for (j = 0; j < len; j++)
			if (reserved[i][j] != tok->text[j])
				break;
		if (j == len)
			return i;
	}
	return -1;
}

// This function should prepare the token array for parsing
// This function is hacky. This is really the preprocessors job, just like in C. but doing it in the parser is fine for now I suppose
// basically, this function removes whitespace and comments.
int parse_pass0(struct token *tokens, int tok_len)
{
	int i,j,k,l;
	for (i=0; i<tok_len ; i++)
	{
		if (tokens[i].type == CHAR_TYPE_SYM)
		{
			tokens[i].sym = is_keyword(&tokens[i]);
			if (tokens[i].sym >= 0)
			{
				// Token is a keyword
			}
			else
			{
				// Token must be a symbol or an error
			}
		}
		else if (tokens[i].type == CHAR_TYPE_OPR)
		{
			//check if this is a comment
			if (tokens[i].text_len>1 && tokens[i].text[0] == '/' && tokens[i].text[1] == '/')
			{
				for (j = i; j < tok_len; j++)
				{
					if (tokens[j].type == CHAR_TYPE_WHT)
					{
						for (k = 0; k < tokens[j].text_len; k++)
							if (tokens[j].text[k] == '\n')
								break;
						if (tokens[j].text[k] == '\n')
						{
							// then we need to merge i-j into i and mark i as a comment
							for (k = i; k < j; k++)
								tokens[i].text_len += tokens[k].text_len;
							for (k = i+1; k < j; k++)
							{
								for (l = i+1; l < tok_len-1; l++)
									memcpy(&tokens[l], &tokens[l+1], sizeof(struct token));
								tok_len += -1;
							}
							tokens[i].type = CHAR_TYPE_COM;
							break;
						}
					}
				}
			}
		}
		else if (tokens[i].type == CHAR_TYPE_WHT)
		{
			// Remove whitespace tokens
			for (j = i; j < tok_len-1; j++)
				memcpy(&tokens[j], &tokens[j+1], sizeof(struct token));
			tok_len += -1;
			i += -1; //because the next symbol moved into the current position
		}
	}
	return tok_len;
}

static inline int is_type_keyword(int sym)
{
	switch (sym)
	{
		case KEY_SIZE_BYTE:
		case KEY_SIZE_WORD:
		case KEY_SIZE_DWORD:
		case KEY_SIZE_QWORD:
			return 1;
	}
	if (sym >= KEY_REG_RANGE_BEGIN &&
		sym <= KEY_REG_RANGE_END)
		return 1;
	else
		return 0;
}

static inline int ignorable(int type)
{
	if (type == CHAR_TYPE_COM || type == CHAR_TYPE_WHT || type == CHAR_TYPE_PRE)
		return 1;
	else
		return 0;
}

// this tries to count symbols to find out how much memory we will need for the symbol table. however, it sucks.
int parse_count_symbols(struct token *tokens, int tok_len)
{
	int i, count=0;
	for (i=0; i<tok_len; i++)
	{
		if (tokens[i].type == CHAR_TYPE_SYM)
		{
			// If this looks like a symbol, check if it also looks like 
			// a declaration;
			if (tokens[i].sym == -1)
			{
				if ((tokens[i-1].type == CHAR_TYPE_SYM && is_type_keyword(tokens[i-1].sym))
					|| (tokens[i+1].type == CHAR_TYPE_SPE && tokens[i+1].sym == PUNC_LABL))
				{
					count++;
				}
			}
		}
	}

	return count;
}

void parsing_error(int current, struct token *tokens, char *msg, int len)
{
	char line_str[32] = {0};
	unsigned int line_len;
	write(2, "parsing error at line: ", 23);
	line_len = sprintf(line_str, "%d", tokens[current].line); // TODO: replace sprintf
	write(2, line_str, line_len);
	write(2, ", token: ", 9);
	write(2, tokens[current].text, tokens[current].text_len);
	write(2, " - ", 3);
	write(2, msg, len);
	exit(1);
}

struct parse_node *parse_tree;
static int current_parse_node = 0;

int parse_node_create(int token_num, int node_type, int parent)
{
	current_parse_node++;

	parse_tree[current_parse_node].token_num = token_num;
	parse_tree[current_parse_node].node_type = node_type;
	parse_tree[current_parse_node].parent = parent;

	return current_parse_node;
}

void parse_node_update_section(int node, int type, const char *name, int name_len)
{
//	parse_tree[node].data.section.section_type = type;
	parse_tree[node].data.section.section_name = name;
	parse_tree[node].data.section.section_name_len = name_len;
}

void parse_node_update_identifier(int node, int type, const char *name, int name_len)
{
	parse_tree[node].data.identifier.id_name = name;
	parse_tree[node].data.identifier.id_name_len = name_len;
}

inline static void parse_node_reparent(int node, int parent)
{
	parse_tree[node].parent = parent;
}

inline static int parse_node_parent(int node)
{
	return parse_tree[node].parent;
}

inline static int parse_node_last()
{
	return current_parse_node;
}

int parse_token_lookahead(int current, struct token *tokens, int tok_len)
{
	for (current+=1; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
			return current;
	}
}

int parse_value(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;
	int next;

	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == -1)
		{
			next = parse_token_lookahead(current, tokens, tok_len);
			if (tokens[next].type == CHAR_TYPE_GRP && tokens[next].text[0] == '(')
			{
				current = parse_arguments(next+1, tokens, tok_len, symbols);
				// then this is a function call!
				break;
				// TODO: parse function calls
			}
			else
			{
				// otherwise its a variable
				break;
			}
		}
		else
		if (tokens[current].type == CHAR_TYPE_NUM)
		{
			break;
		}
		else
		if (tokens[current].type == CHAR_TYPE_STR)
		{
			break;
		}
		else
		if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == '(')
		{
			current = parse_expression(current+1, tokens, tok_len, symbols);

			if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == ')')
			{
				// sub expressions need to end with a matching parenthesis
				break;
			}
			else
			{
				parsing_error(current, tokens, "expected ).\n", 12);
			}
		}
		else
		{
			parsing_error(current, tokens, "invalid value.\n", 15);
		}
	}

	return current;
}

// because mainspring processes expressions left to right, I need a trick here to create 
// the correct parse tree. My plan is to make a node, but let it float, then recurse into the 
// next level and attach the floating node to the recursively created node. Then, when the 
// expression ends, attach the last node to the true parent of the expression.
int parse_expression_right(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;
	int next;

	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_OPR)
		{
			parse_node_create(current, PARSE_NODE_OPERATOR, me);
			// TODO: handle unary operators
			for (current++; current<tok_len; current++)
			{
				if (ignorable(tokens[current].type))
					continue;
				else
				{
					parse_node_create(current, PARSE_NODE_R_VALUE, me);
					current = parse_value(current, tokens, tok_len, symbols);
					break;
				}
			}
			break;
		}
		else
		{
			//either an invalid expression, or the end of expression. assume end
			return current;
		}
	}

	next = parse_token_lookahead(current, tokens, tok_len);

	current+=1;

	if (tokens[next].type == CHAR_TYPE_OPR)
	{
		// then the expression is not over, so this expression is an lvalue for the new one
		int lval_node = parse_node_create(current, PARSE_NODE_L_VALUE, -1);
		// don't parse a value, because this expression is the lvalue
		int expr_node = parse_node_create(next, PARSE_NODE_EXPRESSION, parse_node_parent(me));
		parse_node_reparent(lval_node, expr_node);
		parse_node_reparent(me, lval_node);
		current = parse_expression_right(next, tokens, tok_len, symbols);
	}

	return current;
}

int parse_expression(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;

	//some hacky shit up ahead
	int lval_node = parse_node_create(current, PARSE_NODE_L_VALUE, -1);
	current = parse_value(current, tokens, tok_len, symbols);
	int expr_node = parse_node_create(current, PARSE_NODE_EXPRESSION, me);
	parse_node_reparent(lval_node, expr_node);
	current = parse_expression_right(current+1, tokens, tok_len, symbols);

	return current;
}

int parse_simple_statement(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;

	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		// if the first token on the line is a storage specifier, this must be a definition (either function or variable)
		if ((tokens[current].type == CHAR_TYPE_SYM 
			&& is_keyword(&tokens[current]) >= KEY_SIZE_BYTE 
			&& is_keyword(&tokens[current]) <= KEY_REG_RANGE_END))
		{
			current = parse_definition(current+1, tokens, tok_len, symbols);
			break;
		}
		else
		// if the first token on the line is "prototype", this must be a declaration
		if (tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == KEY_DECL_PROTOTYPE)
		{
			parse_node_create(current, PARSE_NODE_DECLARATION, me);
			current = parse_declaration(current+1, tokens, tok_len, symbols);
			break;
		}
		else
		if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == '{')
		{
			parse_node_create(current, PARSE_NODE_COMPOUND_STATEMENT, me);
			current = parse_compound_statement(current+1, tokens, tok_len, symbols);
			break;
		}
		else
		if (tokens[current].type == CHAR_TYPE_SPE && tokens[current].sym == PUNC_TERM)
		{
			// handle an empty statement
			// TODO: This can be handled by parse_expression instead. Decide if this is needed.
			break;
		}
		else
		{
			current = parse_expression(current, tokens, tok_len, symbols);
			if (tokens[current].type == CHAR_TYPE_SPE && tokens[current].sym == PUNC_TERM)
			{
				// expressions only form a valid statement if they end with a statement terminator
				break;
			}
			else
			{
				parsing_error(current, tokens, "invalid expression. missing semicolon?\n", 39);
			}
		}
	}
	if (tokens[current].type == CHAR_TYPE_SPE && tokens[current].sym == PUNC_TERM)
	{
		return current;
	}
	else
	{
		parsing_error(current, tokens, "invalid statement. missing semicolon?\n", 39);
	}
	return current;
}

int parse_compound_statement(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;

	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == '}')
			break;
		else
		{
			parse_node_create(current, PARSE_NODE_SIMPLE_STATEMENT, me);
			current = parse_simple_statement(current, tokens, tok_len, symbols);
		}
	}

	return current;
}

int parse_section(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;

	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_STR)
		{
			parse_node_update_section(me, 0, tokens[current].text, tokens[current].text_len);
			break;
		}
/*		if ((tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == KEY_SECTION_TEXT))
		{
			parse_tree[current_parse_node].data.section.section_type = KEY_SECTION_TEXT;
			break;
		}
		else
		if ((tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == KEY_SECTION_DATA))
		{
			parse_tree[current_parse_node].data.section.section_type = KEY_SECTION_DATA;
			break;
		}
		else
		if ((tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == KEY_SECTION_RODATA))
		{
			parse_tree[current_parse_node].data.section.section_type = KEY_SECTION_RODATA;
			break;
		}
		else
		if ((tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == KEY_SECTION_BSS))
		{
			parse_tree[current_parse_node].data.section.section_type = KEY_SECTION_BSS;
			break;
		}*/
		else
		{
			parsing_error(current, tokens, "Invalid section name.\n", 22);
		}
	}

	current+=1;

	for (current; current < tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		{
			parse_node_create(current, PARSE_NODE_SIMPLE_STATEMENT, me);
			current = parse_simple_statement(current, tokens, tok_len, symbols);
			break;
		}
	}

	return current;
}

int parse(struct token *tokens, int tok_len, struct symbol *symbols)
{
	// create space for parse tree. this is just a guess.. bad things happen if this is wrong....
	// TODO: need a better way of allocating space for the parse tree
	parse_tree = malloc(sizeof(struct parse_node) * tok_len * 2);
	current_parse_node = 0;

	parse_tree[0].node_type = PARSE_NODE_ROOT;

	int i, sym_cur=0;
	for (i=0; i<tok_len; i++)
	{
		if (ignorable(tokens[i].type))
			continue;
		if ((tokens[i].type == CHAR_TYPE_SYM && is_keyword(&tokens[i]) == KEY_DECL_SECTION))
		{
			parse_node_create(i, PARSE_NODE_SECTION, 0);
			i = parse_section(i+1, tokens, tok_len, symbols);
		}
		else
		// if the first token on the line is "prototype", this must be a declaration
		if (tokens[i].type == CHAR_TYPE_SYM && is_keyword(&tokens[i]) == KEY_DECL_PROTOTYPE)
		{
			parse_node_create(i, PARSE_NODE_DECLARATION, 0);
			i = parse_declaration(i+1, tokens, tok_len, symbols);
		}
		else
		{
			parsing_error(i, tokens, "unexpected token.\n", 18);
		}
	}

	write(1, "Parsed Successfully.\n", 21);

	return 0;
}

// this funtion parses argument declarations for a function. note that this
// function additionally takes a symbol id
int parse_argument_declarations(int current, struct token *tokens, int tok_len, struct symbol *symbols, int sym)
{
	int me = current_parse_node;
	int type = -1;

	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_SYM && is_type_keyword(tokens[current].sym))
		{
			parse_node_create(current, PARSE_NODE_ARGUMENT, me);
			type = tokens[current].sym;

			current += 1;
			for (current; current<tok_len; current++)
			{
				if (ignorable(tokens[current].type))
					continue;
				else
				if (tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == -1)
				{
					sym = symbol_create(symbols, tokens, current);
					symbol_update(symbols, sym, 0, type);
					break;
				}
				else
					parsing_error(current, tokens, "invalid identifier.\n", 20);
			}
			current += 1;
			for (current; current<tok_len; current++)
			{
				if (ignorable(tokens[current].type))
					continue;
				else
				if (tokens[current].type == CHAR_TYPE_OPR && tokens[current].text[0] == ',')
				{
					break;
				}
				else
				if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == ')')
				{
					current += -1; // allow outer loop to process this
					break;
				}
				else
					parsing_error(current, tokens, "unexpected token - expected , or ).\n", 36);
			}
		}
		else
		if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == ')')
		{
			break;
		}
	}

	return current;
}

// this funtion parses arguments being passed to a function
int parse_arguments(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;

	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == ')')
		{
			break;
		}
		else
		{
			parse_node_create(current, PARSE_NODE_EXPRESSION, me);
			current = parse_expression(current, tokens, tok_len, symbols);
			if (tokens[current].type = CHAR_TYPE_OPR && tokens[current].text[0] == ',')
			{
			}
			else
			{
				break;
			}
		}
	}

	return current;
}

// Declarations exclusively impart knowledge to the parser. They do not result in code gen, however, they do affect "scope".
// they begin with the keyword "prototype"
int parse_declaration(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	int me = current_parse_node;
	int next;

	int type = -1; // the storage specifier of the symbol
	int sym; // the identifier of the symbol in the symbol table
	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_SYM && is_type_keyword(tokens[current].sym))
		{
			type = tokens[current].sym;
			break;
		}
		else
			parsing_error(current, tokens, "storage specifier expected.\n", 28);
	}

	current += 1;
	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) == -1)
		{
			sym = symbol_create(symbols, tokens, current);
			symbol_update(symbols, sym, 0, type);
			break;
		}
		else
			parsing_error(current, tokens, "invalid identifier.\n", 20);
	}

	current += 1;
	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_SPE && tokens[current].sym == PUNC_TERM)
		{
			// end of declaration. this is a simple variable
			break;
		}
		if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == '(')
		{
			// symbol is a function
			parse_node_create(current, PARSE_NODE_ARGUMENTS, me);
			current = parse_argument_declarations(current+1, tokens, tok_len, symbols, sym);

			next = parse_token_lookahead(current, tokens, tok_len);
			if (tokens[next].type == CHAR_TYPE_SPE && tokens[current].sym == PUNC_TERM)
				break;
			else
				// expect preserve or destroy clause
				current = parse_argument_declarations(current+1, tokens, tok_len, symbols, sym);
//			break;
		}
		else
			parsing_error(current, tokens, "unexpected token.\n", 18);
	}

	return current;
}

// Definitions are basically anything that would generate a linkable symbol
// they begin with either a storage specifier, or with an identifier followed by a colon (label punctuator)
int parse_definition(int current, struct token *tokens, int tok_len, struct symbol *symbols)
{
	for (current; current<tok_len; current++)
	{
		if (ignorable(tokens[current].type))
			continue;
		else
		if (tokens[current].type == CHAR_TYPE_SYM && is_keyword(&tokens[current]) < 0)
		{
//			add_symbol(current, tokens, tok_len, symbols);
		}
		else
		if (tokens[current].type == CHAR_TYPE_GRP && tokens[current].text[0] == '}')
			return current;
		else
			parsing_error(current, tokens, "invalid identifier.\n", 20);
	}
}

//int parse_label(struct token *tokens, int tok_n, struct symbol *symbols)
//{
//	int i;
//	for (i = 0; i < tokens[tok_n].text_len; i++)
//	{
//		printf("%c", tokens[tok_n].text[i]);
//	}
//	printf(" <-- label\n");

//	add_symbol(tokens, symbols, 0, tok_n);

//	return tok_n;
//}
