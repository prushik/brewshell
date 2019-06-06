#include <math.h>
#include "beer.h"
#include "ibu.h"

// tinseth method
// ibu = (1.65 * 0.000125^(Ggravity - 1)) * ((1 - e^(-0.04*time)) * a * m * 1000
// ibu = (1.65*0.000125^(gravity-1))*(1-EXP(-0.04*time))*alpha*mass*1000
double hop_to_tinseth(double vol, double mass, double alpha, double og, double minutes)
{
	return 1.65 * pow(0.000125, og - 1.0) * 
			(1.0 - exp(-0.04 * minutes)) * alpha * mass * 28.3 * 1000 / 
			(3.785 * vol * 4.15);
}

// rager method
// ibu = (Woz * U * a * 7489) / (Vgal * gravity)
double hop_to_rager(double vol, double mass, double alpha, double og, double utilization)
{
	return (mass * utilization * alpha * 7489.0) / (vol * og);
}

