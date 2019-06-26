#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "host_config.h"
#include "lang_config.h"
#include "tokenizer.h"

static inline int token_boundary(const char *expr, int pos, int type, int start_pos)
{
	int i;

	if (char_set[type][0])
	{
		for (i = 0; i < strlen(char_set[type]); i++)
			if (char_set[type][i] == expr[pos])
				return 0; // then this is still part of the same token, do nothing.
		return 1; // This is not part of the current token's charset
	}

	if ((pos-1) > start_pos)
	{
		for (i = 0; i < strlen(char_end[type]); i++)
			if (char_end[type][i] == expr[pos-1])
				return 1; // we just passed an end character
		return 0; // We did not pass an end character
	}
	return 0; // Only happens if this is the first character
}

int id_token_str(const char *token)
{
	int i,j;
	for (i = 0; i < CHARSETS; i++)
	{
		for (j = 0; j < strlen(char_ini[i]); j++)
		{
			if (token[0] == char_ini[i][j])
				return i;
		}
	}
	return CHAR_TYPE_INV;
}

int id_operator(const char *op_str, int len)
{
	int i;
	for (i = 0; i < OPERATORS; i++)
	{
		if (strlen(operator[i]) == len && strncmp(operator[i], op_str, len) == 0)
			return i;
	}
	return OP_COM; // ? I needed something to use for invalid operators
}

int id_punctuator(const char *punc_str, int len)
{
	int i;
	for (i = 0; i < PUNCTUATORS; i++)
	{
		if (strlen(punctuator[i]) == len && strncmp(punctuator[i], punc_str, len) == 0)
			return i;
	}

	return PUNC_INVD;
}

int count_tokens(const char *expr, unsigned int len)
{
	int cur_tok, n_tok=0, tok_start_pos=0;
	int i,j;
	//Figure out what the first token is
	cur_tok = id_token_str(expr);

	//Step through the string, locate token boundaries, and count tokens
	for (i = 0; i < len; i++)
	{
		if (token_boundary(expr, i, cur_tok, tok_start_pos))
		{
//			// start next token
			cur_tok = id_token_str(&expr[i]);
			n_tok++;
			tok_start_pos = i;
		}
	}

	return n_tok;
}

int tokenize(const char *expr, unsigned int len, struct token *token_array)
{
	int cur_tok, n_tok, tok_start_pos, line;
	int i;

	//We figured out how many tokens there are, now lets figure out what they are and how to handle them
	line = 0;
	n_tok = 0;
	int level = 0;
	cur_tok = id_token_str(expr);
	token_array[n_tok].type = cur_tok;
	token_array[n_tok].text = expr;
	token_array[n_tok].level = level;
	token_array[n_tok].line = line;
	for (i=0; i < len; i++)
	{
		if (token_boundary(expr, i, cur_tok, tok_start_pos))
		{
			token_array[n_tok].text_len = (&expr[i]) - token_array[n_tok].text;

			// Interpret the token to get it into a more usable form
			if (cur_tok == CHAR_TYPE_NUM)
			{
				token_array[n_tok].sym = strtol(token_array[n_tok].text, NULL, 0);
				token_array[n_tok].fsym = strtod(token_array[n_tok].text, NULL);
			}
			if (cur_tok == CHAR_TYPE_GRP)
			{
				int k;
				for (k=0;k<token_array[n_tok].text_len;k++)
					if (token_array[n_tok].text[k] == '(')
						level++;
					else
						level--;
			}
			if (cur_tok == CHAR_TYPE_OPR)
			{
				token_array[n_tok].sym = id_operator(token_array[n_tok].text, token_array[n_tok].text_len);
			}
			if (cur_tok == CHAR_TYPE_SPE)
			{
				token_array[n_tok].sym = id_punctuator(token_array[n_tok].text, token_array[n_tok].text_len);
			}
			cur_tok = id_token_str(&expr[i]); // learn what the next token is
			n_tok++;
			tok_start_pos = i;
			token_array[n_tok].type = cur_tok;
			token_array[n_tok].text = &expr[i];
			token_array[n_tok].level = level;
			token_array[n_tok].line = line;
			if (cur_tok == CHAR_TYPE_WHT && token_array[n_tok].text[0] == '\n')
			{
				line++;
			}
		}
	}

	token_array[n_tok].text_len = (&expr[i]) - token_array[n_tok].text;

	// Interpret the token to get it into a more usable form
	if (cur_tok == CHAR_TYPE_NUM)
	{
		token_array[n_tok].sym = strtol(token_array[n_tok].text, NULL, 0);
		token_array[n_tok].fsym = strtod(token_array[n_tok].text, NULL);
	}
	if (cur_tok == CHAR_TYPE_GRP)
	{
		int k;
		for (k=0;k<token_array[n_tok].text_len;k++)
			if (token_array[n_tok].text[k] == '(')
				level++;
			else
				level--;
	}
	if (cur_tok == CHAR_TYPE_OPR)
	{
		token_array[n_tok].sym = id_operator(token_array[n_tok].text, token_array[n_tok].text_len);
	}
	if (cur_tok == CHAR_TYPE_SPE)
	{
		token_array[n_tok].sym = id_punctuator(token_array[n_tok].text, token_array[n_tok].text_len);
	}

	return n_tok;
}

// This function will cleanup the tokenization, catching any type of
// tokenization which can not be handled in one pass.
// Ideally, this should not be needed if preprocessing is already
// completed. This handles: comments
int tokenize_fixup(struct token *tokens, int tok_len)
{
	int i,j,k,l,t;
	for (i=0; i<tok_len ; i++)
	{
		if (tokens[i].type == CHAR_TYPE_OPR)
		{
			//check if this is a comment
			for (t=0; t < COMMENTS; t++)
			{
				if (tokens[i].text_len > 1 && strncmp(tokens[i].text, comment_ini[t], strlen(comment_ini[t])) == 0)
				{
					for (j = i; j < tok_len; j++)
					{
						if (tokens[j].type == CHAR_TYPE_WHT)
						{
							for (k = 0; k < tokens[j].text_len; k++)
								if (strncmp(&tokens[j].text[k], comment_end[t], strlen(comment_end[t])) == 0)
									break;
							if (strncmp(&tokens[j].text[k], comment_end[t], strlen(comment_end[t])) == 0)
							{
								// then we need to merge i-j into i and mark i as a comment
								for (k = i+1; k < j; k++)
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
		}
	}
	return tok_len;
}
