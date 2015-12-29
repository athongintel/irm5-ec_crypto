#ifndef CE_CURVE_H
#define CE_CURVE_H

#include <CE_point.h>
#include <gmp.h>

class CE_curve
{
public:
	mpz_t p;
	mpz_t a4;
	mpz_t a6;
	mpz_t n;
	CE_point* g;
	
	//constructors
	CE_curve(const char* p, const char* n, const char* a4, const char* a6, const CE_point* g);

	//operations
	CE_point* add(const CE_point* P, const CE_point* Q);
	CE_point* mul(const CE_point* P, const mpz_t* k);
	CE_point* dbl(const CE_point* P);	
	CE_point* inverse(const CE_point* P);
	
	~CE_curve();

};

#endif // CE_CURVE_H
