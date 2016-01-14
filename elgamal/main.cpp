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

int serversideSecureConnection(int serverSocketID, int clientSocketID, const char* serverKeyPath){
	//temp variables
	char* buffer = new char[LENGTH_1024];
	
	char* base64encoded;// = new char[LENGTH_1024];
	unsigned char* base64decoded;// = new unsigned char[LENGTH_1024];
	unsigned char* base64decoded2;// = new unsigned char[LENGTH_1024];
		
	char* trois[3];
	char servercurvename[LENGTH_256];	
	char clientcurvename[LENGTH_256];
	char serverPrivateKeyBase64[LENGTH_256];
	char clientPublicKeyX[LENGTH_256];
	char clientPublicKeyY[LENGTH_256];
	char signature[LENGTH_1024];
	char encryptedMessage[LENGTH_1024];
	char decryptedMessage[LENGTH_1024];
    unsigned char aeskey[LENGTH_256];
	char iv[LENGTH_64];
	char ivcopy[LENGTH_64];
	
	CE_curve* curve = new CE_curve();
	size_t base64size;
	SHA256 sha256 = SHA256();	
	ECDSA* ecdsa;
	AES* aes;
	//variables
	
	FILE* serverKeyFile = fopen(serverKeyPath, "rt");
	
	CE_point* gx;
	CE_point* gy;
	CE_point* gxy;	
    CE_point* u;
	CE_point* v;
	
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

	//1.3 init variable
	mpz_t serverPrivateKey;
	if (!utils::Base64Decode(serverPrivateKeyBase64, &base64size, &base64decoded)){
		printf("\nError: server private key damaged");
		goto closeConnection;
	}
	mpz_init_set_str(serverPrivateKey, (char*) base64decoded, 10);
	

	if (loadParameter(servercurvename, curve)){
		printf("\nError: error loading paramters");
		goto closeConnection;
	}
	
	//3. START DECHIFFREMENT	
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
	//printf("\nReading gx...");
	//printf("\n%s",buffer);
	//fflush(stdout);

	utils::split(buffer, ';', trois);
	
	//printf("\n%s", trois[0]);
	//printf("\n%s", trois[1]);
	//printf("\n%s", trois[2]);
    
    //create point u
    u = new CE_point(trois[0],trois[1]);
    //encryptedMessage = trois[2];
    //create point v
    v = curve->mul(u, &serverPrivateKey);
    
    
    strcpy(buffer, mpz_get_str(NULL, 10, u->x));
	strcat(buffer, mpz_get_str(NULL, 10, u->y));
	strcat(buffer, mpz_get_str(NULL, 10, v->x));
	strcat(buffer, mpz_get_str(NULL, 10, v->y));
	
	sha256.init();
	sha256.update((unsigned char*) buffer, strlen(buffer));
	sha256.final(aeskey);
	
    aes = new AES(aeskey);
	aes->cbc_decrypt(trois[2], decryptedMessage);
    printf("\nReceived: %s",decryptedMessage);
    
	close(clientSocketID);
	close(serverSocketID);
	return 0;
	
	closeConnection:
		close(clientSocketID);
		close(serverSocketID);
		return 1;
}



int clientsideSecureConnection(int socketID, const char* serverKeyPath){
		
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
	
	CE_point* u;
	CE_point* gy;
	CE_point* v;
	CE_point* h;
	mpz_t randomb;
	
	//1. read parameters from file
	printf("\nInitializing...");
	//1.1. read client private key	
		
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
	h = new CE_point(&serverPublicKeyPx, &serverPublicKeyPy);
	
	loadParameter(servercurvename,curve);	

	//3.1. generate random b, calculate u=b*g and send
	//3.1.1. generate b
	int request_security_strength;
	request_security_strength = curve->getRequestSecurityLength();
	mpz_init(randomb);
	utils::getRandomNumber(&randomb, request_security_strength);	
	//3.1.2. calculate gx
	u = curve->mul(curve->g, &randomb);
	v = curve->mul(h, &randomb);
	
	strcpy(buffer, mpz_get_str(NULL, 10, u->x));
	strcat(buffer, mpz_get_str(NULL, 10, u->y));
	strcat(buffer, mpz_get_str(NULL, 10, v->x));
	strcat(buffer, mpz_get_str(NULL, 10, v->y));
	
	sha256.init();
	sha256.update((unsigned char*) buffer, strlen(buffer));
	sha256.final(aeskey);
	
	printf("\nInput a message: ");
	
    //Word without space : scanf
    scanf("%[^\n]", buffer);
    if(strlen(buffer)>0 && (buffer[strlen(buffer)-1]== '\n'))
            buffer[strlen(buffer)-1] = '\0';
	//printf("\nVous avez entré : \n%s",buffer);
    

	aes = new AES(aeskey);
	aes->cbc_encrypt(buffer, finalMessage);
	
	strcpy(buffer, mpz_get_str(NULL, 10, u->x));
	strcat(buffer, ";");
	strcat(buffer, mpz_get_str(NULL, 10, u->y));
	strcat(buffer, ";");
	strcat(buffer, finalMessage);
		
	int writeResult;
	writeResult = write(socketID, buffer, strlen(buffer));
	if (writeResult<0){
		printf("\nError: cannot write to buffer");
		goto closeConnection;
	}	
	
	close(socketID);
	return 0;
	
	closeConnection:
		close(socketID);
		return 1;
}

int main(int argc, char **argv){
	/*
	 * elgamal using:
	 * 1. help function
	 * elgamal
	 * elgamal -h
	 * elgamal --help
	 * 
	 * 2. server service
	 * elgamal -s <path to server private key>
	 * elgamal --server
	 *
	 * 3. client service
	 * elgamal -c <server IP>  <path to server public key>
	 * elgamal --client
	 * 
	 * */
	 
	if (argc==1 || strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0){
		//display help
		printf("ELGAMAL ENCRYPTION");
		printf("\nThis program is used for sending and receiving secure data between two stations\n");		
		printf("\nDETAILS\n");
		printf("\n\t -h, --help");		
		printf("\n\t\t Display this help");
		printf("\n\t -s, --server");
		printf("\n\t\t <path to server private key>");
		printf("\n\t\t Start server service");
		printf("\n\t -c, --client");
		printf("\n\t\t <server IP> <path to server public key>");
		printf("\n\t\t Start client service");
		printf("\n");
	}
	else if (argc==4 && ((strcmp(argv[1], "-c")==0) || (strcmp(argv[1], "--client")==0))){
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
		if (clientsideSecureConnection(socketID, argv[3])){
			printf("\nCannot establish secure connection\n");
			goto error;
		}
		return 0;
		
		error:
			return 1;
	}
	else if (argc==3 && ((strcmp(argv[1], "-s")==0) || (strcmp(argv[1], "--server")==0))){	
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
		if (serversideSecureConnection(serverSocketID, clientSocketID, argv[2])){
			printf("\nCannot establish secure connection\n");
			goto servererror;
		}
		printf("\nCommunication terminated!\n");
		return 0;
		
		servererror:
			return 1;
	}
	else{
		printf("\nWrong usage of elgamal. See --help for more information.\n");
	}
	return 0;
}