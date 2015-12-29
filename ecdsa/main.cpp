#include <stdio.h>
#include <cstring>
#include <exception>
#include <openssl/aes.h>
#include <gmp.h>

#include "CE_point.h"
#include "CE_curve.h"
#include "sha256.h"
#include "utils.h"
#include "ECDSA.h"

#define MESSAGE_MAX_LENGTH 2048
#define NAME_MAX_LENGTH 256
#define KEY_MAX_LENGTH 512

const char* EC_PATH ="./ecparam/"; //path to save ec curve parameters
CE_curve* ec_curve;

int loadParameter(const char* curvename){
	FILE* paramFile;
	char* filePath =  new char[NAME_MAX_LENGTH];
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
		
		ec_curve = new CE_curve(
		params[0],  //p
		params[1],  //n
		params[2],  //a4
		params[3],  //a6
		new CE_point(
			params[6], //gx
			params[7] //gy
		));
		
		
		fclose(paramFile);		
		return 0;
	}
	else{	
		return 1;
	}
}

/*
int test_curve(CE_curve* curve, int number){
	mpz_t factor;
	mpz_init(factor);
	int count=0;
	for (int i=0;i<number;i++){		
		mpz_set_ui(factor, i);
		CE_point* point = curve->mul(curve->g, &factor);
		CE_point* infini = curve->mul(point, &curve->n);
		if (infini->inf) count++;
	}
	return count==number;
}
*/

