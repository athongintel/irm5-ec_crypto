#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/aes.h>
#include <netdb.h>
#include <math.h>

#include "CE_curve.h"
#include "CE_point.h"
#include "utils.h"
#include "ECDSA.h"
#include "AES.h"
 
bool working;
bool requestDisconnect;
const char* EC_PATH ="./ecparam/";
const char* REQUEST_DISCONNECT = "[exit]";

#define SERVER_PORT 9192
#define MAX_ACCEPT_CONNECTIONS 10
#define LENGTH_1024 1024
#define LENGTH_512 512
#define LENGTH_256 256
#define LENGTH_64 64

void printUnsigned(unsigned char* data){
	printf("\n");
	int length = strlen((char*)data);
	for (int i=0;i<length;i++){
		printf("%d ", data[i]);
	}
	printf("\n");
}

void* listenThread(void* args){
	char* arguments = (char*) args;
	char* split[2];
	char* buffer = new char[LENGTH_1024];
	unsigned char* unsignedbuffer = new unsigned char[LENGTH_1024];
	unsigned char* base64decoded;
	char decryptedMessage[LENGTH_1024];
	
	utils::split(arguments, ';', split);	
	//decode base64 aeskey		
	size_t size;
	utils::Base64Decode(split[1], &size, &base64decoded);
	
	//socketID for I/O
	int socketID = atoi(split[0]);
	//aeskey for decryption
	AES* aes = new AES(base64decoded);
		
	do{
		//1. read from client
		int readResult = read(socketID, buffer, LENGTH_1024);
		if (readResult>0){
			//decode base64
			if (!aes->cbc_decrypt(buffer, decryptedMessage)){
				printf(">>[Message damaged]\n");
			}
			else{
				printf(">>%s\n", decryptedMessage);				
			}				
			fflush(stdout);	
			if (strcmp(decryptedMessage, REQUEST_DISCONNECT)==0){				
				printf("\nCommunication terminated!\n");				
				fflush(stdout);
				exit(0);
			}
		}
	}
	while (true);
}

void writeThread(AES* aes, int socketID){
	printf("\nInput something to send:\n");
	char encrypted[LENGTH_1024];
	char input[LENGTH_1024];
	int writeResult;
	
	while (true){		
		fgets(input, LENGTH_1024, stdin);
		input[strlen(input)-1] = '\0'; //remove enter character
		//encrypt by AES and send		
		aes->cbc_encrypt(input, encrypted);
		writeResult = write(socketID, encrypted, strlen(encrypted));
		if (writeResult<=0){
			printf("\nError: cannot write to buffer");
			fflush(stdout);
		}
		if (strcmp(input, REQUEST_DISCONNECT)==0){
			printf("\nCommunication terminated!\n");			
			fflush(stdout);
			exit(0);
		}
	}
}

int loadParameter(const char* curvename, CE_curve* curve){
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
		return 0;
	}
	else{	
		return 1;
	}
}

