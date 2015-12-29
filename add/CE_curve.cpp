#include "CE_curve.h"

CE_curve::CE_curve(const char* p, const char* n, const char* a4, const char* a6, const CE_point* g)
{
	mpz_init_set_str(this->p, p, 10);
	mpz_init_set_str(this->n, n, 10);
	mpz_init_set_str(this->a4, a4, 10);
	mpz_init_set_str(this->a6, a6, 10);
	this->g = new CE_point(&g->x, &g->y);	
}

CE_point* CE_curve::add(const CE_point* P, const CE_point* Q){	
	CE_point* result;

	if (P->inf || Q->inf){		
		if (!P->inf){
			//return p1
			result = new CE_point(&P->x, &P->y);
		}
		else if (!Q->inf){
			result = new CE_point(&Q->x, &Q->y);			
		}
		else{
			result = new CE_point();
			result->inf = true;
		}		
		return result;
	}
	else if (mpz_cmp(P->x, Q->x)==0){
		if (mpz_cmp(P->y, Q->y)==0){
			//two identical points
			return dbl(P);
		}
		else{
			result = new CE_point();
			result->inf = true;
			return result;
		}
	}
	else{
		//compute the slope		
		mpz_t s;
		mpz_t denom;
		
		mpz_init_set(s, P->y);
		mpz_sub(s, s, Q->y);
	
		mpz_init_set(denom, P->x);			
		mpz_sub(denom, denom, Q->x);		
		mpz_invert(denom, denom, this->p);		
		mpz_mul(s, s, denom);
		mpz_mod(s, s, this->p);
		
		//compute R coordinate
		mpz_t xR;
		mpz_t yR;

		//xR = s^2 - (xP + xQ)
		mpz_init(xR);
		mpz_powm_ui(xR, s, 2, this->p);
		mpz_sub(xR, xR, P->x);
		mpz_sub(xR, xR, Q->x);
		mpz_mod(xR, xR, this->p);
		
		//yR = s(xP - xR) - yP
		mpz_init_set(yR, P->x);
		mpz_sub(yR, yR, xR);
		mpz_mul(yR, yR, s);
		mpz_sub(yR, yR, P->y);
		mpz_mod(yR, yR, this->p);
		
		result = new CE_point(&xR, &yR);
		return result;
	}
	
}

CE_point* CE_curve::mul(const CE_point* P, const mpz_t* k){	
	//compute inverse binary
	mpz_t b_inverse;
	mpz_t tmp;
	
	mpz_init(b_inverse);	
	mpz_init_set(tmp, *k);
	
	int bitlength=0;
	while (mpz_sgn(tmp)>0){
		bitlength++;
		mpz_mul_ui(b_inverse, b_inverse, 2);
		if (mpz_odd_p(tmp)){
			mpz_add_ui(b_inverse, b_inverse, 1);
		}
		mpz_fdiv_q_ui(tmp, tmp, 2);
	}
	
	CE_point* result = new CE_point();
	
	while (mpz_sgn(b_inverse)>0 || bitlength){
		bitlength--;		
		result = dbl(result);
		if (mpz_odd_p(b_inverse)){
			result = add(result, P);
		}
		mpz_fdiv_q_ui(b_inverse, b_inverse, 2);
	}
	return result;
}

CE_point* CE_curve::dbl(const CE_point* P){	
	
	CE_point* result;
	if (P->inf || mpz_sgn(P->y)==0){
		result = new CE_point();
		result->inf = true;
		return result;
	}
	else{		
		//compute the slope
		mpz_t s;
		mpz_init(s);
		mpz_powm_ui(s, P->x, 2, this->p);
		mpz_mul_ui(s, s, 3);
		mpz_add(s, s, this->a4);
		
		mpz_t denom;
		mpz_init_set(denom, P->y);
		mpz_mul_ui(denom, denom, 2);
		mpz_invert(denom, denom, this->p);
		mpz_mul(s, s, denom);
		
		//compute xR and yR
		mpz_t xR;
		mpz_t yR;
		
		//xR = s^2 - 2xP
		mpz_init(xR);
		mpz_powm_ui(xR, s, 2, this->p);
		mpz_sub(xR, xR, P->x);
		mpz_sub(xR, xR, P->x);
		mpz_mod(xR, xR, this->p);
		
		//yR = s(xP - xR) - yP
		mpz_init_set(yR, P->x);
		mpz_sub(yR, yR, xR);
		mpz_mul(yR, yR, s);
		mpz_sub(yR, yR, P->y);
		mpz_mod(yR, yR, this->p);
		
		return new CE_point(&xR, &yR);
	}
}

CE_point* CE_curve::inverse(const CE_point* p){
	mpz_t y_inverse;
	mpz_init(y_inverse);
	mpz_neg(y_inverse, p->y);	
	return new CE_point(&p->x, &y_inverse);
}


CE_curve::~CE_curve()
{
}