int main(int argc, char **argv)
{
	/*
	 * ECDSA using:
	 * 1. help function
	 * ecdsa
	 * ecdsa -h
	 * ecdsa --help
	 * 
	 * 2. keypair generation
	 * ecdsa -k [curve name] [key pair path]
	 * ecdsa --keygen [curve name] [key pair path]
	 *
	 * 3. signature generation
	 * ecdsa -s <private key path> <based64 data>
	 * ecdsa --signature <private key path> <based64 data>
	 *  
	 * 4. signature verification
	 * ecdsa -v <public key path> <message> <signature>
	 * ecdsa --verify <public key path> <message> <signature>
	 * */
	if (argc==1 || strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0){
		printf("ECDSA SIGNATURE GENERATION AND VERIFICATION");
		printf("\nThis program is used for generating and verifying an ecdsa signature\n");		
		printf("\nDETAILS\n");
		printf("\n\t -h, --help");		
		printf("\n\t\t Display this help");
		printf("\n\t -k, --keygen");
		printf("\n\t\t [curve name] [key pair path]");
		printf("\n\t\t Generate a pair of asymetric key");
		printf("\n\t -s, --signature");
		printf("\n\t\t <private key path> <based64 data>");
		printf("\n\t\t Generate a signature");
		printf("\n\t -v, --verify");
		printf("\n\t\t <public key path> <message> <signature>");
		printf("\n\t\t Verify a signature\n");

		return 0;
	}
	else if (strcmp(argv[1],"-k")==0 || strcmp(argv[1],"--keygen")==0){
		
		printf("EC key pair generation");		
		
		char* curvename = new char[NAME_MAX_LENGTH];	
		if (argv[2]!=NULL && argc>=2){			
			strcpy(curvename, argv[2]);
		}
		else{
			printf("\nInput curve name: ");
			scanf("%s",curvename);
		}
		
		printf("\nLoading key parameters...");
		if (loadParameter(curvename)){
			printf("\nInvalid parameter file %s %d", curvename, strlen(curvename));
			goto error;
		}
		printf("OK");
		
		printf("\nTesting key parameters...");
		//cheking for length
			
		int requested_security_strength = ec_curve->getRequestSecurityLength();
		if (requested_security_strength<0){
			printf("\nNot a secure length!");
			goto error;
		}
		printf("OK");
		
		//random state, seed from urandom
		gmp_randstate_t randomstate;
		gmp_randinit_default(randomstate);		
		mpz_t seed;
		mpz_init(seed);
		
		int N = ec_curve->getRequestSecurityLength();
		utils::getRandomNumber(&seed, N/8);
		gmp_randseed(randomstate, seed);
				
		mpz_t random_c;
		mpz_init(random_c);
		mpz_rrandomb(random_c, randomstate, N+64);
			
		//calculate d = (c mod (n-1))+1
		mpz_t final_d;
		mpz_t tmp;
		mpz_init(final_d);
		mpz_init_set(tmp, ec_curve->n);
		mpz_sub_ui(tmp, tmp, 1);
		mpz_mod(final_d, random_c, tmp);
		mpz_add_ui(final_d, final_d, 1);
		
		//calculate Q = dG
		CE_point* Q = ec_curve->mul(ec_curve->g, &final_d);
		
		//check if Q is still one the curve and order n
		CE_point* tp = ec_curve->mul(Q, &ec_curve->n);
		if (!ec_curve->contains(Q)) printf("\nnot contain");
		if (!tp->inf) printf("\nnot order n");
		
		char* filename = new char[NAME_MAX_LENGTH];
		char* pubfilename = new char[NAME_MAX_LENGTH];
		
		if (argv[3]!=NULL && argc>=3){
			strcpy(filename,argv[3]);			
		}
		else{
			printf("\nEnter the file in which to save the key: ");
			scanf("%s",filename);
		}
		
		strcpy(pubfilename, filename);
		strcat(pubfilename, ".pub");		
		
		FILE* file;
		//write public key to file
		char* base64EncodeOutput;
		file = fopen(pubfilename, "wt");
		if (file){
			fprintf(file,"=====ecdsa-public-key-scheme=====\n");
			fprintf(file,"%s\n", curvename);
			//write Q point
			
			char* pubkey;			
			pubkey = mpz_get_str(NULL, 10, Q->x);			
			utils::Base64Encode((unsigned char*)pubkey, strlen(pubkey), &base64EncodeOutput);
			fprintf(file, "%s\n", base64EncodeOutput);
			pubkey=mpz_get_str(NULL, 10, Q->y);			
			utils::Base64Encode((unsigned char*)pubkey, strlen(pubkey), &base64EncodeOutput);
			fprintf(file, "%s\n", base64EncodeOutput);
			fprintf(file,"================end==============\n");
			fclose(file);
		}
		else{
			printf("\nError: Cannot open file '%s' for writing key", pubfilename);
			goto error;
		}
		//write private key	to file
		file = fopen(filename, "wt");
		if (file){
			fprintf(file,"=====ecdsa-private-key-scheme=====\n");
			fprintf(file,"%s\n", curvename);
			//write d
			char* prikey;
			unsigned char* uprikey;
			prikey = mpz_get_str(NULL, 10, final_d);						
			utils::Base64Encode((unsigned char*)prikey, strlen(prikey), &base64EncodeOutput);
			fprintf(file, "%s\n", base64EncodeOutput);			
			fprintf(file,"================end==============\n");			
			fclose(file);
			printf("\nKey pair generation succeeded\n");
			return 0;
		}
		else{
			printf("\nError: Cannot open file '%s' for writing key", filename);
			goto error;
		}
	}
	else if (strcmp(argv[1],"-s")==0 || strcmp(argv[1],"--signature")==0){
		//EC signature generation

		if (argc<4) goto error;
		//extract curve name from private key
		char* curvename = new char[NAME_MAX_LENGTH];
		char* prikeybase64 = new char[NAME_MAX_LENGTH];
		if (argv[2]!=NULL && argc>=2){
			//open file to read curvename and private key
			FILE* file;
			file = fopen(argv[2],"rt");
			if (file){
			fscanf(file, "%s", curvename); //remove first line
			fscanf(file, "%s", curvename);
			fscanf(file, "%s", prikeybase64);			
			fclose(file);
			}
			else{
				printf("\nError: cannot open private key!");
				goto error;
			}
		}
		else{			
			goto error;
		}
		
		if (loadParameter(curvename)){
			printf("\nInvalid parameter file");
			goto error;
		}
	
		//cheking for length	
		int requested_security_strength = ec_curve->getRequestSecurityLength();		
		if (requested_security_strength<0){
			printf("\nNot a secure length!");
			goto error;
		}
		
		//retrieve private key from base64
		unsigned char* prikey;// = new unsigned char[256];				
		size_t length;
		if (!utils::Base64Decode(prikeybase64, &length, &prikey)){
			printf("\nError: bad base64 private key encoding");
			goto error;
		}
		
		//SIGNATURE GENERATION
		mpz_t p;
		mpz_init_set_str(p, (char*)prikey, 10);
		ECDSA ecdsa = ECDSA(ec_curve, NULL, &p);
		//1. decode the message
		unsigned char* messageDecoded;// = new unsigned char[MESSAGE_MAX_LENGTH];		
		if (!utils::Base64Decode(argv[3], &length, &messageDecoded))
		{
			printf("\nError: bad base64 message encoding");
			goto error;
		}
		
		//2. generate signature
		char signature[MESSAGE_MAX_LENGTH];
		ecdsa.generateSignature((char*)messageDecoded, signature);
		
		//3. print-out signature
		printf("%s\n", signature);
		return 0;
	}
	else if (strcmp(argv[1],"-v")==0 || strcmp(argv[1],"--verify")==0){
		//EC signature verification

		if (argc<5) goto error;
		//extract curve name from public key
		char* curvename = new char[256];
		char* publicbase64x = new char[256];
		char* publicbase64y = new char[256];
		if (argv[2]!=NULL && argc>=2){
			//open file to read curvename and public key
			FILE* file;
			file = fopen(argv[2],"rt");
			if (file){
			fscanf(file, "%s", curvename); //remove first line
			fscanf(file, "%s", curvename);
			fscanf(file, "%s", publicbase64x);
			fscanf(file, "%s", publicbase64y);			
			fclose(file);
			}
			else{
				printf("\nError: cannot open public key!");
				goto error;
			}
		}
		else{			
			goto error;
		}		
		
		if (loadParameter(curvename)){
			printf("\nInvalid parameter file");
			goto error;
		}
	
		//cheking for key length		
		int requested_security_strength = ec_curve->getRequestSecurityLength();
		if (requested_security_strength<0){
			printf("\nNot a secure length!");
			goto error;
		}
		
		//retrieve public key from base64
		unsigned char* pubkeyx;// = new unsigned char[256];			
		unsigned char* pubkeyy;// = new unsigned char[256];
		size_t length;
		if (!utils::Base64Decode(publicbase64x, &length, &pubkeyx)){
			printf("\nError: bad base64 public key encoding");
			goto error;
		}
		if (!utils::Base64Decode(publicbase64y, &length, &pubkeyy)){
			printf("\nError: bad base64 public key encoding");
			goto error;
		}
		
		CE_point* publicKey = new CE_point((char*) pubkeyx, (char*) pubkeyy);
		
		//SIGNATURE VERIFICATION
		//1. decode base64 the message and signature						
		unsigned char* messageDecoded;				
		if (!utils::Base64Decode(argv[3], &length, &messageDecoded))
		{
			printf("\nError: bad base64 message encoding");
			goto error;
		}		

		unsigned char* signatureDecoded;
		if (!utils::Base64Decode(argv[4], &length, &signatureDecoded))
		{
			printf("\nError: bad base64 signature encoding");
			goto error;
		}
		
		ECDSA ecdsa = ECDSA(ec_curve, publicKey, NULL);
		if (ecdsa.verifySignature((char*)messageDecoded, (char*)signatureDecoded))
		{	
			printf("\nSignature is CORRECT!\n");
				return 0;
		}
		else{
			printf("\nSignature is WRONG!\n");
				return 1;
		}
	}
	else{
		goto error;
	}	

	error:
		printf("\nWrong usage of ecdsa. Use -h or --help for help\n");
		return 1;
}