#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "beer.h"
#include "constants.h"

#include "mainspring/host_config.h"
#include "mainspring/lang_config.h"

#define str(x) x,sizeof(x)

static struct recipe beer = {0};

void beer_init()
{
	
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

double beer_get_float(int parameter)
{
	
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
			beer.yeasts = realloc(beer.hops, sizeof(struct yeast)*beer.yeast_n);
			return beer.yeast_n;
			break;
		default:
			return 0;
	}
}

void beer_print_recipe()
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
