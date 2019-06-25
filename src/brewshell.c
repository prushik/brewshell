#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "beer.h"
#include "ibu.h"
#include "mash.h"
#include "srm.h"
#include "yeast.h"

#include "constants.h"

#include "linenoise.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mainspring/host_config.h"
#include "mainspring/lang_config.h"
#include "mainspring/tokenizer.h"

#include <sqlite3.h>

#define str(x) x,(sizeof(x)-1)

static struct recipe beer = {0};
static char buffer[64];
static unsigned int buf_len = 0;
static sqlite3 *db;
static sqlite3_stmt *qry;


void clear_recipe(unsigned long int beer_id);
void recipe_add_malt(unsigned long int beer_id, unsigned long int malt_id, double quantity, unsigned long int row_id);
void recipe_add_hops(unsigned long int beer_id, unsigned long int hop_id, double quantity, unsigned int time, unsigned long int row_id);
void recipe_add_yeast(unsigned long int beer_id, unsigned long int yeast_id, double quantity, unsigned long int row_id);
void recipe_set_style(unsigned long int beer_id, unsigned long int style_id);
void recipe_update(unsigned long int beer_id, const char* name, double size);

#define ACTION_GETBEER		0x00
#define ACTION_ADDBEER		0x01
#define ACTION_ADDMALT		0x02
#define ACTION_ADDHOP		0x03
#define ACTION_ADDYEAST		0x04
#define ACTION_GETMALT		0x05
#define ACTION_GETHOPS		0x06
#define ACTION_GETSTYLES	0x07
#define ACTION_GETYEASTS	0x08
#define ACTION_SETMALT		0x09
#define ACTION_SETHOPS		0x0a
#define ACTION_SETYEASTS	0x0b
#define ACTION_SETSTYLE		0x0c
#define ACTION_CLEARBEER	0x0d
#define ACTION_CALCULATE	0x0e
#define ACTION_EDITBEER		0x0f
#define ACTION_UPDATEBEER	0x10

void linenoise_complete(const char *text, linenoiseCompletions *lc);
char *linenoise_hint(const char *buf, int *color, int *bold);

int main(int argc, char **argv)
{
	int i;
	char* buf;

	for (i = 1; i < argc; i++)
	{
		
	}

	/* Set the completion callback. This will be called every time the
	 * user uses the <tab> key. */
	linenoiseSetCompletionCallback(linenoise_complete);
	linenoiseSetHintsCallback(linenoise_hint);

	while ((buf = linenoise(">> ")) != 0)
	{
		if (strlen(buf) > 0)
		{
			linenoiseHistoryAdd(buf);
		}

		parse_command(buf);

		printf("[%s]\n", buf);

		// readline malloc's a new buffer every time.
		free(buf);
	}

}

void linenoise_complete(const char *text, linenoiseCompletions *lc)
{
	int i;

	for (i = 0; i < RESERVED_KEYWORDS; i++)
	{
		if (strncmp(text, reserved[i], strlen(text)) == 0) {
			linenoiseAddCompletion(lc, reserved[i]);
		}
	}

	return;
}

char *linenoise_hint(const char *buf, int *color, int *bold)
{
	if (!strcasecmp(buf,"hello"))
	{
		*color = 35;
		*bold = 0;
		return " World";
	}
	return 0;
}

static inline int ignorable(int type)
{
	if (type == CHAR_TYPE_COM || type == CHAR_TYPE_WHT)
		return 1;
	else
		return 0;
}

void parse_set_string_parameter(struct token *tokens, int len, int current)
{
	int i;

	for (i=current; i<len; i++)
	{
		if (ignorable(tokens[i].type))
			continue;

		if (tokens[i].type == CHAR_TYPE_SYM)
		{
			switch (tokens[i].sym)
			{
				case KEY_PARAM_NAME:
					for (; i<len; i++)
					{
						if (tokens[i].type == CHAR_TYPE_STR)
						{
							beer_set_string(KEY_PARAM_NAME, &tokens[i].text[1], tokens[i].text_len-2);
							break;
						}
					}
					break;
				case KEY_PARAM_AUTHOR:
					for (; i<len; i++)
					{
						if (tokens[i].type == CHAR_TYPE_STR)
						{
							beer_set_string(KEY_PARAM_AUTHOR, &tokens[i].text[1], tokens[i].text_len-2);
							break;
						}
					}
					break;
			}
		}
	}
}

