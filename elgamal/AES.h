#ifndef AES_H
#define AES_H

#include <openssl/aes.h>
#include <math.h>

#include "utils.h"

class AES
{	
AES_KEY aes_enc_key;
AES_KEY aes_dec_key;

public:
	AES(const unsigned char* key);
	bool cbc_encrypt(char* message, char* encrypted);
	bool cbc_decrypt(char* message, char* decrypted);
	~AES();

};

#endif // AES_H
