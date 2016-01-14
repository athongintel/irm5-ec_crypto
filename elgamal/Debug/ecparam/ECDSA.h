#ifndef ECDSA_H
#define ECDSA_H

#include "CE_curve.h"
#include "CE_point.h"
#include "sha256.h"
#include "utils.h"

class ECDSA
{
	
private:
	CE_curve* curve;
	CE_point* pubKey;
	mpz_t* priKey;
	
public:
	ECDSA();
	ECDSA(CE_curve* curve, CE_point* pubKey, mpz_t* priKey);
	int generateSignature(const char* message, char* signature);
	bool verifySignature(const char* messages, const char* signature);
	~ECDSA();
};

#endif // ECDSA_H