void parse_set(struct token *tokens, int len, int current)
{
	int i;

	for (i=current; i<len; i++)
	{
		if (ignorable(tokens[i].type))
			continue;

		if (tokens[i].type == CHAR_TYPE_SYM)
		{
			if (tokens[i].sym >= KEY_STRING_PARAMETERS_START && tokens[i].sym <= KEY_STRING_PARAMETERS_END)
				parse_set_string_parameter(tokens, len, i);
//			if (tokens[i].sym >= KEY_FLOAT_PARAMETERS_START && tokens[i].sym <= KEY_FLOAT_PARAMETERS_END)
//				parse_set_string_parameter(tokens, len, i);
//			if (tokens[i].sym >= KEY_FLOAT_PARAMETERS_START && tokens[i].sym <= KEY_FLOAT_PARAMETERS_END)
//				parse_set_array_parameter(tokens, len, i);
		}

	}
}

int parse_command(const char *text)
{
	int len, i, j;
	struct token *tokens;

	len=strlen(text);
	if (len < 0)
		return 0;
	tokens = malloc(sizeof(struct token) * (count_tokens(text,len)+1));
	len = tokenize(text, len, tokens);

	// convert builtins to symbolic names
	for (i=0; i<len+1; i++)
	{
		if (tokens[i].type == CHAR_TYPE_SYM)
		{
			tokens[i].sym = -1;
			for (j=0;j<RESERVED_KEYWORDS; j++)
			{
				if (strncmp(tokens[i].text, reserved[j], tokens[i].text_len) == 0)
				{
					tokens[i].sym = j;
				}
			}
		}
	}

	for (i=0; i<len+1; i++)
	{
		if (ignorable(tokens[i].type))
			continue;

		if (tokens[i].type == CHAR_TYPE_SYM)
		{
			switch (tokens[i].sym)
			{
				case KEY_ACTION_PRINT:
					printf("beer:\n");
					beer_print_recipe();
					break;
				case KEY_ACTION_SET:
					printf("setting a parameter\n");
					parse_set(tokens, len+1, i);
					break;
				default:
					printf("unknown action: %ld\n", tokens[i].sym);
					break;
			}
		}
		else
		{
			printf("type: %d\n",tokens[i].type);
		}
	}

	free(tokens);
}

