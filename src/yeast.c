#include "yeast.h"
#include "mash.h"

double og_to_fg(double og, double attenuation)
{
	return points_to_gravity(gravity_to_points(og) * ((100.0 - attenuation)/100.0));
}

// The Charlie Papazian Method
double gravity_to_abv(double og, double fg)
{
	return (og - fg) * 131.25;
}

// The Ray Daniels Method
double gravity_to_abv_high(double og, double fg)
{
	return (76.08 * (og-fg) / (1.775-og)) * (fg / 0.794);
}
