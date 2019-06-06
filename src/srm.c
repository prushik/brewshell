#include "srm.h"

double mcu_to_srm(double mcu)
{
	return 1.49 * (mcu * 0.69);
}

unsigned int srm_to_rgb(double mcu)
{
	double m2 = mcu * mcu;
	double m3 = mcu * mcu * mcu;

	return
		srm_to_r(mcu, m2) << 16 |
		srm_to_g(mcu, m2, m3) << 8  |
		srm_to_b(mcu, m2, m3);
}

// (0.028027*x^3) - (2.02009*x^2) + (43.0636*x) + 30
unsigned char srm_to_b(double mcu, double mcu2, double mcu3)
{
	double a = mcu * 43.0636;
	double b = mcu2 * 2.02009;
	double c = mcu3 * 0.028027;
	short y = 255 - (a - b + c + 30);
	if (y<0) y=0;
	if (y>255) y = 255;
	return y;
}

// (0.00251199 * x^3) - (0.355374 * x^2) + (16.4441*x)
unsigned char srm_to_g(double mcu, double mcu2, double mcu3)
{
	double a = mcu * 16.4441;
	double b = mcu2 * 0.355374;
	double c = mcu3 * 0.002512;
	short y = 255 - (a - b + c);
	if (y<0) y = 0;
	if (y>255) y = 255;
	return y;
}

// (0.0327209 * x^2) + (4.09636*x)
unsigned char srm_to_r(double mcu, double mcu2)
{
	double a = mcu * 4.09636;
	double b = mcu2 * 0.0327209;
	short y = 255 - (a + b);
	if (y<0) y = 0;
	if (y>255) y = 255;
	return y;
}