int serversideSecureConnection(int serverSocketID, int clientSocketID, const char* serverKeyPath, const char* clientKeyPath){
	//temp variables
	char* buffer = new char[LENGTH_1024];
	
	char* base64encoded;// = new char[LENGTH_1024];
	unsigned char* base64decoded;// = new unsigned char[LENGTH_1024];
	unsigned char* base64decoded2;// = new unsigned char[LENGTH_1024];
		
	char* twoPoint[2];
	char servercurvename[LENGTH_256];	
	char clientcurvename[LENGTH_256];
	char serverPrivateKeyBase64[LENGTH_256];
	char clientPublicKeyX[LENGTH_256];
	char clientPublicKeyY[LENGTH_256];
	char signature[LENGTH_1024];
	unsigned char aeskey[LENGTH_256];
	unsigned char encryptedMessage[LENGTH_1024];
	unsigned char decryptedMessage[LENGTH_1024];	
	char iv[LENGTH_64];
	char ivcopy[LENGTH_64];
	
	CE_curve* curve = new CE_curve();
	size_t base64size;
	SHA256 sha256 = SHA256();	
	ECDSA* ecdsa;
	AES* aes;
	//variables
	
	FILE* serverKeyFile = fopen(serverKeyPath, "rt");
	FILE* clientKeyFile = fopen(clientKeyPath, "rt");
	
	CE_point* gx;
	CE_point* gy;
	CE_point* gxy;	
	CE_point* clientPublicKeyP;
	mpz_t randomy;
	
	//1. read parameters from file
	printf("\nInitializing...");
	fflush(stdout);
	//1.1. read server private key	
	if (!serverKeyFile){
		printf("\nError: error reading server private key");
		goto closeConnection;
	}	
	//1.1.1. read curve name and private key
	fscanf(serverKeyFile, "%s", servercurvename); //ignore first line
	fscanf(serverKeyFile, "%s", servercurvename);
	fscanf(serverKeyFile, "%s", serverPrivateKeyBase64);
	fclose(serverKeyFile);	
	//1.2. read client pubic key		
	if (!clientKeyFile){
		printf("\nError: error reading client public key");
		goto closeConnection;
	}
	//1.2.1. read curve name and public key
	fscanf(clientKeyFile, "%s", clientcurvename); //ignore first line
	fscanf(clientKeyFile, "%s", clientcurvename); //read curve name
	fscanf(clientKeyFile, "%s", clientPublicKeyX); //read first point
	fscanf(clientKeyFile, "%s", clientPublicKeyY); //read second point
	fclose(clientKeyFile);	
	//1.3 init variable
	mpz_t serverPrivateKey;
	if (!utils::Base64Decode(serverPrivateKeyBase64, &base64size, &base64decoded)){
		printf("\nError: server private key damaged");
		goto closeConnection;
	}
	mpz_init_set_str(serverPrivateKey, (char*) base64decoded, 10);
	
	mpz_t clientPublicKeyPx;
	if (!utils::Base64Decode(clientPublicKeyX, &base64size, &base64decoded)){
		printf("\nError: client public key damaged");
		goto closeConnection;
	}
	mpz_init_set_str(clientPublicKeyPx, (char*) base64decoded, 10);
	
	mpz_t clientPublicKeyPy;
	if (!utils::Base64Decode((char*) clientPublicKeyY, &base64size, &base64decoded)){
		printf("\nError: client public key damaged");
		goto closeConnection;
	}
	mpz_init_set_str(clientPublicKeyPy, (char*) base64decoded, 10);	
	clientPublicKeyP = new CE_point(&clientPublicKeyPx, &clientPublicKeyPy);
	
	//2. check parameters
	if (strcmp(clientcurvename, servercurvename)!=0){
		printf("\nError: client and server not share the same paramters");
		goto closeConnection;
	}	
	if (loadParameter(clientcurvename, curve)){
		printf("\nError: error loading paramters");
		goto closeConnection;
	}
	
	//3. STARTING PROTOCOL	
	int readResult;
	//3.1. waiting for gx	
	bzero(buffer,LENGTH_1024);
	do{
		readResult = read(clientSocketID,buffer,LENGTH_1024);
	}
	while (readResult==0);
	if (readResult < 0){
		printf("\nError: error reading gx");
		goto closeConnection;
	}
	printf("\nReading gx...");
	fflush(stdout);
	
	//3.1.1. convert g.x from base64 to number
	if (!utils::Base64Decode(buffer,  &base64size, &base64decoded)){
		printf("\nError: gx base64 encode");
		goto closeConnection;
	}
	
	utils::split((char*)base64decoded, ';', twoPoint);

	gx = new CE_point(twoPoint[0], twoPoint[1]);
	
	//3.2 calculate random number y
	utils::getRandomNumber(&randomy, curve->getRequestSecurityLength());
	//3.3. calculate gy
	gy = curve->mul(curve->g, &randomy);
	//3.4. calculate gxy
	gxy = curve->mul(gx, &randomy);
	
	//3.5. compute and send gy, Ek(S(gy, gx))
	//3.5.1 concat gy,gx	
	strcpy(buffer, mpz_get_str(NULL, 10, gy->x));
	strcat(buffer, ";");
	strcpy(buffer, mpz_get_str(NULL, 10, gy->y));
	strcat(buffer, ";");
	strcat(buffer, mpz_get_str(NULL, 10, gx->x));	
	strcat(buffer, ";");
	strcpy(buffer, mpz_get_str(NULL, 10, gy->y));
	ecdsa = new ECDSA(curve, NULL, &serverPrivateKey);
	
	ecdsa->generateSignature(buffer, signature);
	//printf("\nSignature: %s", signature);
	//printf("\nSignature length: %d ", strlen(signature));
	//printUnsigned((unsigned char*) signature);
	
	//3.5.2. generating key for AES by hashing the K = gxy	
	strcpy(buffer, mpz_get_str(NULL, 10, gxy->x));
	strcat(buffer, mpz_get_str(NULL, 10, gxy->y));
	
	bzero(aeskey, LENGTH_256);
	sha256.init();
	sha256.update((unsigned char*) buffer, strlen(buffer));
	sha256.final(aeskey);
	//printf("\nAES key length: %d", strlen((char*) aeskey)*8);
	//printUnsigned(aeskey);
	
	//3.5.3. use AES 256 to encrypt message	
	AES_KEY aes_enc_key;
	AES_set_encrypt_key(aeskey, strlen((char*) aeskey)*8, &aes_enc_key);
	//generate an IV	
	bzero(iv, LENGTH_64);
	utils::getRandomString(iv, 32);
	//save a copy of iv, which will be modified after the encryption
	strcpy(ivcopy, iv);
	
	//pre-compute the encryption length
	int encryptionLength;
	encryptionLength = ceil(strlen(signature)/16.0)*16;
	bzero(encryptedMessage, LENGTH_1024);
	AES_cbc_encrypt((unsigned char*) signature, encryptedMessage, encryptionLength, &aes_enc_key, (unsigned char*) iv, AES_ENCRYPT);	
	
	//3.5.4. send as gx;gy;signature;iv
	utils::Base64Encode(encryptedMessage, encryptionLength, &base64encoded);	
		
	//3.5.4.1 concat gy, encryptedMessage
	char finalMessage[LENGTH_1024];
	strcpy(finalMessage, mpz_get_str(NULL, 10, gy->x));
	strcat(finalMessage, ";");
	strcat(finalMessage, mpz_get_str(NULL, 10, gy->y));
	strcat(finalMessage, ";");
	strcat(finalMessage, base64encoded);
	//3.5.4.2 convert concat iv
	utils::Base64Encode((unsigned char*)ivcopy, 32, &base64encoded);		
	strcat(finalMessage, ";");
	strcat(finalMessage, base64encoded);	
	utils::Base64Encode((unsigned char*) finalMessage, strlen((char*) finalMessage), &base64encoded);
	
	//3.5.5 send finalMessage
	printf("\nSending signature...");
	fflush(stdout);
	int writeResult;
	writeResult = write(clientSocketID, base64encoded, strlen(base64encoded));
	if (writeResult<0){
		printf("\nError: cannot write to buffer");
		goto closeConnection;
	}
	
	//3.6. reading client signature
	printf("\nReading signature...");
	fflush(stdout);
	do{
		readResult = read(clientSocketID,buffer,LENGTH_1024);
	}
	while (readResult==0);	
	
	//3.7 verify client signature
	printf("\nVerifying signature...");
	fflush(stdout);
	//3.7.1. decode message

	if (!utils::Base64Decode(buffer, &base64size, &base64decoded)){
		printf("\nError: bad encoding or signature damaged");
		goto closeConnection;
	}
	
	//3.7.2. split and decode signature and iv
	char* split[2];
	utils::split((char*) base64decoded, ';', split);
	
	if (!utils::Base64Decode(split[0], &base64size, &base64decoded)){
		printf("\nError: signature damaged");
		goto closeConnection;
	}
	if (!utils::Base64Decode(split[1], &base64size, &base64decoded2)){
		printf("\nError: iv damaged");
		goto closeConnection;
	}
	
	//3.7.3. using AES to decrypt signature
	AES_KEY aes_dec_key;
	AES_set_decrypt_key(aeskey, LENGTH_256, &aes_dec_key);
	AES_cbc_encrypt(base64decoded, decryptedMessage, LENGTH_1024, &aes_dec_key, base64decoded2, AES_DECRYPT);

	//3.3.4 verify the signature
	ecdsa = new ECDSA(curve, clientPublicKeyP, NULL);
	if (!ecdsa->verifySignature(buffer, (char*) decryptedMessage)){
		printf("\nError: signature verification fail");
		goto closeConnection;
	}		
	
	//4. start communication
	printf("\nSecure connection established!");
	fflush(stdout);	
	//4.1. create a thread to listen	
	pthread_t listeningThread;	
	//4.2. parameter: port;base64(aeskey)
	char args[LENGTH_256];
	sprintf(args, "%d", clientSocketID);
	//printf("\nMain thread socketID: %d", clientSocketID);
	strcat(args, ";");
	utils::Base64Encode(aeskey, 32, &base64encoded);
	strcat(args, base64encoded);
	if (pthread_create(&listeningThread, NULL, listenThread, args)){
		printf("\nError: cannot create listening thread");
		goto closeConnection;
	}
	
	//4.2. read and send until reach REQUEST_DISCONNECT
	working = true;
	aes = new AES(aeskey);	
	writeThread(aes, clientSocketID);
	
	//5. disconnect
	close(clientSocketID);
	close(serverSocketID);
	return 0;
	
	closeConnection:
		close(clientSocketID);
		close(serverSocketID);
		return 1;
}



