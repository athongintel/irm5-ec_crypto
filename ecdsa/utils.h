#ifndef UTILS_H
#define UTILS_H

#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <assert.h>
#include <openssl/buffer.h>


class utils
{
private:
	static size_t calcDecodeLength(const char* b64input);
public:
	utils();
	~utils();
	static int getRandomString(char* myRandomData, int length);
	static int Base64Decode(char* b64message, size_t* length, unsigned char** buffer);
	static int Base64Encode(const unsigned char* buffer, size_t length, char** b64text);
	static int split(const char* input, const char delimiter, char** output);
	static void getRandomNumber(mpz_t* value, int securityLength);
};

#endif // UTILS_H
