#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#include "beer.h"
#include "constants.h"

#include "mainspring/host_config.h"
#include "mainspring/lang_config.h"

#define str(x) x,(sizeof(x)-1)

static int db_open = 0;
static sqlite3 *db;
static sqlite3_stmt *qry;

static struct recipe beer = {0};

void beer_init()
{
	beer.vol = 5.0;
}

void beer_set_float(int parameter, double value)
{
	switch (parameter)
	{
		case KEY_PARAM_VOLUME:
			beer.vol=value;
			break;
		default:
			return;
	}
}

void beer_set_array(int array, int index, int parameter, double value)
{
	if (index < 0)
	{
		parse_error("Invalid index.");
		return;
	}

	switch (array)
	{
		case KEY_PARAM_MALTS:
			if (index < beer.malt_n)
				switch (parameter)
				{
					case KEY_PARAM_QUANTITY:
						beer.malts[index].mass = value;
						break;
				}
			break;
		case KEY_PARAM_HOPS:
			if (index < beer.hop_n)
				switch (parameter)
				{
					case KEY_PARAM_QUANTITY:
						beer.hops[index].mass = value;
						break;
					case KEY_PARAM_TIME:
						beer.hops[index].time = value;
						break;
				}
			break;
		case KEY_PARAM_YEASTS:
			if (index < beer.yeast_n)
				switch (parameter)
				{
					case KEY_PARAM_QUANTITY:
						beer.yeasts[index].amount = value;
						break;
				}
			break;
		default:
			return;
	}
}

double beer_get_float(int parameter)
{
	
}

void open_db()
{
	if (!db_open)
	{
		sqlite3_open(DATABASE, &db);
		sqlite3_busy_timeout(db, 30000); // 30 seconds...
		db_open = 1;
	}
}

void beer_list_ingredients(int parameter)
{
	open_db();
	char buffer[64];

	switch (parameter)
	{
		case KEY_PARAM_MALTS:
			sqlite3_prepare_v2(db, str("select name, potential, mcu, id from malts order by pts_potential desc;"), &qry, NULL);
			while (sqlite3_step(qry) != SQLITE_DONE)
			{
				strcpy(buffer, sqlite3_column_text(qry, 3));
				printf("[%s]\t", buffer);
				strcpy(buffer, sqlite3_column_text(qry, 0));
				printf("%s\n", buffer);
			}
			sqlite3_finalize(qry);
			break;
		case KEY_PARAM_HOPS:
			sqlite3_prepare_v2(db, str("select name, alpha, id from hops order by alpha desc;"), &qry, NULL);
			while (sqlite3_step(qry) != SQLITE_DONE)
			{
				strcpy(buffer, sqlite3_column_text(qry, 2));
				printf("[%s]\t", buffer);
				strcpy(buffer, sqlite3_column_text(qry, 0));
				printf("%s\n", buffer);
			}
			sqlite3_finalize(qry);
			break;
		case KEY_PARAM_YEASTS:
			sqlite3_prepare_v2(db, str("select name, attenuation, id from yeasts order by attenuation desc;"), &qry, NULL);
			while (sqlite3_step(qry) != SQLITE_DONE)
			{
				strcpy(buffer, sqlite3_column_text(qry, 2));
				printf("[%s]\t", buffer);
				strcpy(buffer, sqlite3_column_text(qry, 0));
				printf("%s\n", buffer);
			}
			sqlite3_finalize(qry);
			break;
	}
}

void beer_set_string(int parameter, const char *value, unsigned int len)
{
//	beer.name = realloc(beer.name, len+1);
	if (len>=31)
		return;
	switch (parameter)
	{
		case KEY_PARAM_NAME:
			strcpy(beer.name, value);
			beer.name[len]='\0';
			break;
		case KEY_PARAM_AUTHOR:
			strcpy(beer.author, value);
			beer.author[len]='\0';
			break;
		default:
			return;
	}
}

int beer_add_array(int parameter)
{
	switch (parameter)
	{
		case KEY_PARAM_MALTS:
			beer.malt_n += 1;
			beer.malts = realloc(beer.malts, sizeof(struct malt)*beer.malt_n);
			return beer.malt_n;
			break;
		case KEY_PARAM_HOPS:
			beer.hop_n += 1;
			beer.hops = realloc(beer.hops, sizeof(struct hop)*beer.hop_n);
			return beer.hop_n;
			break;
		case KEY_PARAM_YEASTS:
			beer.yeast_n += 1;
			beer.yeasts = realloc(beer.yeasts, sizeof(struct yeast)*beer.yeast_n);
			return beer.yeast_n;
			break;
		default:
			return 0;
	}
}

void beer_set_malt_by_id(int index, long int id)
{
	open_db();

	sqlite3_prepare_v2(db, str("select name, potential, pts_potential, mcu from malts where id = ? limit 1;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, id);

	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.malts[index].name, sqlite3_column_text(qry, 0));
		beer.malts[index].potential = sqlite3_column_double(qry, 1);
		beer.malts[index].pts_potential = sqlite3_column_double(qry, 2);
		beer.malts[index].mcu = sqlite3_column_double(qry, 3);
	}
	sqlite3_finalize(qry);
}

