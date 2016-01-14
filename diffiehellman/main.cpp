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

const char* EC_PATH ="./ecparam/";

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

int serversideSecureConnection(int serverSocketID, int clientSocketID, const char* curvename){
    //temp variables
    char* buffer = new char[LENGTH_1024];
    char* dual[2];
    CE_curve* curve = new CE_curve();
    CE_point* gx;
    CE_point* gy;
    CE_point* gxy;
    CE_point* u;
    CE_point* v;
    CE_point* commonkey;
    CE_point* clientpoint;
    CE_point* serverpoint;
    gmp_randstate_t randomstate;
    mpz_t seed;
    mpz_t random_c;
    int requested_security_strength;

    // read parameters from file
	printf("\nInitializing...");
	fflush(stdout);
    
    printf("\nLoading key parameters...");
    if (loadParameter(curvename,curve)){
        printf("\nInvalid parameter file %s %d", curvename, strlen(curvename));
        goto closeConnection;
    }
    printf("OK");
    
    printf("\nTesting key parameters...");
    //cheking for length
        
    requested_security_strength = curve->getRequestSecurityLength();
    if (requested_security_strength<0){
        printf("\nNot a secure length!");
        goto closeConnection;
    }
    
    
    //1.Generate random value
    //random state, seed from urandom
    
    gmp_randinit_default(randomstate);
    
    mpz_init(seed);
    
    int N;
    N = curve->getRequestSecurityLength();
    utils::getRandomNumber(&seed, N/8);
    gmp_randseed(randomstate, seed);
            
    
    mpz_init(random_c);
    mpz_rrandomb(random_c, randomstate, N+64);
    
    
    //2. Compute point
    
    serverpoint = curve->mul(curve->g, &random_c);
    
    //3. Receive client point
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
	printf("\n%s",buffer);
	fflush(stdout);
    utils::split(buffer, ';', dual);
    clientpoint = new CE_point(dual[0],dual[1]);
    
    
    
    //4. Send point
   	strcpy(buffer, mpz_get_str(NULL, 10, serverpoint->x));
	strcat(buffer, ";");
	strcat(buffer, mpz_get_str(NULL, 10, serverpoint->y));

    int writeResult;
	writeResult = write(clientSocketID, buffer, strlen(buffer));
	if (writeResult<0){
		printf("\nError: cannot write to buffer");
		goto closeConnection;
	}	
	
    
    //5. compute common key
    
    commonkey = curve->mul(clientpoint, &random_c);
    printf("\n\nPoint commun :\n%s ; %s",mpz_get_str(NULL, 10,commonkey->x),mpz_get_str(NULL, 10,commonkey->y));
    
     
	close(clientSocketID);
	close(serverSocketID);
	return 0;
	
	closeConnection:
		close(clientSocketID);
		close(serverSocketID);
		return 1;
     
}



int clientsideSecureConnection(int socketID, const char* curvename){
		
	//temp variables
    char* buffer = new char[LENGTH_1024];
    char* dual[2];
    CE_curve* curve = new CE_curve();
    CE_point* u;
    CE_point* gy;
    CE_point* v;
    CE_point* h;
    CE_point* clientpoint;
    CE_point* serverpoint;
    mpz_t randomb;
    gmp_randstate_t randomstate;
    mpz_t seed;
    mpz_t random_c;
    
    printf("\nInitializing...");
    // read parameters from file
    printf("\nLoading key parameters...");
    if (loadParameter(curvename,curve)){
        printf("\nInvalid parameter file %s %d", curvename, strlen(curvename));
        goto closeConnection;
    }
    printf("OK");
    
    printf("\nTesting key parameters...");
    //cheking for length
    int requested_security_strength;    
    requested_security_strength = curve->getRequestSecurityLength();
    if (requested_security_strength<0){
        printf("\nNot a secure length!");
        goto closeConnection;
    }


   
    //1.Generate random value
    //random state, seed from urandom
    gmp_randinit_default(randomstate);
    mpz_init(seed);
    int N;
    N = curve->getRequestSecurityLength();
    utils::getRandomNumber(&seed, N/8);
    gmp_randseed(randomstate, seed);
    mpz_init(random_c);
    mpz_rrandomb(random_c, randomstate, N+64);
    
    
    //2. Compute point
    clientpoint = curve->mul(curve->g, &random_c);
    
    //3. Send point
   	strcpy(buffer, mpz_get_str(NULL, 10, clientpoint->x));
	strcat(buffer, ";");
	strcat(buffer, mpz_get_str(NULL, 10, clientpoint->y));

    int writeResult;
	writeResult = write(socketID, buffer, strlen(buffer));
	if (writeResult<0){
		printf("\nError: cannot write to buffer");
		goto closeConnection;
	}	
	
    
    //4. Receive client point
    int readResult;
    
	bzero(buffer,LENGTH_1024);
	do{
		readResult = read(socketID,buffer,LENGTH_1024);
	}
	while (readResult==0);
	if (readResult < 0){
		printf("\nError: error reading gx");
		goto closeConnection;
	}
	printf("\nReading gx...");
	printf("\n%s",buffer);
	fflush(stdout);
    utils::split(buffer, ';', dual);
    serverpoint = new CE_point(dual[0],dual[1]);
    
    
    
    //5. compute common key
    CE_point* commonkey;
    commonkey = curve->mul(serverpoint, &random_c);
    printf("\n\nPoint commun :\n%s ; %s",mpz_get_str(NULL, 10, commonkey->x),mpz_get_str(NULL, 10,commonkey->y));
    
	close(socketID);
	return 0;
	
	closeConnection:
		close(socketID);
		return 1;
}

int main(int argc, char **argv){
	/*
	 * diffie hellman using:
	 * 1. help function
	 * diffiehellman
	 * diffiehellman -h
	 * diffiehellman --help
	 * 
	 * 2. server service
	 * diffiehellman -s <curve name>
	 * diffiehellman --server <curve name>
	 *
	 * 3. client service
	 * diffiehellman -c <server IP>  <curve name>
	 * diffiehellman --client <server IP>  <curve name>
	 * 
	 * */
	 
	if (argc==1 || strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0){
		//display help
		printf("DIFFIE HELLMAN PROTOCOL");
		printf("\nThis program is used for sending and receiving secure data between two stations\n");		
		printf("\nDETAILS\n");
		printf("\n\t -h, --help");		
		printf("\n\t\t Display this help");
		printf("\n\t -s, --server");
		printf("\n\t\t <curve name>");
		printf("\n\t\t Start server service");
		printf("\n\t -c, --client");
		printf("\n\t\t <server IP> <curve name>");
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
		printf("\nWrong usage of diffiehellman. See --help for more information.\n");
	}
	return 0;
}