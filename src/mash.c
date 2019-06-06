#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "beer.h"
#include "mash.h"

double gravity_to_points(double g)
{
	return (g - 1.00) * 1000.0;
}

double points_to_gravity(double pts)
{
	return (pts / 1000.0) + 1.00;
}

double potential_to_gravity(double vol, double mass, double potential)
{
	return points_to_gravity((gravity_to_points(potential) * mass) / vol);
}

double potential_points_to_gravity(double vol, double mass, double pts_potential)
{
	return (pts_potential * mass) / vol;
}