int clientsideSecureConnection(int socketID, const char* clientKeyPath, const char* serverKeyPath){
		
	//temp variables
	char* buffer = new char[LENGTH_1024];
	unsigned char* unsignedbuffer = new unsigned char[LENGTH_1024];
	
	char* base64encoded;
	unsigned char* base64decoded;
			
	char servercurvename[LENGTH_256];	
	char clientcurvename[LENGTH_256];
	char clientPrivateKeyBase64[LENGTH_256];
	char serverPublicKeyX[LENGTH_256];
	char serverPublicKeyY[LENGTH_256];
	
	unsigned char aeskey[LENGTH_256];	
	char signature[LENGTH_1024];
	unsigned char encryptedMessage[LENGTH_1024];
	unsigned char decryptedMessage[LENGTH_1024];	
	char iv[LENGTH_64];
	char ivcopy[LENGTH_64];
	char finalMessage[LENGTH_1024];
	
	CE_curve* curve = new CE_curve();
	size_t base64size;
	SHA256 sha256 = SHA256();	
	ECDSA* ecdsa;
	AES* aes;
	
	FILE* serverKeyFile = fopen(serverKeyPath, "rt");
	FILE* clientKeyFile = fopen(clientKeyPath, "rt");
	
	CE_point* gx;
	CE_point* gy;
	CE_point* gxy;	
	CE_point* serverPublicKeyP;
	mpz_t randomx;
	
	//1. read parameters from file
	printf("\nInitializing...");
	//1.1. read client private key	
	if (!clientKeyFile){
		printf("\nError: error reading client private key");
		goto closeConnection;
	}	
	//1.1.1. read curve name and private key
	fscanf(clientKeyFile, "%s", clientcurvename); //ignore first line
	fscanf(clientKeyFile, "%s", clientcurvename);
	fscanf(clientKeyFile, "%s", clientPrivateKeyBase64);
	fclose(clientKeyFile);	
	//1.2. read server pubic key		
	if (!serverKeyFile){
		printf("\nError: error reading server public key");
		goto closeConnection;
	}
	//1.2.1. read curve name and public key
	fscanf(serverKeyFile, "%s", servercurvename); //ignore first line
	fscanf(serverKeyFile, "%s", servercurvename); //read curve name
	fscanf(serverKeyFile, "%s", serverPublicKeyX); //read first point
	fscanf(serverKeyFile, "%s", serverPublicKeyY); //read second point
	fclose(serverKeyFile);	
	//1.3 init variable
	mpz_t clientPrivateKey;
	if (!utils::Base64Decode(clientPrivateKeyBase64, &base64size, &base64decoded)){
		printf("\nError: client private key x damaged");
		goto closeConnection;
	}
	mpz_init_set_str(clientPrivateKey, (char*) base64decoded, 10);
	
	mpz_t serverPublicKeyPx;
	if (!utils::Base64Decode(serverPublicKeyX, &base64size, &base64decoded)){
		printf("\nError: server public key y damaged");
		goto closeConnection;
	}
	mpz_init_set_str(serverPublicKeyPx, (char*) base64decoded, 10);
	
	mpz_t serverPublicKeyPy;
	if (!utils::Base64Decode((char*) serverPublicKeyY, &base64size, &base64decoded)){
		printf("\nError: server public key damaged");
		goto closeConnection;
	}
	mpz_init_set_str(serverPublicKeyPy, (char*) base64decoded, 10);	
	serverPublicKeyP = new CE_point(&serverPublicKeyPx, &serverPublicKeyPy);
	
	//2. check parameters
	if (strcmp(clientcurvename, servercurvename)!=0){
		printf("\nError: client and server not share the same paramters");
		goto closeConnection;
	}	
	if (loadParameter(clientcurvename, curve)){
		printf("\nError: error loading paramters");
		goto closeConnection;
	}
	
	//3. START PROTOCOL
	//3.1. generate random x, calculate gx and send
	//3.1.1. generate x
	int request_security_strength;
	request_security_strength = curve->getRequestSecurityLength();
	mpz_init(randomx);
	utils::getRandomNumber(&randomx, request_security_strength);	
	//3.1.2. calculate gx
	gx = curve->mul(curve->g, &randomx);
	strcpy(buffer, mpz_get_str(NULL, 10, gx->x));
	strcat(buffer, ";");
	strcat(buffer, mpz_get_str(NULL, 10, gx->y));
	utils::Base64Encode((unsigned char*) buffer, strlen(buffer), &base64encoded);
	//3.1.3 send gx
	printf("\nSending gx...");
	fflush(stdout);
	int writeResult;
	writeResult = write(socketID, base64encoded, strlen(base64encoded));
	if (writeResult<0){
		printf("\nError: cannot write to buffer");
		goto closeConnection;
	}	
	
	//3.2. wait to receive gy and Ek(signature) from server
	printf("\nReading signature...");
	fflush(stdout);
	int readResult;
	do{
		readResult = read(socketID,buffer,LENGTH_1024);
	}
	while (readResult==0);

	if (!utils::Base64Decode(buffer, &base64size, &base64decoded)){
		printf("\nError: bad encoding");
		goto closeConnection;
	}
	
	//3.2.1 decode signature = gy.x;gy.y;signature;iv
	char* split[4];
	utils::split((char*) base64decoded, ';', split);
	
	//3.2.2 calculte K = gxy
	gy = new CE_point(split[0], split[1]);
	gxy = curve->mul(gy, &randomx);
	
	//3.3 decrypt signature and verify
	printf("\nVerifying signature...");
	fflush(stdout);
	//3.3.1. generate aes key
	//bzero(buffer, LENGTH_1024);
	strcpy(buffer, mpz_get_str(NULL, 10, gxy->x));
	strcat(buffer, mpz_get_str(NULL, 10, gxy->y));	
	sha256.init();
	sha256.update((unsigned char*) buffer, strlen(buffer));
	sha256.final(aeskey);
	
	//3.3.1.1 decode base64 of signature before decryption
	if (!utils::Base64Decode(split[2], &base64size, &unsignedbuffer)){
		printf("\nError: signature damaged");
		goto closeConnection;
	}
	//3.3.1.2 decode iv before decryption
	if (!utils::Base64Decode(split[3], &base64size, &base64decoded)){
		printf("\nError: iv damaged");
		goto closeConnection;
	}
	
	//3.3.2 use AES 256 to decrypt message	
	AES_KEY aes_dec_key;
	AES_set_decrypt_key(aeskey, LENGTH_256, &aes_dec_key);
	AES_cbc_encrypt(unsignedbuffer, decryptedMessage, LENGTH_1024, &aes_dec_key, base64decoded, AES_DECRYPT);
	

	//3.3.4 verify the signature
	ecdsa = new ECDSA(curve, serverPublicKeyP, NULL);
	if (!ecdsa->verifySignature(buffer, (char*) decryptedMessage)){
		printf("\nError: signature verification fail");
		goto closeConnection;
	}
	
	//3.4. send Ek(S(gx, gy))
	printf("\nSending signature...");
	fflush(stdout);
	//3.4.1. concat gx,gy
	bzero(buffer, LENGTH_1024);
	strcpy(buffer, mpz_get_str(NULL, 10, gx->x));
	strcat(buffer, ";");
	strcpy(buffer, mpz_get_str(NULL, 10, gx->y));
	strcat(buffer, ";");
	strcat(buffer, mpz_get_str(NULL, 10, gy->x));	
	strcat(buffer, ";");
	strcpy(buffer, mpz_get_str(NULL, 10, gy->y));
	ecdsa = new ECDSA(curve, NULL, &clientPrivateKey);
	
	bzero(signature, LENGTH_1024);
	ecdsa->generateSignature(buffer, signature);
	
	//3.4.2. use AES 256 to encrypt message	
	AES_KEY aes_enc_key;
	AES_set_encrypt_key(aeskey, LENGTH_256, &aes_enc_key);
	//generate an IV	
	bzero(iv, LENGTH_64);
	utils::getRandomString(iv, 32);
	//save a copy of iv, which will be modified after the encryption
	strcpy(ivcopy, iv);
	
	//pre-compute the encryption length
	int encryptionLength;
	encryptionLength = ceil(strlen(signature)/16.0)*16;
	bzero(encryptedMessage, LENGTH_1024);
	AES_cbc_encrypt((unsigned char*) signature, encryptedMessage, encryptionLength, &aes_enc_key, (unsigned char*) iv, AES_ENCRYPT);	
	
	//3.4.3. send as signature;iv
	bzero(finalMessage, LENGTH_1024);
	utils::Base64Encode(encryptedMessage, encryptionLength, &base64encoded);		
	strcpy(finalMessage, base64encoded);
	//3.4.3.1 convert concat iv
	utils::Base64Encode((unsigned char*)ivcopy, strlen(ivcopy), &base64encoded);		
	strcat(finalMessage, ";");
	strcat(finalMessage, base64encoded);
	//3.4.3.1 final encoding
	utils::Base64Encode((unsigned char*) finalMessage, strlen((char*) finalMessage), &base64encoded);
	//printf("\nfinal message: %s", buffer);
	
	//3.4.4 send signature
	writeResult = write(socketID, base64encoded, strlen(base64encoded));
	if (writeResult<0){
		printf("\nError: cannot write to buffer");
		goto closeConnection;
	}
	
	//4. start communication
	printf("\nSecure connection established!");
	fflush(stdout);	
	//4.1. create a thread to listen	
	pthread_t listeningThread;	
	//4.2. parameter: port;base64(aeskey)
	char args[LENGTH_256];
	sprintf(args, "%d", socketID);
	strcat(args, ";");
	utils::Base64Encode(aeskey, 32, &base64encoded);
	strcat(args, base64encoded);
	if (pthread_create(&listeningThread, NULL, listenThread, args)){
		printf("\nError: cannot create listening thread");
		goto closeConnection;
	}
	
	//4.2. read and send until reach REQUEST_DISCONNECT
	working = true;	
	aes = new AES(aeskey);
	writeThread(aes, socketID);
	close(socketID);
	return 0;
	
	closeConnection:
		close(socketID);
		return 1;
}