int web_main(int argc, char **argv)
{
	int i;
	unsigned long int beer_id = 0;
	unsigned long int style_id = 0;
	unsigned long int action = 0;
	unsigned long int ing_id = 0;
	unsigned long int row_id = 0;
	double amount = 0, time = 0;
	char *beer_name;
	char *author_name;

	for (i=1; i<argc; i++)
	{
		// can't use str() here because gcc optimizer can mess this up (strncmp is a builtin)
		if (strncmp(argv[i], "beer_id", 7) == 0)
		{
			beer_id = strtoul(&argv[i][8], NULL, 0);
		}
		if (strncmp(argv[i], "action", 6) == 0)
		{
			if (strncmp(&argv[i][7], "getbeer", 7) == 0)
				action = ACTION_GETBEER;
			if (strncmp(&argv[i][7], "addbeer", 7) == 0)
				action = ACTION_ADDBEER;
			if (strncmp(&argv[i][7], "addmalt", 7) == 0)
				action = ACTION_ADDMALT;
			if (strncmp(&argv[i][7], "addhops", 7) == 0)
				action = ACTION_ADDHOP;
			if (strncmp(&argv[i][7], "addyeast", 8) == 0)
				action = ACTION_ADDYEAST;
			if (strncmp(&argv[i][7], "getmalt", 7) == 0)
				action = ACTION_GETMALT;
			if (strncmp(&argv[i][7], "gethops", 7) == 0)
				action = ACTION_GETHOPS;
			if (strncmp(&argv[i][7], "getyeasts", 9) == 0)
				action = ACTION_GETYEASTS;
			if (strncmp(&argv[i][7], "getstyles", 9) == 0)
				action = ACTION_GETSTYLES;
			if (strncmp(&argv[i][7], "setmalt", 7) == 0)
				action = ACTION_SETMALT;
			if (strncmp(&argv[i][7], "sethops", 7) == 0)
				action = ACTION_SETHOPS;
			if (strncmp(&argv[i][7], "setyeasts", 9) == 0)
				action = ACTION_SETYEASTS;
			if (strncmp(&argv[i][7], "setstyle", 8) == 0)
				action = ACTION_SETSTYLE;
			if (strncmp(&argv[i][7], "clearbeer", 9) == 0)
				action = ACTION_CLEARBEER;
			if (strncmp(&argv[i][7], "calculate", 9) == 0)
				action = ACTION_CALCULATE;
			if (strncmp(&argv[i][7], "editbeer", 8) == 0)
				action = ACTION_EDITBEER;
			if (strncmp(&argv[i][7], "updatebeer", 10) == 0)
				action = ACTION_UPDATEBEER;
		}
		if (strncmp(argv[i], "ing_id", 6) == 0)
		{
			ing_id = strtoul(&argv[i][7], NULL, 0);
		}
		if (strncmp(argv[i], "amount", 6) == 0)
		{
			amount = strtod(&argv[i][7], NULL);
		}
		if (strncmp(argv[i], "time", 4) == 0)
		{
			time = strtod(&argv[i][5], NULL);
		}
		if (strncmp(argv[i], "name", 4) == 0)
		{
			beer_name = &argv[i][5];
		}
		if (strncmp(argv[i], "author", 6) == 0)
		{
			author_name = &argv[i][7];
		}
		if (strncmp(argv[i], "style_id", 8) == 0)
		{
			style_id = strtoul(&argv[i][9], NULL, 0);
		}
		if (strncmp(argv[i], "row", 3) == 0)
		{
			row_id = strtoul(&argv[i][4], NULL, 0);
		}
	}

	sqlite3_open(DATABASE, &db);
	sqlite3_busy_timeout(db, 30000); // 30 seconds...

	if (action == ACTION_GETBEER)
		recipe_json(beer_id);

	if (action == ACTION_ADDBEER)
		add_recipe(beer_name, style_id, author_name);

	if (action == ACTION_GETMALT)
		malt_json();

	if (action == ACTION_GETHOPS)
		hops_json();

	if (action == ACTION_GETYEASTS)
		yeasts_json();

	if (action == ACTION_GETSTYLES)
		styles_json();

	if (action == ACTION_CLEARBEER)
		clear_recipe(beer_id);

	if (action == ACTION_SETSTYLE)
		recipe_set_style(beer_id, style_id);

	if (action == ACTION_ADDMALT)
		recipe_add_malt(beer_id, ing_id, amount, row_id);

	if (action == ACTION_ADDHOP)
		recipe_add_hops(beer_id, ing_id, amount, time, row_id);

	if (action == ACTION_ADDYEAST)
		recipe_add_yeast(beer_id, ing_id, amount, row_id);

	if (action == ACTION_CALCULATE)
		calculate_json(beer_id);

	if (action == ACTION_EDITBEER)
		beer_json(beer_id);

	if (action == ACTION_UPDATEBEER)
		recipe_update(beer_id, beer_name, amount);

	sqlite3_close(db);
}

