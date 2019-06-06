#include "beer.h"
#include "ibu.h"
#include "mash.h"
#include "srm.h"
#include "yeast.h"

void calculate_recipe(struct recipe * beer)
{
	int i;
	double g_pts = 0, ibu = 0, mcu = 0;
	for (i=0; i < beer->malt_n; i++)
	{
		g_pts += potential_points_to_gravity(beer->vol, beer->malts[i].mass, beer->malts[i].pts_potential);
		mcu += (beer->malts[i].mcu * beer->malts[i].mass) / beer->vol;
	}
	beer->srm = mcu_to_srm(mcu);
	beer->og = points_to_gravity(0.7*g_pts);
	for (i=0; i < beer->hop_n; i++)
		ibu += hop_to_tinseth(beer->vol, beer->hops[i].mass, beer->hops[i].alpha/100.0, beer->og, beer->hops[i].time);
	beer->ibu = ibu * 1.1; // * 1.1 if pellets
	for (i=0; i < beer->yeast_n; i++)
		beer->fg = og_to_fg(beer->og, beer->yeasts[i].attenuation);
	beer->abv = gravity_to_abv(beer->og, beer->fg);
//	ibu = hop_to_rager(5, 0.01, 5, 1.06, 0.211);
}
