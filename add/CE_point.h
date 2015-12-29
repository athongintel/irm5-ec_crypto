#ifndef CE_POINT_H
#define CE_POINT_H

#include <gmp.h>

class CE_point
{
public:
	mpz_t x;
	mpz_t y;
	bool inf;

	CE_point();
	CE_point(const mpz_t* xpos, const mpz_t* ypos);
	CE_point(char* x_str, char* y_str);
	~CE_point();

};

#endif // CE_POINT_H

