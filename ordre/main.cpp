#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <CE_point.h>
#include <CE_curve.h>

const char* EC_PATH ="./ecparam/";
#define LENGTH_1024 1024
#define LENGTH_512 512
#define LENGTH_256 256
#define LENGTH_64 64

void pointOrder(CE_curve* curve, const CE_point* P, mpz_t** factor,  const int* power, int factorNumber, mpz_t* result);
bool loadParameter(const char* curvename, CE_curve* curve);



int main(int argc, char **argv)
{
	CE_curve* curve = new CE_curve();
	if (!loadParameter("w256-001.gp", curve)){
		printf("Cannot load curve parameter\n");
	}
	else{
	
		//input la factorisation de #E
		int r;	
		printf("Entrez le nombre de facteurs: ");
		scanf("%d", &r);
		
		char f[256];
		mpz_t* factor = new mpz_t[r];
		int power[r];
		
		//read factors and their powers
		for (int i=0;i<r;i++){
			printf("p[%d] = ", i);
			scanf("%s", f);		
			mpz_init_set_str(factor[i], f, 10);
			printf("e[%d] = ", i);
			scanf("%d", &power[i]);
		}
		
		//read point P
		CE_point* P;
		mpz_t Px;
		mpz_t Py;
		
		printf("Entrez le point P\n");
		printf("Px = ");
		scanf("%s", f);
		mpz_init_set_str(Px, f, 10);
		printf("Py = ");
		scanf("%s", f);
		mpz_init_set_str(Py, f, 10);
		P = new CE_point(&Px, &Py);	
		
		if (curve->contains(P)){
			mpz_t result;
			pointOrder(curve, P, &factor, power, r, &result);
			printf("L'ordre du point P est: %s\n", mpz_get_str(NULL, 10, result));		
		}
		else{
			printf("La courbe ne contient pas ce point\n");
		}	
	}
	return 0;
}


bool loadParameter(const char* curvename, CE_curve* curve){
	
	FILE* paramFile;
	char* filePath =  new char[LENGTH_256];
	strcpy(filePath, EC_PATH);
	strcat(filePath, curvename);
	paramFile = fopen(filePath, "r");
	if (paramFile){
		const int BUFFER_LENGTH = 100;
		char params[9][BUFFER_LENGTH];
		
		int c;
		int i=0;
		int j;
		do{			
			j=0;
			do
			{
				c = (char)fgetc(paramFile);				
			}
			while (c!='=' && c!=-1);
			if (c!=-1){
				do{					
					c = (char)fgetc(paramFile);				
					if (c>='0' && c<='9'){
						params[i][j]=c;
						j++;				
					}
					else{
						params[i][j]='\0';
						break;
					}
				}
				while (c!='\n' && c!=-1);
				i++;			
			}
		}
		while (c!=-1);
		
		mpz_init_set_str(curve->p, params[0], 10);
		mpz_init_set_str(curve->n, params[1], 10);
		mpz_init_set_str(curve->a4, params[2], 10);
		mpz_init_set_str(curve->a6, params[3], 10);		
		curve->g = new CE_point(params[6], params[7]);
		fclose(paramFile);		
		return true;
	}
	else{	
		return false;
	}
}

void pointOrder(CE_curve* curve, const CE_point* P, mpz_t** factor,  const int* power, int factorNumber, mpz_t* result){
	
	mpz_t mod;
	mpz_init_set(*result, curve->n);
	mpz_init_set(mod, curve->n);
	mpz_add_ui(mod, mod, 1);
	
	CE_point* Q;
		
	for (int i=0;i<factorNumber;i++){
		mpz_t tmp;
		mpz_init_set(tmp, *(factor[i]));		
		mpz_powm_ui(tmp, tmp, power[i], mod);	
		mpz_tdiv_q(*result, *result, tmp);		
		Q = curve->mul(P, result);		
		while (!Q->inf){			
			Q = curve->mul(Q, factor[i]);			
			mpz_mul(*result, *result, *(factor[i]));			
		}
	}	
}