void beer_set_hop_by_id(int index, long int id)
{
	open_db();

	sqlite3_prepare_v2(db, str("select name, alpha from hops where id = ? limit 1;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, id);

	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.hops[index].name, sqlite3_column_text(qry, 0));
		beer.hops[index].alpha = sqlite3_column_double(qry, 1);
	}
	sqlite3_finalize(qry);
}

void beer_set_yeast_by_id(int index, long int id)
{
	open_db();

	sqlite3_prepare_v2(db, str("select name, attenuation, flocculation from yeasts where id = ? limit 1;"), &qry, NULL);
	sqlite3_bind_int(qry, 1, id);

	while (sqlite3_step(qry) != SQLITE_DONE)
	{
		strcpy(beer.yeasts[index].name, sqlite3_column_text(qry, 0));
		beer.yeasts[index].attenuation = sqlite3_column_double(qry, 1);
		beer.yeasts[index].flocculation = sqlite3_column_int(qry, 1);
	}
	sqlite3_finalize(qry);
}

void beer_set_ingredient_by_id(int parameter, int index, long int id)
{
	switch (parameter)
	{
		case KEY_PARAM_MALTS:
			beer_set_malt_by_id(index,id);
			break;
		case KEY_PARAM_HOPS:
			beer_set_hop_by_id(index,id);
			break;
		case KEY_PARAM_YEASTS:
			beer_set_yeast_by_id(index,id);
			break;
		default:
			return;
	}
}

void beer_print_recipe_json()
{
	int i, buf_len;
	char buffer[64];

	calculate_recipe(&beer);

	write(1, str(
		"{\n"
	));
	write(1, str("	\"name\" : \""));
	write(1, beer.name, strlen(beer.name));
	write(1, str("\",\n"));

	write(1, str("	\"author\" : \""));
	write(1, beer.author, strlen(beer.author));
	write(1, str("\",\n"));

	write(1, str("	\"volume\" : \""));
	sprintf(buffer, "%.4lf%n", beer.vol, &buf_len);
	write(1, buffer, buf_len);
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

void beer_print_recipe()
{
	int i, buf_len;
	char buffer[64];

	calculate_recipe(&beer);

	write(1, str("name : \""));
	write(1, beer.name, strlen(beer.name));
	write(1, str("\"\n"));

	write(1, str("author : \""));
	write(1, beer.author, strlen(beer.author));
	write(1, str("\"\n"));

	write(1, str("volume : "));
	sprintf(buffer, "%.4lf%n", beer.vol, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\n"));

	write(1, str("og : "));
	sprintf(buffer, "%.4lf%n", beer.og, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\t"));

	write(1, str("fg: "));
	sprintf(buffer, "%.4lf%n", beer.fg, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\n"));

	write(1, str("abv: "));
	sprintf(buffer, "%.1lf%n", beer.abv, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\t"));

	write(1, str("ibu: "));
	sprintf(buffer, "%.1lf%n", beer.ibu, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\n"));

	write(1, str("srm: "));
	sprintf(buffer, "%.1lf%n", beer.srm, &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\t"));

	write(1, str("rgb: "));
	sprintf(buffer, "%06x%n", srm_to_rgb(beer.srm), &buf_len);
	write(1, buffer, buf_len);
	write(1, str("\n"));

	write(1, str("	malts: \n"));
	for (i=0; i < beer.malt_n; i++)
	{
		write(1, str("		\""));
		write(1, beer.malts[i].name, strlen(beer.malts[i].name));
		write(1, str("\"\n"));

		write(1, str("			weight : "));
		sprintf(buffer, "%.2lf%n", beer.malts[i].mass, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\n"));
	}
	write(1, str("\n"));
	write(1, str("	hops: \n"));
	for (i=0; i < beer.hop_n; i++)
	{
		write(1, str("		\""));
		write(1, beer.hops[i].name, strlen(beer.hops[i].name));
		write(1, str("\"\n"));

		write(1, str("			weight : "));
		sprintf(buffer, "%.2lf%n", beer.hops[i].mass, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\t"));

		write(1, str("time : "));
		sprintf(buffer, "%d%n", beer.hops[i].time, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\n"));
	}
	write(1, str("\n"));
	write(1, str("	yeasts: \n"));
	for (i=0; i < beer.yeast_n; i++)
	{
		write(1, str("		\""));
		write(1, beer.yeasts[i].name, strlen(beer.yeasts[i].name));
		write(1, str("\"\n"));

		write(1, str("			amount : "));
		sprintf(buffer, "%.2lf%n", beer.yeasts[i].amount, &buf_len);
		write(1, buffer, buf_len);
		write(1, str("\n"));
	}
}
