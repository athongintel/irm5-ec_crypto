#include "utils.h"
#include <cstring>

//Decodes Base64
#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

utils::utils()
{
}

utils::~utils()
{
}


size_t utils::calcDecodeLength(const char* b64input) { //Calculates the length of a decoded string
	size_t len = strlen(b64input),
		padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
		padding = 2;
	else if (b64input[len-1] == '=') //last char is =
		padding = 1;

	return (len*3)/4 - padding;
}

int utils::Base64Decode(char* b64message, size_t* length, unsigned char** buffer) { //Decodes a base64 encoded string
	BIO *bio, *b64;

	int decodeLen = calcDecodeLength(b64message);
	*buffer = (unsigned char*)malloc(decodeLen + 1);
	(*buffer)[decodeLen] = '\0';

	bio = BIO_new_mem_buf(b64message, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	*length = BIO_read(bio, *buffer, strlen(b64message));
	//length should equal decodeLen, else something went horribly wrong
	BIO_free_all(bio);

	return (*length == decodeLen); //success
}


int utils::Base64Encode(const unsigned char* buffer, size_t length, char** b64text) { //Encodes a binary safe base 64 string
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
	BIO_write(bio, buffer, length);
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	BIO_set_close(bio, BIO_NOCLOSE);
	BIO_free_all(bio);

	*b64text=(*bufferPtr).data;

	return (0); //success
}

int utils::getRandomString(char* myRandomDataOutput, const int length){
	int randomData = open("/dev/urandom", O_RDONLY);
	char myRandomData[length];
	size_t randomDataLen = 0;
	while (randomDataLen < sizeof myRandomData)
	{
		ssize_t result = read(randomData, myRandomData + randomDataLen, (sizeof myRandomData) - randomDataLen);
		if (result <= 0)
		{
			printf("\ne");
		}
		randomDataLen += result;
	}
	close(randomData);
	strcpy(myRandomDataOutput, myRandomData);
	for (int i=0; i<length;i++){
		if (myRandomDataOutput[i]==0) myRandomDataOutput[i]=1;
	}
	myRandomDataOutput[length]='\0';
	return strlen(myRandomDataOutput);
}

int utils::split(const char* input, const char delimiter, char** output){
	int i=0;
	int j=0;
	int count=0;	
		
	do{
		if (j==0){
			output[count] = (char*) malloc(sizeof (char) * 1024);	
		}
		char c = (char) input[i];
		if (c==delimiter || c=='\0' || c=='\n'){
			output[count][j] = '\0';
			j=0;
			count++;
			if (c=='\0' || c=='\n'){
				return count;
			}
		}
		else{
			output[count][j] = c;
			j++;
		}
		i++;
	}
	while (1);
}

void utils::getRandomNumber(mpz_t* value, int securityLength){
	char randomString[securityLength/8];
	utils::getRandomString(randomString, securityLength/8);		
	int length = strlen(randomString);
	for (int i=0;i<length;i++){
		mpz_mul_ui(*value, *value, 256);		
		mpz_add_ui(*value, *value, (unsigned char) randomString[i]);
	}
}