int main(int argc, char **argv){
	/*
	 * dh-sts using:
	 * 1. help function
	 * dh-sts
	 * dh-sts -h
	 * dh-sts --help
	 * 
	 * 2. server service
	 * dh-sts -s <path to server private key> <path to client public key>
	 * dh-sts --server
	 *
	 * 3. client service
	 * dh-sts -c <server IP> <path to client private key> <path to server public key>
	 * dh-sts --client
	 * 
	 * */
	 
	if (argc==1 || strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0){
		//display help
		printf("DIFFIE-HELLMAN STS PROTOCOL");
		printf("\nThis program is used for establishing a secure connection between two stations\n");		
		printf("\nDETAILS\n");
		printf("\n\t -h, --help");		
		printf("\n\t\t Display this help");
		printf("\n\t -s, --server");
		printf("\n\t\t <path to server private key> <path to client public key>");
		printf("\n\t\t Start server service");
		printf("\n\t -c, --client");
		printf("\n\t\t <server IP> <path to client private key> <path to server public key>");
		printf("\n\t\t Start client service");
		printf("\n");
	}
	else if (argc==5 && ((strcmp(argv[1], "-c")==0) || (strcmp(argv[1], "--client")==0))){
		//1. create client socket
		int socketID;
		struct sockaddr_in serverSocket;
		struct hostent *server;
		socketID = socket(AF_INET, SOCK_STREAM, 0);
						
		if (socketID < 0) {
			printf("\nError: cannot open socket");
			goto error;			
		}
		server = gethostbyname(argv[2]);
		if (server == NULL) {
			printf("\nError: server not found");
			goto error;
		}
		bzero((char *) &serverSocket, sizeof(serverSocket));
		serverSocket.sin_family = AF_INET;
		bcopy((char *)server->h_addr, 
			 (char *)&serverSocket.sin_addr.s_addr,
			 server->h_length);
		serverSocket.sin_port = htons(SERVER_PORT);
		if (connect(socketID,(struct sockaddr *) &serverSocket,sizeof(serverSocket)) < 0){			
			printf("\nError: cannot connect to server");
			goto error;
		}
		if (clientsideSecureConnection(socketID, argv[3], argv[4])){
			printf("\nCannot establish secure connection\n");
			goto error;
		}
		return 0;
		
		error:
			return 1;
	}
	else if (argc==4 && ((strcmp(argv[1], "-s")==0) || (strcmp(argv[1], "--server")==0))){	
		//create server socket
		int serverSocketID;
		int clientSocketID;
		struct sockaddr_in serverSocket;
		struct sockaddr_in clientSocket;
		
		printf("\nListenning for incomming request...");
		fflush(stdout);
		serverSocketID = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocketID < 0) {
			printf("\nError: cannot open server socket");
			goto servererror;
		}
		
		int iSetOption;
		iSetOption = 1;
		setsockopt(serverSocketID, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,
        sizeof(iSetOption));
		//fill zero to stream
		bzero((char *) &serverSocket, sizeof(serverSocket));
		//assign port and conection mode
		serverSocket.sin_family = AF_INET;
		serverSocket.sin_addr.s_addr = INADDR_ANY;
		serverSocket.sin_port = htons(SERVER_PORT);
		
		if (bind(serverSocketID, (struct sockaddr *) &serverSocket, sizeof(serverSocket)) < 0) {
              printf("\nError: cannot bind server socket");
			  goto servererror;
		}	
		
		//may add a while(1) to listen to many connection requests		
		listen(serverSocketID, MAX_ACCEPT_CONNECTIONS);
		socklen_t clientSocketLength;
		
		clientSocketLength = sizeof(clientSocket);
		clientSocketID = accept(serverSocketID, (struct sockaddr *) &clientSocket, &clientSocketLength);
		if (clientSocketID < 0) {
			printf("\nError: cannot accept new connection");
			goto servererror;
		}
		
		//starting serverside protocol
		if (serversideSecureConnection(serverSocketID, clientSocketID, argv[2], argv[3])){
			printf("\nCannot establish secure connection\n");
			goto servererror;
		}
		printf("\nCommunication terminated!\n");
		return 0;
		
		servererror:
			return 1;
	}
	else{
		printf("\nWrong usage of dh-sts. See --help for more information.\n");
	}
	return 0;
}