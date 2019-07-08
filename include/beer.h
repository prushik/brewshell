struct malt
{
	char name[32];       		// name of malt
	double mass;         		// in lbs
	double potential;    		// gravity potential (1.046 for pure sugar)
	double pts_potential;		// points potential (46 for pure sugar)
	double mcu;          		// Malt color unit
};

struct hop
{
	char name[32];       		// name of hop
	double mass;         		// in oz
	int time;            		// in seconds
	double utilization;  		// for rager method
	double alpha;        		// in percent
};

struct yeast
{
	char name[32];       		// name of hop
	double amount;       		// in packages
	int time;            		// in seconds
	double attenuation;  		// in percent
	int flocculation;    		// 
};

struct recipe
{
	char name[32];       		// beer name
	char author[32];     		// beer maker's name
	double vol;          		// volume (gallons)
	double bg,og,fg;     		// gravity
	double ibu;          		// bitterness
	double mash_eff;     		// mash efficiency
	double srm;          		// SRM color
	double abv;          		// alcohol by volume
	int hop_n;           		// num of hops
	struct hop *hops;    		// array of hops
	int malt_n;          		// num of malts
	struct malt *malts;  		// array of malts
	int yeast_n;         		// num of yeasts
	struct yeast *yeasts;		// array of yeasts
};

#define ING_TYPE_MALT     0
#define ING_TYPE_HOPS     1
#define ING_TYPE_YEAST    2

#define HOP_TYPE_PELLET   0
#define HOP_TYPE_WHOLE    1

#define DATABASE          "/var/db/brewshell.sqlite"