int add_recipe(const char *name, const unsigned long int style_id, const char *author)
{
	sqlite3_prepare_v2(db, str("insert into recipe (name, style_id, author, volume, malt_n, hops_n, yeast_n) values (?, ?, ?, 5, 0, 0, 0);"), &qry, NULL);
	sqlite3_bind_text(qry, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int64(qry, 2, style_id);
	sqlite3_bind_text(qry, 3, author, -1, SQLITE_STATIC);

	char sqlstr[512];

	int code, len;
	while ((code = sqlite3_step(qry)) != SQLITE_DONE)
	{
		sprintf(sqlstr,"%d %s\n%n", code, sqlite3_errmsg(db), &len);
		write(2, sqlstr, len);
		break;
	}

	sqlite3_finalize(qry);

	int beer_id = sqlite3_last_insert_rowid(db);

	write(1, str("{ \"beer_id\":"));
	sprintf(buffer, "%d%n", beer_id, &buf_len);
	write(1, buffer, buf_len);
	write(1, str(" }\n"));

}

void clear_recipe(unsigned long int beer_id)
{
	sqlite3_prepare_v2(db, str("update recipe set malt_n = 0, hops_n = 0, yeast_n = 0 where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	sqlite3_prepare_v2(db, str("delete from ingredients where recipe_id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	write(1, str("{}\n"));
	return;
}


void recipe_add_malt(unsigned long int beer_id, unsigned long int malt_id, double quantity, unsigned long int row_id)
{
	sqlite3_prepare_v2(db, str("insert into ingredients (recipe_id, ingredient_id, quantity, type) values (?,?,?,?);"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, malt_id);
	sqlite3_bind_double(qry, 3, quantity);
	sqlite3_bind_int(qry, 4, ING_TYPE_MALT);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	sqlite3_prepare_v2(db, str("update recipe set malt_n = malt_n+1 where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	sprintf(buffer, "{\"saved\":\"malt\",\"row\":%ld}%n", row_id, &buf_len);
	write(1, buffer, buf_len);
	return;
}

void recipe_add_hops(unsigned long int beer_id, unsigned long int hop_id, double quantity, unsigned int time, unsigned long int row_id)
{
	sqlite3_prepare_v2(db, str("insert into ingredients (recipe_id, ingredient_id, quantity, time, type) values (?,?,?,?,?);"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, hop_id);
	sqlite3_bind_double(qry, 3, quantity);
	sqlite3_bind_int(qry, 4, time);
	sqlite3_bind_int(qry, 5, ING_TYPE_HOPS);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	sqlite3_prepare_v2(db, str("update recipe set hops_n = hops_n+1 where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	sprintf(buffer, "{\"saved\":\"hop\",\"row\":%ld}%n", row_id, &buf_len);
	write(1, buffer, buf_len);
	return;
}

void recipe_add_yeast(unsigned long int beer_id, unsigned long int yeast_id, double quantity, unsigned long int row_id)
{
	sqlite3_prepare_v2(db, str("insert into ingredients (recipe_id, ingredient_id, quantity, type) values (?,?,?,?);"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, yeast_id);
	sqlite3_bind_double(qry, 3, quantity);
	sqlite3_bind_int(qry, 4, ING_TYPE_YEAST);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	sqlite3_prepare_v2(db, str("update recipe set yeast_n = yeast_n+1 where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	sprintf(buffer, "{\"saved\":\"yeast\",\"row\":%ld}%n", row_id, &buf_len);
	write(1, buffer, buf_len);
	return;
}

void recipe_set_style(unsigned long int beer_id, unsigned long int style_id)
{
	sqlite3_prepare_v2(db, str("update recipe set style_id = ? where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, style_id);
	sqlite3_bind_int(qry, 2, beer_id);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	write(1, str("{}\n"));
	return;
}

void recipe_update(unsigned long int beer_id, const char* name, double size)
{
	sqlite3_prepare_v2(db, str("update recipe set name = ?, volume = ? where id = ?;"), &qry, NULL);
	sqlite3_bind_text(qry, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_double(qry, 2, size);
	sqlite3_bind_int(qry, 3, beer_id);
	while (sqlite3_step(qry) != SQLITE_DONE) ;
	sqlite3_finalize(qry);

	write(1, str("{\"saved\":\"name\",\"saved\":\"volume\"}\n"));
	return;
}

int malt_json()
{
	int i = 0;

	write(1, str("{ \"malts\": ["));

	sqlite3_prepare_v2(db, str("select name, potential, mcu, id from malts order by pts_potential desc;"), &qry, NULL);
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		if (i) write(1, str(", ")); i++;
		write(1, str("{ \"name\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 0));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"potential\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 1));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"mcu\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 2));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"id\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 3));
		write(1, buffer, strlen(buffer));
		write(1, str("\" }"));
	}
	sqlite3_finalize(qry);

	write(1, str("] }\n"));
}

int hops_json()
{
	int i = 0;

	write(1, str("{ \"hops\": ["));

	sqlite3_prepare_v2(db, str("select name, alpha, id from hops order by alpha;"), &qry, NULL);
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		if (i) write(1, str(", ")); i++;
		write(1, str("{ \"name\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 0));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"alpha\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 1));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"id\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 2));
		write(1, buffer, strlen(buffer));
		write(1, str("\" }"));
	}
	sqlite3_finalize(qry);

	write(1, str("] }\n"));
}

int yeasts_json()
{
	int i = 0;

	write(1, str("{ \"yeasts\": ["));

	sqlite3_prepare_v2(db, str("select name, attenuation, flocculation, id from yeasts order by attenuation;"), &qry, NULL);
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		if (i) write(1, str(", ")); i++;
		write(1, str("{ \"name\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 0));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"attenuation\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 1));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"flocculation\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 2));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"id\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 3));
		write(1, buffer, strlen(buffer));
		write(1, str("\" }"));
	}
	sqlite3_finalize(qry);

	write(1, str("] }\n"));
}

int styles_json()
{
	int i = 0;

	write(1, str("{ \"styles\": ["));

	sqlite3_prepare_v2(db, str("select name, og_min, og_max, fg_min, fg_max, ibu_min, ibu_max, color_min, color_max, carb_min, carb_max, abv_min, abv_max, id from style order by color_max;"), &qry, NULL);
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		if (i) write(1, str(", ")); i++;
		write(1, str("{ \"name\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 0));
		write(1, buffer, strlen(buffer));
		write(1, str("\", \"og\": ["));
		strcpy(buffer, sqlite3_column_text(qry, 1));
		write(1, buffer, strlen(buffer));
		write(1, str(", "));
		strcpy(buffer, sqlite3_column_text(qry, 2));
		write(1, buffer, strlen(buffer));
		write(1, str("], \"fg\": ["));
		strcpy(buffer, sqlite3_column_text(qry, 3));
		write(1, buffer, strlen(buffer));
		write(1, str(", "));
		strcpy(buffer, sqlite3_column_text(qry, 4));
		write(1, buffer, strlen(buffer));
		write(1, str("], \"ibu\": ["));
		strcpy(buffer, sqlite3_column_text(qry, 5));
		write(1, buffer, strlen(buffer));
		write(1, str(", "));
		strcpy(buffer, sqlite3_column_text(qry, 6));
		write(1, buffer, strlen(buffer));
		write(1, str("], \"srm\": ["));
		strcpy(buffer, sqlite3_column_text(qry, 7));
		write(1, buffer, strlen(buffer));
		write(1, str(", "));
		strcpy(buffer, sqlite3_column_text(qry, 8));
		write(1, buffer, strlen(buffer));
		write(1, str("], \"carb\": ["));
		strcpy(buffer, sqlite3_column_text(qry, 9));
		write(1, buffer, strlen(buffer));
		write(1, str(", "));
		strcpy(buffer, sqlite3_column_text(qry, 10));
		write(1, buffer, strlen(buffer));
		write(1, str("], \"abv\": ["));
		strcpy(buffer, sqlite3_column_text(qry, 11));
		write(1, buffer, strlen(buffer));
		write(1, str(", "));
		strcpy(buffer, sqlite3_column_text(qry, 12));
		write(1, buffer, strlen(buffer));
		write(1, str("], \"id\": \""));
		strcpy(buffer, sqlite3_column_text(qry, 13));
		write(1, buffer, strlen(buffer));
		write(1, str("\" }"));
	}
	sqlite3_finalize(qry);

	write(1, str("] }\n"));
}

int recipe_json(unsigned long int beer_id)
{
	int i;

	sqlite3_prepare_v2(db, str("select name, author, volume, malt_n, hops_n, yeast_n, id from recipe where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);

	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.name, sqlite3_column_text(qry, 0));
		strcpy(beer.author, sqlite3_column_text(qry, 1));
		beer.vol = sqlite3_column_double(qry, 2);
		beer.malt_n = sqlite3_column_int(qry, 3);
		beer.hop_n = sqlite3_column_int(qry, 4);
		beer.yeast_n = sqlite3_column_int(qry, 5);
		if (beer_id != sqlite3_column_int(qry, 6))
		{
			write(1, str("No recipe with that number found. Something is wrong.\n"));
			// error
		}
	}
	sqlite3_finalize(qry);

	beer.malts = malloc(beer.malt_n * sizeof(struct malt));
	beer.hops = malloc(beer.hop_n * sizeof(struct hop));
	beer.yeasts = malloc(beer.yeast_n * sizeof(struct yeast));


	// Get the malts
	sqlite3_prepare_v2(db, str("select malts.name, malts.pts_potential, malts.mcu, ingredients.quantity, ingredients.adjustment from ingredients left join malts on malts.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_MALT);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.malts[i].name, sqlite3_column_text(qry, 0));
		beer.malts[i].pts_potential = sqlite3_column_double(qry, 1);
		beer.malts[i].mcu = sqlite3_column_double(qry, 2);
		beer.malts[i].mass = sqlite3_column_double(qry, 3);
		i++;
	}
	sqlite3_finalize(qry);

	// Get the hops
	sqlite3_prepare_v2(db, str("select hops.name, hops.alpha, hops.type, ingredients.quantity, ingredients.time, ingredients.adjustment from ingredients left join hops on hops.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_HOPS);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.hops[i].name, sqlite3_column_text(qry, 0));
		beer.hops[i].alpha = sqlite3_column_double(qry, 1);
//		beer.hops[i].type = sqlite3_column_int(qry, 2);
		beer.hops[i].mass = sqlite3_column_double(qry, 3);
		beer.hops[i].time = sqlite3_column_int(qry, 4);
		i++;
	}
	sqlite3_finalize(qry);

	// Get the hops
	sqlite3_prepare_v2(db, str("select yeasts.name, yeasts.attenuation, ingredients.quantity, ingredients.time, ingredients.adjustment from ingredients left join yeasts on yeasts.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_YEAST);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.yeasts[i].name, sqlite3_column_text(qry, 0));
		beer.yeasts[i].attenuation = sqlite3_column_double(qry, 1);
		beer.yeasts[i].amount = sqlite3_column_double(qry, 2);
		beer.yeasts[i].time = sqlite3_column_int(qry, 3);
		i++;
	}
	sqlite3_finalize(qry);

	calculate_recipe(&beer);

	write(1, str(
		"{\n"
	));
	write(1, str(
		"	\"name\" : \""
	));
	write(1, beer.name, strlen(beer.name));

	write(1, str(
		"\",\n"
		"	\"author\" : \""
	));
	write(1, beer.author, strlen(beer.author));

	write(1, str(
		"\",\n"
		"	\"volume\" : "
	));
	sprintf(buffer, "%.2lf%n", beer.vol, &buf_len);
	write(1, buffer, buf_len);

	write(1, str(
		",\n"
	));

	write(1, str("	\"og\" : \""));
	sprintf(buffer, "%.4lf%n", beer.og, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"fg\": \""));
	sprintf(buffer, "%.4lf%n", beer.fg, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"abv\": \""));
	sprintf(buffer, "%.1lf%n", beer.abv, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"ibu\": \""));
	sprintf(buffer, "%.1lf%n", beer.ibu, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str(
		"	\"malts\":\n"
		"	[\n"
	));
	for (i=0; i < beer.malt_n; i++)
	{
		write(1, str("		{\n"));
		write(1, str("			\"name\" : \""));
		write(1, beer.malts[i].name, strlen(beer.malts[i].name));
		write(1, str("\",\n"));

		write(1, str("			\"weight\" : \""));
		sprintf(buffer, "%.2lf%n", beer.malts[i].mass, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\",\n"));

		write(1, str("			\"potential\" : \""));
		sprintf(buffer, "%.1lf%n", beer.malts[i].pts_potential, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\"\n"));
		if (i+1<beer.malt_n)
			write(1, str("		},\n"));
		else
			write(1, str("		}\n"));
	}
	write(1, str("	],\n"));

	write(1, str(
		"	\"hops\":\n"
		"	[\n"
	));
	for (i=0; i < beer.hop_n; i++)
	{
		write(1, str("		{\n"));
		write(1, str("			\"name\" : \""));
		write(1, beer.hops[i].name, strlen(beer.hops[i].name));
		write(1, str("\",\n"));

		write(1, str("			\"mass\" : \""));
		sprintf(buffer, "%.2lf%n", beer.hops[i].mass, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\",\n"));

		write(1, str("			\"time\" : \""));
		sprintf(buffer, "%d%n", beer.hops[i].time, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\",\n"));

		write(1, str("			\"alpha\" : \""));
		sprintf(buffer, "%.1lf%n", beer.hops[i].alpha, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\"\n"));

		if (i+1<beer.hop_n)
			write(1, str("		},\n"));
		else
			write(1, str("		}\n"));
	}
	write(1, str("	],\n"));

	write(1, str(
		"	\"yeast\":\n"
		"	[\n"
	));
	for (i=0; i < beer.yeast_n; i++)
	{
		write(1, str("		{\n"));
		write(1, str("			\"name\" : \""));
		write(1, beer.yeasts[i].name, strlen(beer.yeasts[i].name));
		write(1, str("\",\n"));

		write(1, str("			\"amount\" : \""));
		sprintf(buffer, "%.2lf%n", beer.yeasts[i].amount, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\",\n"));

		write(1, str("			\"attenuation\" : \""));
		sprintf(buffer, "%.1lf%n", beer.yeasts[i].attenuation, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\"\n"));

		if (i+1<beer.yeast_n)
			write(1, str("		},\n"));
		else
			write(1, str("		}\n"));
	}
	write(1, str("	]\n"));

	write(1, str(
		"}\n"
	));

	return 1;
}

int calculate_json(unsigned long int beer_id)
{
	int i;

	sqlite3_prepare_v2(db, str("select name, author, volume, malt_n, hops_n, yeast_n, id from recipe where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);

	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.name, sqlite3_column_text(qry, 0));
		strcpy(beer.author, sqlite3_column_text(qry, 1));
		beer.vol = sqlite3_column_double(qry, 2);
		beer.malt_n = sqlite3_column_int(qry, 3);
		beer.hop_n = sqlite3_column_int(qry, 4);
		beer.yeast_n = sqlite3_column_int(qry, 5);
		if (beer_id != sqlite3_column_int(qry, 6))
		{
			write(1, str("No recipe with that number found. Something is wrong.\n"));
			// error
		}
	}
	sqlite3_finalize(qry);

	beer.malts = malloc(beer.malt_n * sizeof(struct malt));
	beer.hops = malloc(beer.hop_n * sizeof(struct hop));
	beer.yeasts = malloc(beer.yeast_n * sizeof(struct yeast));

	// Get the malts
	sqlite3_prepare_v2(db, str("select malts.pts_potential, malts.mcu, ingredients.quantity, ingredients.adjustment from ingredients left join malts on malts.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_MALT);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		beer.malts[i].pts_potential = sqlite3_column_double(qry, 0);
		beer.malts[i].mcu = sqlite3_column_double(qry, 1);
		beer.malts[i].mass = sqlite3_column_double(qry, 2);
		i++;
	}
	sqlite3_finalize(qry);

	// Get the hops
	sqlite3_prepare_v2(db, str("select hops.alpha, hops.type, ingredients.quantity, ingredients.time, ingredients.adjustment from ingredients left join hops on hops.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_HOPS);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		beer.hops[i].alpha = sqlite3_column_double(qry, 0);
//		beer.hops[i].type = sqlite3_column_int(qry, 1);
		beer.hops[i].mass = sqlite3_column_double(qry, 2);
		beer.hops[i].time = sqlite3_column_int(qry, 3);
		i++;
	}
	sqlite3_finalize(qry);

	// Get the yeast
	sqlite3_prepare_v2(db, str("select yeasts.attenuation, ingredients.quantity, ingredients.time, ingredients.adjustment from ingredients left join yeasts on yeasts.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_YEAST);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		beer.yeasts[i].attenuation = sqlite3_column_double(qry, 0);
		beer.yeasts[i].amount = sqlite3_column_double(qry, 1);
		beer.yeasts[i].time = sqlite3_column_int(qry, 2);
		i++;
	}
	sqlite3_finalize(qry);

	calculate_recipe(&beer);

	write(1, str(
		"{\n"
	));
	write(1, str("	\"og\" : \""));
	sprintf(buffer, "%.4lf%n", beer.og, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"fg\": \""));
	sprintf(buffer, "%.4lf%n", beer.fg, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"abv\": \""));
	sprintf(buffer, "%.1lf%n", beer.abv, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"ibu\": \""));
	sprintf(buffer, "%.1lf%n", beer.ibu, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"srm\": \""));
	sprintf(buffer, "%.1lf%n", beer.srm, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"rgb\": \""));
	sprintf(buffer, "%06x%n", srm_to_rgb(beer.srm), &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\"\n"));
	write(1, str(
		"}\n"
	));

	return 1;
}

