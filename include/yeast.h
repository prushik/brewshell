double og_to_fg(double og, double attenuation);
double gravity_to_abv(double og, double fg);
double gravity_to_abv_high(double og, double fg);

#define YEAST_FLOC_NONE		0
#define YEAST_FLOC_XLOW		1
#define YEAST_FLOC_LOW		2
#define YEAST_FLOC_MIDLOW	3
#define YEAST_FLOC_MEDIUM	4
#define YEAST_FLOC_MIDHIGH	5
#define YEAST_FLOC_HIGH		6
#define YEAST_FLOC_EXTREME	7