int beer_json(int beer_id) {
	struct recipe beer;
	char buffer[32];
	int i;

	sqlite3_prepare_v2(db, str("select name, style_id, volume, malt_n, hops_n, yeast_n, id from recipe where id = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);

	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.name, sqlite3_column_text(qry, 0));
		strcpy(beer.author, sqlite3_column_text(qry, 1));
		beer.vol = sqlite3_column_double(qry, 2);
		beer.malt_n = sqlite3_column_int(qry, 3);
		beer.hop_n = sqlite3_column_int(qry, 4);
		beer.yeast_n = sqlite3_column_int(qry, 5);
		if (beer_id != sqlite3_column_int(qry, 6))
		{
			write(1, str("{\"Error\": \"No recipe with that number found. Something is wrong.\"}"));
		}
	}
	sqlite3_finalize(qry);

	beer.malts = malloc(beer.malt_n * sizeof(struct malt));
	beer.hops = malloc(beer.hop_n * sizeof(struct hop));
	beer.yeasts = malloc(beer.yeast_n * sizeof(struct yeast));


	// Get the malts
	sqlite3_prepare_v2(db, str("select ingredient_id, malts.pts_potential, malts.mcu, ingredients.quantity, ingredients.adjustment from ingredients left join malts on malts.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_MALT);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.malts[i].name, sqlite3_column_text(qry, 0));
		beer.malts[i].pts_potential = sqlite3_column_double(qry, 1);
		beer.malts[i].mcu = sqlite3_column_double(qry, 2);
		beer.malts[i].mass = sqlite3_column_double(qry, 3);
		i++;
	}
	sqlite3_finalize(qry);

	// Get the hops
	sqlite3_prepare_v2(db, str("select ingredient_id, hops.alpha, hops.type, ingredients.quantity, ingredients.time, ingredients.adjustment from ingredients left join hops on hops.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_HOPS);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.hops[i].name, sqlite3_column_text(qry, 0));
		beer.hops[i].alpha = sqlite3_column_double(qry, 1);
//		beer.hops[i].type = sqlite3_column_int(qry, 2);
		beer.hops[i].mass = sqlite3_column_double(qry, 3);
		beer.hops[i].time = sqlite3_column_int(qry, 4);
		i++;
	}
	sqlite3_finalize(qry);

	// Get the hops
	sqlite3_prepare_v2(db, str("select ingredient_id, yeasts.attenuation, ingredients.quantity, ingredients.time, ingredients.adjustment from ingredients left join yeasts on yeasts.id = ingredients.ingredient_id where recipe_id = ? and ingredients.type = ?;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, beer_id);
	sqlite3_bind_int(qry, 2, ING_TYPE_YEAST);

	i = 0;
	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.yeasts[i].name, sqlite3_column_text(qry, 0));
		beer.yeasts[i].attenuation = sqlite3_column_double(qry, 1);
		beer.yeasts[i].amount = sqlite3_column_double(qry, 2);
		beer.yeasts[i].time = sqlite3_column_int(qry, 3);
		i++;
	}
	sqlite3_finalize(qry);

	calculate_recipe(&beer);

	write(1, str(
		"{\n"
	));
	write(1, str("	\"beer_id\" : \""));
	sprintf(buffer, "%d%n", beer_id, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"name\" : \""));
	write(1, beer.name, strlen(beer.name));
	write(1, str("\",\n"));

	write(1, str("	\"volume\" : \""));
	sprintf(buffer, "%.4lf%n", beer.vol, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"style_id\" : \""));
	write(1, beer.author, strlen(beer.author));
	write(1, str("\",\n"));

	write(1, str("	\"og\" : \""));
	sprintf(buffer, "%.4lf%n", beer.og, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"fg\": \""));
	sprintf(buffer, "%.4lf%n", beer.fg, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"abv\": \""));
	sprintf(buffer, "%.1lf%n", beer.abv, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"ibu\": \""));
	sprintf(buffer, "%.1lf%n", beer.ibu, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"srm\": \""));
	sprintf(buffer, "%.1lf%n", beer.srm, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));

	write(1, str("	\"rgb\": \""));
	sprintf(buffer, "%06x%n", srm_to_rgb(beer.srm), &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\",\n"));


	write(1, str("	\"ingredients\": {\n"));
	write(1, str("		\"malts\": [\n			"));
	for (i=0; i < beer.malt_n; i++)
	{
		if (i) write(1, str(", "));
		write(1, str("{\n				\"id\" : \""));
		write(1, beer.malts[i].name, strlen(beer.malts[i].name));
		write(1, str("\",\n"));

		write(1, str("				\"weight\" : \""));
		sprintf(buffer, "%.2lf%n", beer.malts[i].mass, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\"\n			}"));
	}
	write(1, str("\n		],\n"));
	write(1, str("		\"hops\": [\n			"));
	for (i=0; i < beer.hop_n; i++)
	{
		if (i) write(1, str(", "));
		write(1, str("{\n				\"id\" : \""));
		write(1, beer.hops[i].name, strlen(beer.hops[i].name));
		write(1, str("\",\n"));

		write(1, str("				\"weight\" : \""));
		sprintf(buffer, "%.2lf%n", beer.hops[i].mass, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\",\n"));

		write(1, str("				\"time\" : \""));
		sprintf(buffer, "%d%n", beer.hops[i].time, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\"\n			}"));
	}
	write(1, str("\n		],\n"));
	write(1, str("		\"yeasts\": [\n			"));
	for (i=0; i < beer.yeast_n; i++)
	{
		if (i) write(1, str(", "));
		write(1, str("{\n				\"id\" : \""));
		write(1, beer.yeasts[i].name, strlen(beer.yeasts[i].name));
		write(1, str("\",\n"));

		write(1, str("				\"amount\" : \""));
		sprintf(buffer, "%.2lf%n", beer.yeasts[i].amount, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\"\n			}"));
	}
	write(1, str("\n		]\n"));
	write(1, str("	}\n"));

	write(1, str(
		"}\n"
	));
}
